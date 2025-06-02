#ifndef MYDRAW_H
#define MYDRAW_H

#include "mytypes.h"

void drawLoadingScreen();
void drawTexturedCube(float width, float height, float depth, GLuint textureID, float texScaleS = 1.0f, float texScaleT = 1.0f);
void drawPlayerArmAndGun();
void drawMuzzleFlash();
void drawRobot(const Robot& robot);
void drawRobots();
void drawBullets();
void drawTexturedGround();
void drawText(float x, float y, const char *text);
void drawCrosshair();
void drawBuilding(const Building& building);
void drawStreet(const Street& street, bool printDebug = false);
void drawIntersection(const Intersection& intersection);
void drawStreetLight(const StreetLight& light, bool activateOpenGLightSource, GLenum glLightID);
void drawCar(const Car& car);
void drawMinimap();

#endif 
