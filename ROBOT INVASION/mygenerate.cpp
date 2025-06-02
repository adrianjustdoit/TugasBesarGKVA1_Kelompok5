#include "gldefs.h"
#include "myfuncs.h"
#include "mygenerate.h"
#include "gamestate.h"
#include "const.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <sstream>

// Fungsi untuk menentukan apakah sebuah jendela gedung menyala berdasarkan seed.
bool isWindowLit(int seed, int floor, int window, int side) {
    unsigned int hashValue = (seed + floor * 131 + window * 53 + side * 23);
    srand(hashValue);
    return (rand() % 100) < 45;
}

// Fungsi untuk mendapatkan koordinat grid dari posisi dunia.
std::pair<int, int> getGridCoordinates(float x, float z) {
    int gridSize = (int)(STREET_WIDTH * 2.8f);
    int gridX = (int)(x / gridSize);
    int gridZ = (int)(z / gridSize);
    return std::make_pair(gridX, gridZ);
}

// Fungsi untuk menghitung jarak dari sebuah titik ke segmen garis.
float distancePointToLine(float x, float z, float x1, float z1, float x2, float z2) {
    float dx_line = x2 - x1;
    float dz_line = z2 - z1;
    float length_squared = dx_line * dx_line + dz_line * dz_line;

    if (length_squared < 0.0001f) {
        float px = x - x1;
        float pz = z - z1;
        return sqrt(px * px + pz * pz);
    }

    float t = ((x - x1) * dx_line + (z - z1) * dz_line) / length_squared;

    if (t < 0.0f) {
        float px = x - x1;
        float pz = z - z1;
        return sqrt(px * px + pz * pz);
    }
    else if (t > 1.0f) {
        float px = x - x2;
        float pz = z - z2;
        return sqrt(px * px + pz * pz);
    }
    else {
        float projX = x1 + t * dx_line;
        float projZ = z1 + t * dz_line;
        float px = x - projX;
        float pz = z - projZ;
        return sqrt(px * px + pz * pz);
    }
}

// Fungsi untuk menghitung jarak dari sebuah titik ke jalan terdekat.
float distanceToStreet(float x, float z) {
    if (streets.empty()) return CITY_SIZE;

    float minDistance = CITY_SIZE * 2.0f;
    for (const auto& street : streets) {
        float dist = distancePointToLine(x, z, street.x1, street.z1, street.x2, street.z2) - street.width / 2.0f;
        if (dist < minDistance) minDistance = dist;
    }

    return std::max(0.0f, minDistance);
}

// Fungsi untuk memeriksa apakah sebuah bangunan tumpang tindih dengan jalan.
bool buildingOverlapsStreet(float x, float z, float width, float depth) {
    float halfWidth = width / 2.0f;
    float halfDepth = depth / 2.0f;
    float checkPoints[5][2] = {
        {x, z},
        {x - halfWidth, z - halfDepth},
        {x + halfWidth, z - halfDepth},
        {x - halfWidth, z + halfDepth},
        {x + halfWidth, z + halfDepth}
    };

    float placementBufferFromStreet = 4.0f;
    for (int i = 0; i < 5; ++i) {
        if (distanceToStreet(checkPoints[i][0], checkPoints[i][1]) < placementBufferFromStreet)
            return true;
    }

    return false;
}

// Fungsi untuk menambahkan jalan baru ke dalam daftar jalan kota.
bool addStreet(float x1, float z1, float x2, float z2, float width, bool isMainRoad) {
    float dx_add_street = x2 - x1;
    float dz_add_street = z2 - z1;
    float length = sqrt(dx_add_street * dx_add_street + dz_add_street * dz_add_street);

    if (length < STREET_WIDTH * 2.5f) return false;

    Street street_obj;
    street_obj.x1 = x1;
    street_obj.z1 = z1;
    street_obj.x2 = x2;
    street_obj.z2 = z2;
    street_obj.width = width;
    street_obj.isMainRoad = isMainRoad;

    streets.push_back(street_obj);
    size_t newStreetIndex = streets.size() - 1;

    std::pair<int, int> grid1 = getGridCoordinates(x1, z1);
    std::pair<int, int> grid2 = getGridCoordinates(x2, z2);

    if (intersections.find(grid1) == intersections.end())
        intersections[grid1] = {x1, z1, 0, {}};

    intersections[grid1].count++;
    intersections[grid1].connectedStreets.push_back(newStreetIndex);

    if (grid1 != grid2) {
        if (intersections.find(grid2) == intersections.end())
            intersections[grid2] = {x2, z2, 0, {}};

        intersections[grid2].count++;
        intersections[grid2].connectedStreets.push_back(newStreetIndex);
    }

    return true;
}

// Fungsi untuk menghasilkan tata letak kota (jalan dan persimpangan).
void generateCityLayout() {
    streets.clear();
    intersections.clear();

    const float STREET_SPACING = 80.0f;  // Jarak antar jalan
    const float STREET_WIDTH_VAL = 8.0f; // Lebar jalan (menggunakan nama variabel berbeda dari konstanta global)

    int numHorizontalStreets = (int)((CITY_SIZE - STREET_SPACING) / STREET_SPACING) + 1;
    int numVerticalStreets = (int)((CITY_SIZE - STREET_SPACING) / STREET_SPACING) + 1;

    numHorizontalStreets = std::max(numHorizontalStreets, 3);
    numVerticalStreets = std::max(numVerticalStreets, 3);

    for (int i = 0; i < numHorizontalStreets; i++) {
        float z = STREET_SPACING + i * STREET_SPACING;

        if (z > CITY_SIZE - STREET_SPACING) {
            z = CITY_SIZE - STREET_SPACING;
        }

        Street horizontalStreet;
        horizontalStreet.x1 = STREET_WIDTH_VAL / 2.0f;
        horizontalStreet.z1 = z;
        horizontalStreet.x2 = CITY_SIZE - STREET_WIDTH_VAL / 2.0f;
        horizontalStreet.z2 = z;
        horizontalStreet.width = STREET_WIDTH_VAL;

        streets.push_back(horizontalStreet);
    }

    for (int i = 0; i < numVerticalStreets; i++) {
        float x = STREET_SPACING + i * STREET_SPACING;

        if (x > CITY_SIZE - STREET_SPACING) {
            x = CITY_SIZE - STREET_SPACING;
        }

        Street verticalStreet;
        verticalStreet.x1 = x;
        verticalStreet.z1 = STREET_WIDTH_VAL / 2.0f;
        verticalStreet.x2 = x;
        verticalStreet.z2 = CITY_SIZE - STREET_WIDTH_VAL / 2.0f;
        verticalStreet.width = STREET_WIDTH_VAL;

        streets.push_back(verticalStreet);
    }

    for (int h = 0; h < numHorizontalStreets; h++) {
        for (int v = 0; v < numVerticalStreets; v++) {
            float intersectionX = STREET_SPACING + v * STREET_SPACING;
            float intersectionZ = STREET_SPACING + h * STREET_SPACING;

            if (intersectionX > CITY_SIZE - STREET_SPACING ||
                intersectionZ > CITY_SIZE - STREET_SPACING) {
                continue;
            }

            Intersection intersection;
            intersection.x = intersectionX;
            intersection.z = intersectionZ;

            std::pair<int, int> key = std::make_pair((int)intersectionX, (int)intersectionZ);
            intersections[key] = intersection;
        }
    }

    printf("Generated %zu streets and %zu intersections\n", streets.size(), intersections.size());
}

// Fungsi untuk menghasilkan bangunan-bangunan di kota.
void generateBuildings() {
    buildings.clear();
    int attempts = 0;
    int maxAttempts = NUM_BUILDINGS * 25;

    const int NUM_COLORS = 12;
    float buildingColors[NUM_COLORS][3] = {
        {0.20f, 0.20f, 0.25f}, {0.22f, 0.18f, 0.22f}, {0.25f, 0.15f, 0.15f}, {0.18f, 0.20f, 0.18f},
        {0.28f, 0.22f, 0.15f}, {0.15f, 0.22f, 0.28f}, {0.20f, 0.20f, 0.20f}, {0.18f, 0.18f, 0.22f},
        {0.22f, 0.22f, 0.18f}, {0.16f, 0.16f, 0.19f}, {0.25f, 0.25f, 0.30f}, {0.30f, 0.20f, 0.20f}
    };

    while (buildings.size() < NUM_BUILDINGS && attempts < maxAttempts) {
        attempts++;
        Building building;

        building.x = (float)(rand() % (int)(CITY_SIZE - 40.0f)) + 20.0f;
        building.z = (float)(rand() % (int)(CITY_SIZE - 40.0f)) + 20.0f;
        building.width = 20.0f + (rand() % 35);
        building.depth = 20.0f + (rand() % 35);
        building.height = 35.0f + (rand() % 150);

        float distToCenter = sqrt(pow(building.x - CITY_SIZE/2.0f, 2) + pow(building.z - CITY_SIZE/2.0f, 2));
        float centerFactor = 1.0f - (distToCenter / (CITY_SIZE * 0.5f));

        if (centerFactor > 0) building.height += centerFactor * 120.0f;
        if (building.height < 25) building.height = 25;

        int colorIndex = rand() % NUM_COLORS;
        building.r = buildingColors[colorIndex][0];
        building.g = buildingColors[colorIndex][1];
        building.b = buildingColors[colorIndex][2];

        building.numFloors = std::max(1, (int)(building.height / 4.5f));
        building.seed = rand();

        if (buildingOverlapsStreet(building.x, building.z, building.width, building.depth)) continue;

        bool tooClose = false;
        for (const auto& existingBuilding : buildings) {
            float dx_bld_gen = building.x - existingBuilding.x;
            float dz_bld_gen = building.z - existingBuilding.z;
            float distance = sqrt(dx_bld_gen * dx_bld_gen + dz_bld_gen * dz_bld_gen);

            float minSep = ((building.width + existingBuilding.width) / 2.0f + BUILDING_MIN_DISTANCE +
                          (building.depth + existingBuilding.depth) / 2.0f + BUILDING_MIN_DISTANCE) / 2.0f * 0.9f;

            if (distance < minSep) {
                tooClose = true;
                break;
            }
        }

        if (!tooClose) buildings.push_back(building);
    }

    std::cout << "Buildings generated: " << buildings.size() << std::endl;
}

// Fungsi untuk menghasilkan mobil-mobil di jalanan kota.
void generateCars() {
    cars.clear();
    if (streets.empty()) return;

    int numCarsToGenerate = 40;
    for (int k = 0; k < numCarsToGenerate; ++k) {
        int streetIdx = rand() % streets.size();
        const auto& street = streets[streetIdx];

        float dx_car_gen = street.x2 - street.x1;
        float dz_car_gen = street.z2 - street.z1;
        float length = sqrt(dx_car_gen * dx_car_gen + dz_car_gen * dz_car_gen);

        if (length < 25.0f) continue;

        float t = (float)(rand() % 1000) / 1000.0f;
        float carX_pos = street.x1 + t * dx_car_gen;
        float carZ_pos = street.z1 + t * dz_car_gen;

        float baseDirection = atan2(dz_car_gen, dx_car_gen);
        float laneOffset = street.width * 0.28f;
        float perpX = -(dz_car_gen / length);
        float perpZ = (dx_car_gen / length);

        float currentDirection = baseDirection;
        bool movingForwardOnStreet = (rand() % 2 == 0);

        if (movingForwardOnStreet) {
            carX_pos += perpX * laneOffset;
            carZ_pos += perpZ * laneOffset;
        } else {
            carX_pos -= perpX * laneOffset;
            carZ_pos -= perpZ * laneOffset;
            currentDirection += MPI;
        }

        while(currentDirection >= 2.0f * MPI) currentDirection -= 2.0f * MPI;
        while(currentDirection < 0) currentDirection += 2.0f * MPI;

        if (carX_pos >= 0 && carX_pos <= CITY_SIZE && carZ_pos >= 0 && carZ_pos <= CITY_SIZE) {
            bool tooCloseToOtherCar = false;
            for(const auto& existingCar : cars) {
                float dcx = carX_pos - existingCar.x;
                float dcz = carZ_pos - existingCar.z;
                if (dcx * dcx + dcz * dcz < 12.0f * 12.0f) {
                    tooCloseToOtherCar = true;
                    break;
                }
            }

            if (!tooCloseToOtherCar) {
                cars.push_back({
                    carX_pos, 0.0f, carZ_pos, currentDirection,
                    0.15f + ((rand() % 50) / 100.0f),
                    0.15f + ((rand() % 50) / 100.0f),
                    0.15f + ((rand() % 50) / 100.0f),
                    10.0f + (rand() % 1000) / 100.0f,
                    streetIdx, t, movingForwardOnStreet
                });
            }
        }
    }

    std::cout << "Cars generated: " << cars.size() << std::endl;
}

// Fungsi untuk membuat robot-robot awal dalam permainan.
void createRobots() {
    robots.clear();

    for (int i = 0; i < 8; ++i) {
        float spawnX, spawnZ;
        bool validPosition = false;
        int attempts = 0;

        while(!validPosition && attempts < 100) {
            spawnX = (float)(rand() % (int)CITY_SIZE);
            spawnZ = (float)(rand() % (int)CITY_SIZE);

            if (!checkBuildingCollision(spawnX, spawnZ, 6.0f) && distanceToStreet(spawnX, spawnZ) > 12.0f) {
                validPosition = true;
            }
            attempts++;
        }

        if(validPosition) {
            robots.push_back({
                spawnX, 0.0f, spawnZ,
                2.2f + (rand() % 10) / 10.0f,
                6 + (rand() % 4),
                3.5f + (rand() % 20) / 10.0f,
                true, (float)(rand() % 360), 0, 0.0f, spawnX, spawnZ
            });
        }
    }

    if(robots.empty()) {
        robots.push_back({
            CITY_SIZE / 2 + 25.0f, 0.0f, CITY_SIZE / 2 + 25.0f,
            2.5f, 7, 4.0f, true, 0.0f, 0, 0.0f,
            CITY_SIZE / 2 + 25.0f, CITY_SIZE / 2 + 25.0f
        });
    }
}
