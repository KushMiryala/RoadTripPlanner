#include "ui.h"
#include <cmath>

static sf::Vector2f geoToScreen(double lat, double lon,
                                double minLat, double maxLat,
                                double minLon, double maxLon,
                                float w, float h)
{
    float x = static_cast<float>((lon - minLon) / (maxLon - minLon) * w);
    float y = static_cast<float>((maxLat - lat)  / (maxLat - minLat)  * h);
    return {x, y};
}

void drawMap(sf::RenderWindow& win,
             const std::vector<City>& cities,
             const std::vector<int>&  path,
             double minLat, double maxLat,
             double minLon, double maxLon,
             std::vector<CityDot>& dotsOut)
{
    dotsOut.clear();
    if(path.empty()) return;

    auto sz = win.getView().getSize();

    // — green route line —
    sf::VertexArray line(sf::LineStrip, path.size());
    for(std::size_t i=0;i<path.size();++i){
        const City& c = cities[path[i]];
        sf::Vector2f p = geoToScreen(c.lat,c.lon,minLat,maxLat,minLon,maxLon,sz.x,sz.y);
        line[i].position = p; line[i].color = sf::Color::Green;
        dotsOut.push_back({p, (int)path[i]});
    }
    win.draw(line);

    // — white dots + labels for each city on the path —
    sf::Font font; font.loadFromFile("C:/Windows/Fonts/arial.ttf");
    sf::Text label("", font, 12); label.setFillColor(sf::Color::Cyan);
    for(std::size_t i=0;i<dotsOut.size(); ++i){
        sf::CircleShape dot(4); dot.setFillColor(sf::Color::White);
        dot.setOrigin(4,4);
        dot.setPosition(dotsOut[i].pos); win.draw(dot);

        const City& c = cities[dotsOut[i].cityIdx];
        label.setString(c.name);
        label.setPosition(dotsOut[i].pos.x + 6, dotsOut[i].pos.y - 10);
        win.draw(label);
    }
}