#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "const.h"
#include "mytypes.h"
#include "gamestate.h"
#include "mytexture.h"
#include "mydraw.h"
#include "mygenerate.h"
#include "myfuncs.h"

void display();
void reshape(int width, int height);
void keyboard(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);
void mouseMotion(int mx, int my);
void mouseButton(int button, int state, int x, int y);
void gameLoopTimer(int value);
void loadingTimerCallback(int value);

// Fungsi utama program
int main(int argc, char **argv) {
    std::cout << "Program starting..." << std::endl;
    srand(time(NULL));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
    glutInitWindowPosition(50, 50);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Robot Invasion - 3D FPS Game");

    glutDisplayFunc(display);
    PlaySound("TESSOUND.wav", NULL, SND_ASYNC | SND_LOOP);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutPassiveMotionFunc(mouseMotion);
    glutMotionFunc(mouseMotion);
    glutMouseFunc(mouseButton);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    currentGameState = STATE_LOADING;
    mouseCaptured = false;
    glutSetCursor(GLUT_CURSOR_INHERIT);

    glutTimerFunc(100, loadingTimerCallback, 0);

    glutTimerFunc(16, gameLoopTimer, 0);

    glutMainLoop();
    return 0;
}

// Fungsi untuk menggambar semua elemen visual game.
void display() {
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float deltaTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;

    if (deltaTime > 0.1f) deltaTime = 0.1f;
    if (deltaTime <= 0.0001f) deltaTime = 0.0001f;

    if (currentGameState == STATE_PLAYING && !gameOver) {
        if (leftMouseDown && mouseCaptured) {
            autoFireTimer += deltaTime;
            if (autoFireTimer >= AUTO_FIRE_INTERVAL) {
            	playShootSound(); 
                shootBullet();
                autoFireTimer = 0.0f;
            }
        }

        updatePlayerAndCamera(deltaTime);
        updateRobots(deltaTime);
        updateBullets(deltaTime);
        updateCars(deltaTime);
        checkCollisions(deltaTime);

        gameTimeRemaining -= deltaTime;
        if (gameTimeRemaining <= 0.0f) {
            gameTimeRemaining = 0.0f;
            gameOver = true;
        }

        if (playerHealth <= 0) {
            gameOver = true;
        }

        if (gameOver) {
            currentGameState = STATE_GAME_OVER;
            mouseCaptured = false;
            glutSetCursor(GLUT_CURSOR_INHERIT);
            std::cout << "DISPLAY: Game Over triggered! State: STATE_GAME_OVER" << std::endl;
        }
    }
    if ((currentGameState == STATE_PLAYING || currentGameState == STATE_GAME_OVER) && shadowsEnabled) {
	    setupShadowMatrix();
	    renderShadowMap();
	}

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (currentGameState == STATE_LOADING) {
        drawLoadingScreen();
    }
    else if (currentGameState == STATE_HOME_SCREEN) {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, windowWidth, 0, windowHeight);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_POLYGON_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glDisable(GL_FOG);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        const char* titleText = "ROBOT INVASION";
        float titleWidth = 0;
        for(size_t i = 0; i < strlen(titleText); ++i)
            titleWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, titleText[i]);
        drawText(windowWidth / 2.0f - titleWidth / 2.0f, windowHeight * 0.7f, titleText);

        startGameButton.updatePosition(windowWidth, windowHeight,
                                     windowWidth / 2.0f - startGameButton.width / 2.0f,
                                     windowHeight * 0.5f - startGameButton.height / 2.0f);
        startGameButton.draw();

        quitButtonHomeScreen.updatePosition(windowWidth, windowHeight,
                                          windowWidth / 2.0f - quitButtonHomeScreen.width / 2.0f,
                                          startGameButton.y - quitButtonHomeScreen.height - 20.0f);
        quitButtonHomeScreen.draw();

        glPopAttrib();
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }
    else if (currentGameState == STATE_PLAYING || currentGameState == STATE_GAME_OVER) {
        glPushAttrib(GL_ALL_ATTRIB_BITS);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        glEnable(GL_FOG);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float farClip = CITY_SIZE * 0.8f;
        gluPerspective(55.0f, (float)windowWidth / (float)windowHeight, 0.1f, farClip);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(cameraX, cameraY, cameraZ,
                 cameraX + lookX, cameraY + lookY, cameraZ + lookZ,
                 0.0f, 1.0f, 0.0f);

        setupPlayerLight();
        setupGoldenHourSunLight();
        setupMinimapViewport();

        for (GLenum i = GL_LIGHT1; i <= GL_LIGHT1 + MAX_ACTIVE_STREET_LIGHTS - 1; ++i) {
            if (i != GL_LIGHT6) glDisable(i);
        }

        std::vector<std::pair<float, const StreetLight*>> nearbyLightsForActivation;
        for (const auto& light : streetLights) {
            float dx_sl_disp = light.x - cameraX;
            float dz_sl_disp = light.z - cameraZ;
            float distSq_sl_disp = dx_sl_disp * dx_sl_disp + dz_sl_disp * dz_sl_disp;

            if (distSq_sl_disp < 700.0f * 700.0f) {
                nearbyLightsForActivation.push_back({distSq_sl_disp, &light});
            }
        }
        std::sort(nearbyLightsForActivation.begin(), nearbyLightsForActivation.end());

        glDisable(GL_POLYGON_OFFSET_FILL);
        int activatedLightSources = 0;
        for (const auto& light : streetLights) {
            bool activateAsOpenGLSource = false;
            GLenum openGLLightIDToUse = 0;

            for (size_t i = 0; i < nearbyLightsForActivation.size() &&
                 activatedLightSources < MAX_ACTIVE_STREET_LIGHTS; ++i) {
                if (nearbyLightsForActivation[i].second == &light) {
                    activateAsOpenGLSource = true;
                    openGLLightIDToUse = GL_LIGHT1 + activatedLightSources;
                    activatedLightSources++;
                    break;
                }
            }

            drawStreetLight(light, activateAsOpenGLSource, openGLLightIDToUse);
        }

        glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_TEXTURE_BIT | GL_LIGHTING_BIT);
        glDisable(GL_CULL_FACE);
        glDisable(GL_TEXTURE_2D);
        drawTexturedGround();
        if (shadowsEnabled) {
		    drawShadowedScene();
		}
        glPopAttrib();

        glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_TEXTURE_BIT | GL_LIGHTING_BIT);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-1.0f, -1.0f);

        for (const auto& street : streets) drawStreet(street, false);
        for (const auto& pair_is : intersections) drawIntersection(pair_is.second);

        glDisable(GL_POLYGON_OFFSET_FILL);

        for (const auto& car : cars) drawCar(car);
        for (const auto& building : buildings) drawBuilding(building);

        drawRobots();
        drawBullets();

        if (currentGameState == STATE_PLAYING) {
            glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_FOG_BIT);
            glClear(GL_DEPTH_BUFFER_BIT);
            glDisable(GL_FOG);
            drawPlayerArmAndGun();
            drawMuzzleFlash();
            glPopAttrib();

            drawMinimap();
            drawCrosshair();
        }

        glPopAttrib();

        glPushAttrib(GL_ENABLE_BIT | GL_TRANSFORM_BIT | GL_CURRENT_BIT |
                    GL_DEPTH_BUFFER_BIT | GL_POLYGON_BIT);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, windowWidth, 0, windowHeight);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glDisable(GL_FOG);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        char hudText[100];
        sprintf(hudText, "Shadows: %s", shadowsEnabled ? "ON" : "OFF");
		drawText(20, windowHeight - 105, hudText);

        sprintf(hudText, "Health: %d", playerHealth);
        drawText(20, windowHeight - 30, hudText);

        sprintf(hudText, "Robots: %zu", robots.size());
        drawText(20, windowHeight - 55, hudText);

        sprintf(hudText, "Score: %d", playerScore);
        drawText(20, windowHeight - 80, hudText);
        drawText(20, 130, "WASD: Move");
	    drawText(20, 110, "Mouse: Look Around");
	    drawText(20, 90, "Left Click: Shoot");
	    drawText(20, 70, "V: Toggle Shadows");
	    drawText(20, 50, "ESC: Exit Game");

        if (killStreak > 1) {
            sprintf(hudText, "Streak: x%d", killStreak);
            drawText(windowWidth - 150, windowHeight - 30, hudText);
        }

        int minutes = (int)(gameTimeRemaining / 60.0f);
        int seconds = (int)gameTimeRemaining % 60;
        if (seconds < 0) seconds = 0;
        if (minutes < 0) minutes = 0;

        sprintf(hudText, "Time: %02d:%02d", minutes, seconds);
        float timeTextWidth = 0;
        for(size_t i_s = 0; i_s < strlen(hudText); ++i_s)
            timeTextWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, hudText[i_s]);
        drawText(windowWidth / 2.0f - timeTextWidth / 2.0f, windowHeight - 30, hudText);

        if (currentGameState == STATE_PLAYING) {
            drawMinimap();
            drawCrosshair();
        }
        else if (currentGameState == STATE_GAME_OVER) {
            char endHudText[100];
            sprintf(endHudText, "GAME OVER!");
            float textWidthEst = 0;
            for(size_t i_s = 0; i_s < strlen(endHudText); ++i_s)
                textWidthEst += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, endHudText[i_s]);
            drawText(windowWidth / 2.0f - textWidthEst / 2.0f, windowHeight / 2.0f + 60, endHudText);

            sprintf(endHudText, "Final Score: %d", playerScore);
            textWidthEst = 0;
            for(size_t i_s = 0; i_s < strlen(endHudText); ++i_s)
                textWidthEst += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, endHudText[i_s]);
            drawText(windowWidth / 2.0f - textWidthEst / 2.0f, windowHeight / 2.0f + 30, endHudText);

            retryButton.updatePosition(windowWidth, windowHeight,
                                     windowWidth / 2.0f - retryButton.width / 2.0f,
                                     windowHeight / 2.0f - retryButton.height / 2.0f - 20);
            retryButton.draw();

            quitButtonGameOver.updatePosition(windowWidth, windowHeight,
                                           windowWidth / 2.0f - quitButtonGameOver.width / 2.0f,
                                           retryButton.y - quitButtonGameOver.height - 20.0f);
            quitButtonGameOver.draw();
        }

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);

        glPopAttrib();
    }

    glutSwapBuffers();
}

// Fungsi yang untuk resize window
void reshape(int w, int h) {
    if (h == 0) h = 1;

    windowWidth = w;
    windowHeight = h;

    glViewport(0, 0, w, h);

    if (mouseCaptured) {
        glutWarpPointer(windowWidth / 2, windowHeight / 2);
        lastMouseX = windowWidth / 2;
        lastMouseY = windowHeight / 2;
    }
}

// Fungsi yang menangani input keyboard saat tombol ditekan.
void keyboard(unsigned char key, int x, int y) {
    if (key == 27 /* ESC */) {
        if (currentGameState == STATE_PLAYING && mouseCaptured) {
            mouseCaptured = false;
            glutSetCursor(GLUT_CURSOR_INHERIT);
        }
        else if (currentGameState == STATE_HOME_SCREEN || currentGameState == STATE_GAME_OVER) {
        }
        else {
            exit(0);
        }
    }

    if (currentGameState == STATE_PLAYING && mouseCaptured) {
	    if (key == 'v' || key == 'V') {
	        shadowsEnabled = !shadowsEnabled;
	        printf("Shadows %s\n", shadowsEnabled ? "ENABLED" : "DISABLED");
	    }
        if (key >= 'a' && key <= 'z') {
            keys[key] = true;
            keys[key - ('a' - 'A')] = true;
        }
        else if (key >= 'A' && key <= 'Z') {
            keys[key] = true;
            keys[key + ('a' - 'A')] = true;
        }
        else {
            keys[key] = true;
        }
    }
}

// Fungsi yang menangani input keyboard saat tombol dilepas.
void keyboardUp(unsigned char key, int x, int y) {
    if (key >= 'a' && key <= 'z') {
        keys[key] = false;
        keys[key - ('a' - 'A')] = false;
    }
    else if (key >= 'A' && key <= 'Z') {
        keys[key] = false;
        keys[key + ('a' - 'A')] = false;
    }
    else {
        keys[key] = false;
    }
}

// Fungsi yang menangani gerakan mouse.
void mouseMotion(int mx, int my) {
    if (!mouseCaptured) {
        if (currentGameState == STATE_HOME_SCREEN) {
            startGameButton.isHovered = startGameButton.isClicked(mx, my, windowHeight);
            quitButtonHomeScreen.isHovered = quitButtonHomeScreen.isClicked(mx, my, windowHeight);
        }
        else if (currentGameState == STATE_GAME_OVER) {
            retryButton.isHovered = retryButton.isClicked(mx, my, windowHeight);
            quitButtonGameOver.isHovered = quitButtonGameOver.isClicked(mx, my, windowHeight);
        }
    }

    if (!mouseCaptured || currentGameState != STATE_PLAYING) {
        lastMouseX = mx;
        lastMouseY = my;
        return;
    }

    if (firstMouse) {
        lastMouseX = mx;
        lastMouseY = my;
        firstMouse = false;
        glutWarpPointer(windowWidth / 2, windowHeight / 2);
        return;
    }

    int deltaX = mx - windowWidth / 2;
    int deltaY = my - windowHeight / 2;

    cameraYaw += deltaX * MOUSE_SENSITIVITY;
    cameraPitch -= deltaY * MOUSE_SENSITIVITY;

    if (cameraPitch > PITCH_LIMIT) cameraPitch = PITCH_LIMIT;
    if (cameraPitch < -PITCH_LIMIT) cameraPitch = -PITCH_LIMIT;

    if (cameraYaw >= 360.0f) cameraYaw -= 360.0f;
    if (cameraYaw < 0.0f) cameraYaw += 360.0f;

    if (mx != windowWidth / 2 || my != windowHeight / 2) {
        glutWarpPointer(windowWidth / 2, windowHeight / 2);
    }
}

// Fungsi yang menangani input tombol mouse.
void mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            leftMouseDown = true;

            if (currentGameState == STATE_PLAYING && mouseCaptured && !gameOver) {
                shootBullet();
                playShootSound(); 
                autoFireTimer = 0.0f;
            }

            if (currentGameState == STATE_HOME_SCREEN) {
                if (startGameButton.isClicked(x, y, windowHeight)) {
                    startGame();
                }
                else if (quitButtonHomeScreen.isClicked(x, y, windowHeight)) {
                    exit(0);
                }
            }
			else if (currentGameState == STATE_GAME_OVER) {
	            if (retryButton.isClicked(x, y, windowHeight)) {
	                startGame();
	            }
	            else if (quitButtonGameOver.isClicked(x, y, windowHeight)) {
	                exit(0);
	            }
	        }
            else if (currentGameState == STATE_PLAYING && !mouseCaptured) {
                mouseCaptured = true;
                glutSetCursor(GLUT_CURSOR_NONE);
                firstMouse = true;
                glutWarpPointer(windowWidth / 2, windowHeight / 2);
            }
        }
        else if (state == GLUT_UP) {
            leftMouseDown = false;
        }
    }
}

// Fungsi timer utama untuk loop permainan.
void gameLoopTimer(int value) {
    if (currentGameState == STATE_PLAYING && !gameOver) {
        robotSpawnTimer += 16;
        if (robotSpawnTimer >= ROBOT_SPAWN_INTERVAL) {
            spawnRobot();
            robotSpawnTimer = 0;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, gameLoopTimer, 0);
}

// Fungsi timer untuk proses pemuatan sumber daya.
void loadingTimerCallback(int value) {
    if (currentGameState == STATE_LOADING && !resourcesLoaded) {
        switch (currentLoadingStep) {
            case 0:
                initGL();
                break;
            case 1:
                initTextures();
                break;
            case 2:
                generateCityLayout();
                break;
            case 3:
                generateBuildings();
                break;
            case 4:
                generateCars();
                resetGame();
                resourcesLoaded = true;
                currentGameState = STATE_HOME_SCREEN;
                break;
        }

        currentLoadingStep++;
        loadingProgress = currentLoadingStep / LOADING_STEPS;

        if (!resourcesLoaded) {
            glutTimerFunc(100, loadingTimerCallback, 0);
        }

        glutPostRedisplay();
    }
}
