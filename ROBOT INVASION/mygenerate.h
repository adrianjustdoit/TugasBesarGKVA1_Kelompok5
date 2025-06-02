#ifndef MYGENERATE_H
#define MYGENERATE_H

#include "mytypes.h"
#include <utility>

// City generation functions
bool isWindowLit(int seed, int floor, int window, int side);
std::pair<int, int> getGridCoordinates(float x, float z);
float distancePointToLine(float x, float z, float x1, float z1, float x2, float z2);
bool buildingOverlapsStreet(float x, float z, float width, float depth);
float distanceToStreet(float x, float z);
bool addStreet(float x1, float z1, float x2, float z2, float width, bool isMainRoad);
void generateCityLayout();
void generateBuildings();
void generateCars();
void createRobots();

#endif // MYGENERATE_H