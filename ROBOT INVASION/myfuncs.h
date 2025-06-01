#ifndef MYFUNCS_H
#define MYFUNCS_H

#include <utility> 

// Gameplay functions
void spawnRobot();
void shootBullet();
void updateBullets(float deltaTime);
void updateRobots(float deltaTime);
void updateCars(float deltaTime);
void updatePlayerAndCamera(float deltaTime);
void checkCollisions(float deltaTime);
void checkPlayerCarCollision();
bool checkCarCollision(float nextX, float nextZ, float buffer);
std::pair<bool, int> checkCarCollisionDetailed(float nextX, float nextZ, float buffer);
bool checkBuildingCollision(float nextX, float nextZ, float buffer);
void setupPlayerLight();
void setupGoldenHourSunLight();

// Shadow functions
void initShadowMap();
void renderShadowMap();
void setupShadowMatrix();
void drawShadowedScene();
void cleanupShadowMap();

#endif 
