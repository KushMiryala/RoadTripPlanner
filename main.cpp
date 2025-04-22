#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cmath>

#include "file_utils.h"
#include "geo_utils.h"
#include "dijkstra.h"
#include "kdtree.h"
#include "ui.h"

// basic parsers
static bool parseD(const std::string& s, double& v) { std::stringstream ss(s); return bool(ss >> v); }
static bool parseI(const std::string& s, int&    v) { std::stringstream ss(s); return bool(ss >> v); }

struct TextBox { sf::RectangleShape box; std::string txt; bool active = false; };

// find city whose dot is near the mouse
static int pickCity(const std::vector<CityDot>& dots, sf::Vector2f m, float r = 6.f)
{
    for (const auto& d : dots)
        if (std::hypot(m.x - d.pos.x, m.y - d.pos.y) <= r) return d.cityIdx;
    return -1;
}

int main()
{
    const std::string dataFile = "US_trimmed.txt";
    const int   panelW  = 400;
    const sf::Vector2u winSz{1600, 900};

    sf::RenderWindow win(sf::VideoMode(winSz.x, winSz.y), "RoadTrip Planner");
    win.setFramerateLimit(60);

    sf::Font font; font.loadFromFile("C:/Windows/Fonts/arial.ttf");

    const char* lbl[5] = { "Start Lat","Start Lon","End Lat","End Lon","# Cities" };
    TextBox tb[5];
    for (int i = 0; i < 5; ++i) {
        tb[i].box.setSize({ panelW - 40.f, 30.f });
        tb[i].box.setPosition(20, 40 + i * 60);
        tb[i].box.setFillColor(sf::Color(50, 50, 60));
        tb[i].box.setOutlineColor(sf::Color::White);
        tb[i].box.setOutlineThickness(1);
    }
    sf::RectangleShape btn({ panelW - 40.f, 40.f });
    btn.setPosition(20, tb[4].box.getPosition().y + 60);
    btn.setFillColor(sf::Color(70, 120, 70));

    std::vector<City> cities;
    std::vector<int>  path;
    std::vector<CityDot> dots;
    struct PathLabel { sf::FloatRect rect; int idx; };
    std::vector<PathLabel> pathLabels;

    std::vector<Landmark> allLM, currentLM;
    KDNode* lmTree = nullptr;
    std::string selectedCity;
    double minLat{}, maxLat{}, minLon{}, maxLon{};

    // recompute route and KD‑tree
    auto compute = [&]() {
        double sLat, sLon, eLat, eLon; int k;
        if (!(parseD(tb[0].txt, sLat) && parseD(tb[1].txt, sLon) &&
              parseD(tb[2].txt, eLat) && parseD(tb[3].txt, eLon) &&
              parseI(tb[4].txt, k)))
            return;

        minLat = std::min(sLat, eLat);  maxLat = std::max(sLat, eLat);
        minLon = std::min(sLon, eLon);  maxLon = std::max(sLon, eLon);

        cities = readCitiesWithinBox(dataFile, minLat, maxLat, minLon, maxLon);
        if (cities.size() > 100) cities.resize(100);
        int N = cities.size(); if (N < 2) { path.clear(); return; }

        std::vector<std::vector<double>> G(N, std::vector<double>(N));
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                G[i][j] = haversine(cities[i].lat, cities[i].lon, cities[j].lat, cities[j].lon);

        double midLat = (sLat + eLat) / 2, midLon = (sLon + eLon) / 2;
        std::vector<std::pair<double,int>> dist;
        for (int i = 0; i < N; ++i)
            dist.emplace_back(haversine(midLat, midLon, cities[i].lat, cities[i].lon), i);
        std::sort(dist.begin(), dist.end());

        k = std::max(0, k);
        std::vector<int> route;
        for (int i = 0; i < k && i < (int)dist.size(); ++i) route.push_back(dist[i].second);

        auto closest = [&](double qLat,double qLon){
            double best = 1e9; int idx = 0;
            for (int i = 0; i < N; ++i) {
                double d = haversine(qLat,qLon,cities[i].lat,cities[i].lon);
                if (d < best) { best = d; idx = i; }
            }
            return idx;
        };
        route.insert(route.begin(), closest(sLat, sLon));
        route.push_back(closest(eLat, eLon));

        path.clear();
        for (size_t i = 0; i + 1 < route.size(); ++i) {
            auto seg = dijkstra(route[i], route[i+1], G);
            if (i) seg.erase(seg.begin());
            path.insert(path.end(), seg.begin(), seg.end());
        }

        allLM = readLandmarksWithinBox(dataFile, minLat, maxLat, minLon, maxLon);
        lmTree = buildKDTree(allLM);
        currentLM.clear(); selectedCity.clear();
    };

    while (win.isOpen()) {
        sf::Event ev;
        while (win.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) win.close();

            if (ev.type == sf::Event::MouseButtonPressed &&
                ev.mouseButton.button == sf::Mouse::Left) {

                sf::Vector2f m(ev.mouseButton.x, ev.mouseButton.y);

                if (btn.getGlobalBounds().contains(m)) compute();
                for (int i = 0; i < 5; ++i)
                    tb[i].active = tb[i].box.getGlobalBounds().contains(m);

                for (const auto& pl : pathLabels)
                    if (pl.rect.contains(m)) {
                        selectedCity = cities[pl.idx].name;
                        currentLM = findNearestLandmarksKD(lmTree, cities[pl.idx], 5);
                    }

                float vpLeft = panelW / (float)win.getSize().x;
                if (m.x / win.getSize().x >= vpLeft && !path.empty()) {
                    sf::View mapV = win.getView();
                    mapV.setViewport({ vpLeft,0,1 - vpLeft,1 });
                    win.setView(mapV);
                    sf::Vector2f mp = win.mapPixelToCoords(
                        sf::Vector2i(ev.mouseButton.x, ev.mouseButton.y));
                    int idx = pickCity(dots, mp);
                    win.setView(win.getDefaultView());
                    if (idx != -1 && lmTree) {
                        selectedCity = cities[idx].name;
                        currentLM = findNearestLandmarksKD(lmTree, cities[idx], 5);
                    }
                }
            }

            if (ev.type == sf::Event::TextEntered) {
                char c = (char)ev.text.unicode;
                for (int i = 0; i < 5; ++i) if (tb[i].active) {
                    if (c == 8 && !tb[i].txt.empty()) tb[i].txt.pop_back();
                    else if (c == 13) tb[i].active = false;
                    else if (std::isdigit(c) || c == '-' || c == '.')
                        tb[i].txt.push_back(c);
                }
            }
        }

        win.clear(sf::Color(25, 28, 36));
        sf::RectangleShape panel({ (float)panelW, (float)win.getSize().y });
        panel.setFillColor(sf::Color(35, 38, 45));
        win.draw(panel);

        sf::Text txt("", font, 15);

        for (int i = 0; i < 5; ++i) {
            txt.setString(lbl[i]);
            txt.setPosition(20, tb[i].box.getPosition().y - 18); win.draw(txt);
            win.draw(tb[i].box);
            txt.setString(tb[i].txt.empty() ? "_" : tb[i].txt);
            txt.setPosition(tb[i].box.getPosition().x + 4,
                            tb[i].box.getPosition().y + 4); win.draw(txt);
        }
        win.draw(btn);
        txt.setString("Compute");
        txt.setPosition(btn.getPosition().x + (panelW - 40 - 60) / 2.f,
                        btn.getPosition().y + 10); win.draw(txt);

        float y = btn.getPosition().y + 60;
        pathLabels.clear();
        if (!path.empty()) {
            txt.setFillColor(sf::Color::White);
            txt.setString("Path (click a name to see 5 landmarks):");
            txt.setPosition(20, y); win.draw(txt); y += 20;

            for (int idx : path) {
                txt.setString("- " + cities[idx].name);
                txt.setPosition(30, y); win.draw(txt);
                pathLabels.push_back({ txt.getGlobalBounds(), idx });
                y += 18;
            }
        }
        if (!currentLM.empty()) {
            y += 10;
            txt.setString("Landmarks near " + selectedCity + ":");
            txt.setPosition(20, y); win.draw(txt); y += 20;
            for (const auto& lm : currentLM) {
                txt.setString("- " + lm.name);
                txt.setPosition(30, y); win.draw(txt);
                y += 18;
            }
        }

        float vpLeft = panelW / (float)win.getSize().x;
        sf::View mapV = win.getDefaultView();
        mapV.setViewport({ vpLeft,0,1 - vpLeft,1 });
        win.setView(mapV);
        drawMap(win, cities, path, minLat, maxLat, minLon, maxLon, dots);

        win.display();
        win.setView(win.getDefaultView());
    }
    return 0;
}
