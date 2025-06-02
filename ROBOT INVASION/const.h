#ifndef CONST_H
#define CONST_H

#include <GL/gl.h>
#include <cmath>

// Math constants
extern const float MPI;

// Game dimensions
extern const float CITY_SIZE;
extern const int NUM_BUILDINGS;
extern const float BUILDING_MIN_DISTANCE;
extern const float COLLISION_BUFFER;
extern const float STREET_WIDTH;
extern const float STREETLIGHT_SPACING;
extern const float CAR_SCALE_FACTOR;
extern const int MAX_ACTIVE_STREET_LIGHTS;
extern const float DESIRED_GROUND_TILE_WORLD_SIZE;
extern const float GAME_DURATION;
extern const float GROUND_Y_LEVEL;

// Bullet constants
extern const float BULLET_RADIUS;
extern const float BULLET_CASING_LENGTH;
extern const float BULLET_TIP_LENGTH;
extern GLfloat bulletCasingColor[];
extern GLfloat bulletTipColor[];

// Minimap constants
extern const float MINIMAP_SIZE_FACTOR;
extern const float MINIMAP_BORDER_FACTOR;

// Visual culling distances
extern const float STREETLIGHT_VISUAL_CULL_DIST_SQ;

// Player movement constants
extern const float MOVE_SPEED;
extern const float PITCH_LIMIT;
extern const float MOUSE_SENSITIVITY;

// Robot spawning
extern const int ROBOT_SPAWN_INTERVAL;

// Loading screen
extern const float LOADING_STEPS;

#endif 
