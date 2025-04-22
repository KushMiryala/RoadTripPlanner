

#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#endif //FILE_UTILS_H

#pragma once
#include <vector>
#include <string>
#include "geo_utils.h"

std::vector<City> readCitiesWithinBox(const std::string& filename, double minLat, double maxLat, double minLon, double maxLon);
std::vector<Landmark> readLandmarksWithinBox(const std::string& filename, double minLat, double maxLat, double minLon, double maxLon);

