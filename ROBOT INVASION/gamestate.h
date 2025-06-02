#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <vector>
#include "mytypes.h"

// Game state enumeration
enum GameState {
    STATE_LOADING,
    STATE_HOME_SCREEN,
    STATE_PLAYING,
    STATE_GAME_OVER
};

// Global game state variables
extern bool leftMouseDown;
extern float autoFireTimer;
extern float AUTO_FIRE_INTERVAL;
extern GameState currentGameState;
extern int windowWidth;
extern int windowHeight;
extern float gameTimeRemaining;
extern int playerHealth;
extern const int MAX_PLAYER_HEALTH;
extern int playerScore;
extern int killStreak;
extern bool gameOver;
extern bool texturesAllValid;
extern int robotSpawnTimer;
extern bool resourcesLoaded;
extern int currentLoadingStep;
extern float loadingProgress;

// Camera variables
extern float cameraX;
extern float cameraY;
extern float cameraZ;
extern float cameraYaw;
extern float cameraPitch;
extern float lookX, lookY, lookZ;
extern int lastMouseX;
extern int lastMouseY;
extern bool firstMouse;
extern bool mouseCaptured;
extern bool keys[256];
extern float lastFrameTime;

// Shadow variables
extern bool shadowsEnabled;
extern GLuint shadowMapTexture;
extern const int SHADOW_MAP_SIZE;
extern float lightPosition[4];
extern float lightProjectionMatrix[16];
extern float lightViewMatrix[16];

// Minimap variables
extern float minimapScreenX, minimapScreenY, minimapScreenWidth, minimapScreenHeight;
extern float minimapWorldExtent;

// Game objects containers
extern std::vector<Building> buildings;
extern std::vector<Street> streets;
extern std::map<std::pair<int, int>, Intersection> intersections;
extern std::vector<StreetLight> streetLights;
extern std::vector<Car> cars;
extern std::vector<Robot> robots;
extern std::vector<Bullet> bullets;
extern MuzzleFlash muzzleFlashEffect;

// Texture IDs
extern GLuint groundTexture;
extern GLuint robotTexture;
extern GLuint skyboxTextures[6];
extern GLuint muzzleFlashTexture;

// Game state functions
void resetGame();
void startGame();
void initGL();
void setupMinimapViewport();

#endif // GAMESTATE_H
