#include "gldefs.h"
#include "gamestate.h"
#include "const.h"
#include "mygenerate.h"
#include <GL/glut.h>
#include <iostream>

// Game state
bool leftMouseDown = false;
float autoFireTimer = 0.0f;
float AUTO_FIRE_INTERVAL = 0.1f;
GameState currentGameState = STATE_LOADING;
int windowWidth = 1280;
int windowHeight = 720;
float gameTimeRemaining = GAME_DURATION;
int playerHealth = 100;
const int MAX_PLAYER_HEALTH = 100;
int playerScore = 0;
int killStreak = 0;
bool gameOver = false;
bool texturesAllValid = false;
int robotSpawnTimer = 0;
bool resourcesLoaded = false;
int currentLoadingStep = 0;
float loadingProgress = 0.0f;

// Shadow variables
bool shadowsEnabled = true;  // Default shadow aktif
GLuint shadowMapTexture = 0;
const int SHADOW_MAP_SIZE = 1024;
float lightPosition[4] = {CITY_SIZE/2.0f, 200.0f, CITY_SIZE/2.0f, 1.0f};
float lightProjectionMatrix[16];
float lightViewMatrix[16];

// Camera variables
float cameraX = CITY_SIZE / 2.0f;
float cameraY = 2.0f;
float cameraZ = CITY_SIZE / 2.0f;
float cameraYaw = 0.0f;
float cameraPitch = 0.0f;
float lookX = 0.0f, lookY = 0.0f, lookZ = -1.0f;
int lastMouseX = windowWidth / 2;
int lastMouseY = windowHeight / 2;
bool firstMouse = true;
bool mouseCaptured = true;
bool keys[256] = {false};
float lastFrameTime = 0.0f;

// Minimap variables
float minimapScreenX, minimapScreenY, minimapScreenWidth, minimapScreenHeight;
float minimapWorldExtent;

// Game objects containers
std::vector<Building> buildings;
std::vector<Street> streets;
std::map<std::pair<int, int>, Intersection> intersections;
std::vector<StreetLight> streetLights;
std::vector<Car> cars;
std::vector<Robot> robots;
std::vector<Bullet> bullets;
MuzzleFlash muzzleFlashEffect = {0.0f};

// Texture IDs
GLuint groundTexture;
GLuint robotTexture;
GLuint skyboxTextures[6];
GLuint muzzleFlashTexture;

// Fungsi untuk mengatur ulang status permainan ke kondisi awal.
void resetGame() {
    playerHealth = MAX_PLAYER_HEALTH;
    playerScore = 0;
    killStreak = 0;
    gameTimeRemaining = GAME_DURATION;
    gameOver = false;

    bullets.clear();
    robots.clear();

    cameraX = CITY_SIZE / 2.0f;
    cameraY = 2.0f;
    cameraZ = CITY_SIZE / 2.0f;
    cameraYaw = 0.0f;
    cameraPitch = 0.0f;

    lookX = sin(cameraYaw * MPI / 180.0f) * cos(cameraPitch * MPI / 180.0f);
    lookY = sin(cameraPitch * MPI / 180.0f);
    lookZ = -cos(cameraYaw * MPI / 180.0f) * cos(cameraPitch * MPI / 180.0f);
    float mag = sqrt(lookX*lookX + lookY*lookY + lookZ*lookZ);
    if (mag > 0.001f) { lookX /= mag; lookY /= mag; lookZ /= mag; }

    firstMouse = true;

    createRobots();

    muzzleFlashEffect.lifetime = 0.0f;
    robotSpawnTimer = 0;

    std::cout << "Game Reset. Player Health: " << playerHealth << ", Time: " << gameTimeRemaining << std::endl;
}

// Fungsi untuk memulai sesi permainan baru.
void startGame() {
    resetGame();
    currentGameState = STATE_PLAYING;
    mouseCaptured = true;
    glutSetCursor(GLUT_CURSOR_NONE);
    firstMouse = true;

    if (windowWidth > 0 && windowHeight > 0) {
        glutWarpPointer(windowWidth / 2, windowHeight / 2);
        lastMouseX = windowWidth / 2;
        lastMouseY = windowHeight / 2;
    }
    std::cout << "Game Started! State changed to STATE_PLAYING" << std::endl;
}

// Fungsi untuk menginisialisasi pengaturan.
void initGL() {
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);

    GLfloat ambientLightModel[] = {0.45f, 0.35f, 0.25f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLightModel);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat mat_specular[] = {0.15f, 0.15f, 0.15f, 1.0f};
    GLfloat mat_shininess[] = {5.0f};
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    glEnable(GL_FOG);
    GLfloat fogColor[] = {0.7f, 0.55f, 0.35f, 1.0f};
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 150.0f);
    glFogf(GL_FOG_END, 1600.0f);

    if (mouseCaptured) {
        glutSetCursor(GLUT_CURSOR_NONE);
        glutWarpPointer(windowWidth / 2, windowHeight / 2);
    }
    lastFrameTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_MULTISAMPLE);

    glHint(GL_FOG_HINT, GL_NICEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

// Fungsi untuk mengatur viewport dan dimensi minimap.
void setupMinimapViewport() {
    minimapScreenWidth = windowWidth * MINIMAP_SIZE_FACTOR;
    minimapScreenHeight = minimapScreenWidth;
    float padding = windowWidth * MINIMAP_BORDER_FACTOR;
    minimapScreenX = windowWidth - minimapScreenWidth - padding;
    minimapScreenY = padding;

    minimapWorldExtent = CITY_SIZE;
}
