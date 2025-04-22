#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include "file_utils.h"

// Hold screen‑space position + index into original cities vector
struct CityDot { sf::Vector2f pos; int cityIdx; };

/*
 Draw the route (green poly‑line) and only the cities that belong to the
 "path" vector.  The function also fills the dotsOut array with the screen
 coordinates of each city so that main.cpp can do hit‑testing.
 */
void drawMap(sf::RenderWindow& win,
             const std::vector<City>& cities,
             const std::vector<int>&  path,
             double minLat, double maxLat,
             double minLon, double maxLon,
             std::vector<CityDot>& dotsOut);
