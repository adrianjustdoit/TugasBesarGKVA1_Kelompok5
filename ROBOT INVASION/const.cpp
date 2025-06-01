#include "const.h"

// Math constants
const float MPI = acos(-1.0f);

// Game dimensions
const float CITY_SIZE = 2500.0f;
const int NUM_BUILDINGS = 350;
const float BUILDING_MIN_DISTANCE = 12.0f;
const float COLLISION_BUFFER = 0.8f;
const float STREET_WIDTH = 30.0f;
const float STREETLIGHT_SPACING = 180.0f;
const float CAR_SCALE_FACTOR = 3.3f;
const int MAX_ACTIVE_STREET_LIGHTS = 3;
const float DESIRED_GROUND_TILE_WORLD_SIZE = 4.0f;
const float GAME_DURATION = 5 * 60.0f;
const float GROUND_Y_LEVEL = 0.0f;

// Bullet constants
const float BULLET_RADIUS = 0.04f;
const float BULLET_CASING_LENGTH = 0.25f;
const float BULLET_TIP_LENGTH = 0.1f;
GLfloat bulletCasingColor[] = {0.6f, 0.45f, 0.2f, 1.0f};
GLfloat bulletTipColor[] = {0.4f, 0.4f, 0.4f, 1.0f};

// Minimap constants
const float MINIMAP_SIZE_FACTOR = 0.16f;
const float MINIMAP_BORDER_FACTOR = 0.02f;

// Visual culling distances
const float STREETLIGHT_VISUAL_CULL_DIST_SQ = 900.0f * 900.0f;

// Player movement constants
const float MOVE_SPEED = 30.0f;
const float PITCH_LIMIT = 88.0f;
const float MOUSE_SENSITIVITY = 0.12f;

// Robot spawning
const int ROBOT_SPAWN_INTERVAL = 3500;

// Loading screen
const float LOADING_STEPS = 5.0f;