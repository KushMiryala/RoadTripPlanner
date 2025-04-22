# RoadTripPlanner

An SFML app that finds an optimized route between two GPS points and shows nearest landmarks.

## What you need:
CMake (3.20 +)
A C++17 compiler 
SFML 2.5 headers & libs on your system
Git (to clone)

## Build:
```bash
git clone https://github.com/<your‑username>/RoadTripPlanner.git
cd RoadTripPlanner
# create a separate build folder
mkdir build && cd build
# point CMake at your SFML install; adjust the path as needed
cmake .. -DSFML_DIR="C:/SFML-2.5.1/lib/cmake/SFML"
# build the app (Debug or Release)
cmake --build . --config Release



## Using the app:
Type in the starting latitude and starting longitude text boxes to get the intitial starting location.
Type in the ending latitude and ending longitude text boxes to get the final destination location.
Type in the "number of cities" textbox the number of cities you want to visit inbetween location 1 and location 2.
Click Compute.
Finally, Click a city name to see 5 nearby landmarks.
