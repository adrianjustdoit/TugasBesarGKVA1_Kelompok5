#ifndef MYTYPES_H
#define MYTYPES_H

#include <vector>
#include <map>
#include <utility>
#include <GL/gl.h>

// Image structure for texture loading
struct Image {
    unsigned long sizeX;
    unsigned long sizeY;
    char *data;
    bool hasAlpha;
};

// Game objects structures
struct Building {
    float x, z;
    float width, depth, height;
    float r, g, b;
    int numFloors;
    int seed;
};

struct Street {
    float x1, z1, x2, z2;
    float width;
    bool isMainRoad;
};

struct Intersection {
    float x, z;
    int count;
    std::vector<int> connectedStreets;
};

struct StreetLight {
    float x, y, z;
};

struct Car {
    float x, y, z;
    float direction;
    float r, g, b;
    float speed;
    int currentStreetIndex;
    float progressOnStreet;
    bool movingForward;
};

struct Robot {
    float rx, ry, rz;
    float size;
    int hp;
    float moveSpeed;
    bool alive;
    float rotationY;
    int state;
    float stateTimer;
    float targetX, targetZ;
};

struct Bullet {
    float bx, by, bz;
    float dx, dy, dz;
    float speed;
    float lifetime;
};

struct MuzzleFlash {
    float lifetime;
};

struct Button {
    float x, y, width, height;
    std::string text;
    bool isHovered;

    Button(float _x, float _y, float _w, float _h, std::string _text);
    void updatePosition(float winWidth, float winHeight, float newX, float newY);
    void draw();
    bool isClicked(int mouse_x_glut, int mouse_y_glut, int winHeight);
};

// Global UI buttons
extern Button startGameButton;
extern Button retryButton;
extern Button quitButtonHomeScreen;
extern Button quitButtonGameOver;

#endif
