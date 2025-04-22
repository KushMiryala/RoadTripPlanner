
#include "file_utils.h"
#include <fstream>
#include <iterator>
#include <sstream>
#include <vector>

static bool inBox(double lat, double lon,
                  double minLat, double maxLat,
                  double minLon, double maxLon)
{
    return lat >= minLat && lat <= maxLat &&
           lon >= minLon && lon <= maxLon;
}

// Handles four‑column “trimmed” lines and silently skips bad ones.

std::vector<City> readCitiesWithinBox(const std::string& file,
                                      double minLat, double maxLat,
                                      double minLon, double maxLon)
{
    std::ifstream in(file);
    std::vector<City> out;
    std::string line;

    while (std::getline(in, line)) {
        if (line.empty()) continue;

        // Tokenise on any whitespace
        std::istringstream iss(line);
        std::vector<std::string> tok{ std::istream_iterator<std::string>{iss},
                                      std::istream_iterator<std::string>{} };
        if (tok.size() < 4) continue;               

        // Last 3 tokens are lon/lat/class
        // the rest form the name
        std::string  cls  = tok.back(); tok.pop_back();
        double       lon  = std::stod(tok.back()); tok.pop_back();
        double       lat  = std::stod(tok.back()); tok.pop_back();
        std::string  name = tok.empty() ? "" : tok[0];
        for (size_t i = 1; i < tok.size(); ++i) {
            name += ' ' + tok[i];
        }

        if (cls != "P") continue;                    // keep only towns
        if (!inBox(lat, lon, minLat, maxLat, minLon, maxLon)) continue;

        out.push_back({name, /*pop=*/0, lat, lon}); // pop unknown ⇒ 0
    }
    return out;
}

std::vector<Landmark> readLandmarksWithinBox(const std::string& file,
                                             double minLat, double maxLat,
                                             double minLon, double maxLon)
{
    std::ifstream in(file);
    std::vector<Landmark> out;
    std::string line;

    while (std::getline(in, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::vector<std::string> tok{ std::istream_iterator<std::string>{iss},
                                      std::istream_iterator<std::string>{} };
        if (tok.size() < 4) continue;

        std::string  cls  = tok.back(); tok.pop_back();
        double       lon  = std::stod(tok.back()); tok.pop_back();
        double       lat  = std::stod(tok.back()); tok.pop_back();
        std::string  name = tok.empty() ? "" : tok[0];
        for (size_t i = 1; i < tok.size(); ++i) name += ' ' + tok[i];

        if (cls != "L") continue;                    // keep only landmarks
        if (!inBox(lat, lon, minLat, maxLat, minLon, maxLon)) continue;

        out.push_back({name, lat, lon});
    }
    return out;
}
