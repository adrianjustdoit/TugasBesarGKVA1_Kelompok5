#include "gldefs.h"
#include "mygenerate.h"
#include "myfuncs.h"
#include "gamestate.h"
#include "const.h"
#include <GL/glut.h>
#include <GL/gl.h>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <cstdio>
// Tambahkan di bagian atas file setelah includes
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

// Fungsi untuk menginisialisasi shadow map.
void initShadowMap() {
    if (shadowMapTexture != 0) {
        glDeleteTextures(1, &shadowMapTexture);
    }

    glGenTextures(1, &shadowMapTexture);
    glBindTexture(GL_TEXTURE_2D, shadowMapTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0,
                 GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

    glBindTexture(GL_TEXTURE_2D, 0);
}

// Fungsi untuk menyiapkan matriks yang digunakan dalam rendering bayangan.
void setupShadowMatrix() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    float orthoSize = CITY_SIZE * 0.8f;
    glOrtho(-orthoSize/2, orthoSize/2, -orthoSize/2, orthoSize/2, 1.0f, CITY_SIZE);
    glGetFloatv(GL_PROJECTION_MATRIX, lightProjectionMatrix);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    gluLookAt(lightPosition[0], lightPosition[1], lightPosition[2],
              CITY_SIZE/2.0f, 0.0f, CITY_SIZE/2.0f,
              0.0f, 0.0f, 1.0f);
    glGetFloatv(GL_MODELVIEW_MATRIX, lightViewMatrix);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// Fungsi untuk merender scene dari perspektif cahaya untuk membuat shadow map.
void renderShadowMap() {
    if (!shadowsEnabled) return;

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

    glClear(GL_DEPTH_BUFFER_BIT);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(lightProjectionMatrix);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(lightViewMatrix);

    glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glPolygonOffset(2.0f, 4.0f);
    glEnable(GL_POLYGON_OFFSET_FILL);

    for (const auto& building : buildings) {
        glPushMatrix();
        glTranslatef(building.x, 0.0f, building.z);

        float halfWidth = building.width / 2.0f;
        float halfDepth = building.depth / 2.0f;

        glBegin(GL_QUADS);
            glVertex3f(-halfWidth, 0.0f, halfDepth);
            glVertex3f(halfWidth, 0.0f, halfDepth);
            glVertex3f(halfWidth, building.height, halfDepth);
            glVertex3f(-halfWidth, building.height, halfDepth);

            glVertex3f(halfWidth, 0.0f, -halfDepth);
            glVertex3f(-halfWidth, 0.0f, -halfDepth);
            glVertex3f(-halfWidth, building.height, -halfDepth);
            glVertex3f(halfWidth, building.height, -halfDepth);

            glVertex3f(-halfWidth, 0.0f, -halfDepth);
            glVertex3f(-halfWidth, 0.0f, halfDepth);
            glVertex3f(-halfWidth, building.height, halfDepth);
            glVertex3f(-halfWidth, building.height, -halfDepth);

            glVertex3f(halfWidth, 0.0f, halfDepth);
            glVertex3f(halfWidth, 0.0f, -halfDepth);
            glVertex3f(halfWidth, building.height, -halfDepth);
            glVertex3f(halfWidth, building.height, halfDepth);

            glVertex3f(-halfWidth, building.height, -halfDepth);
            glVertex3f(-halfWidth, building.height, halfDepth);
            glVertex3f(halfWidth, building.height, halfDepth);
            glVertex3f(halfWidth, building.height, -halfDepth);
        glEnd();

        glPopMatrix();
    }

    for (const auto& car : cars) {
        glPushMatrix();
        glTranslatef(car.x, car.y, car.z);
        glRotatef(car.direction * 180.0f / MPI, 0.0f, 1.0f, 0.0f);
        glScalef(CAR_SCALE_FACTOR, CAR_SCALE_FACTOR, CAR_SCALE_FACTOR);

        glBegin(GL_QUADS);
            glVertex3f(-0.5f, 0.0f, -1.0f);
            glVertex3f(0.5f, 0.0f, -1.0f);
            glVertex3f(0.5f, 0.4f, 1.0f);
            glVertex3f(-0.5f, 0.4f, 1.0f);
        glEnd();

        glPopMatrix();
    }

    glPopAttrib();

    glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                     0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

// Fungsi untuk menggambar bayangan pada scene utama.
void drawShadowedScene() {
    if (!shadowsEnabled) {
        return;
    }

    static const float biasMatrix[16] = {
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f
    };

    glMatrixMode(GL_TEXTURE);
    glLoadMatrixf(biasMatrix);
    glMultMatrixf(lightProjectionMatrix);
    glMultMatrixf(lightViewMatrix);
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, shadowMapTexture);

    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);
    glEnable(GL_TEXTURE_GEN_Q);

    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
    glDepthMask(GL_FALSE);

    glColor3f(0.6f, 0.6f, 0.6f);
    glBegin(GL_QUADS);
        glVertex3f(0.0f, 0.02f, 0.0f);
        glVertex3f(CITY_SIZE, 0.02f, 0.0f);
        glVertex3f(CITY_SIZE, 0.02f, CITY_SIZE);
        glVertex3f(0.0f, 0.02f, CITY_SIZE);
    glEnd();

    glDepthMask(GL_TRUE);
    glPopAttrib();

    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);
    glDisable(GL_TEXTURE_GEN_Q);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
}

// Fungsi untuk membersihkan sumber daya shadow map.
void cleanupShadowMap() {
    if (shadowMapTexture != 0) {
        glDeleteTextures(1, &shadowMapTexture);
        shadowMapTexture = 0;
    }
}

// Fungsi untuk memunculkan robot baru di lokasi acak.
void spawnRobot() {
    if (gameOver || robots.size() >= 20) return;

    float spawnX, spawnZ;
    bool validPosition = false;
    int attempts = 0;

    while(!validPosition && attempts < 100) {
        float angle_ = (rand() % 360) * MPI / 180.0f;
        float dist = 40.0f + (rand() % 80);

        spawnX = cameraX + cos(angle_) * dist;
        spawnZ = cameraZ + sin(angle_) * dist;

        if (spawnX < 10) spawnX = 10;
        if (spawnX > CITY_SIZE - 10) spawnX = CITY_SIZE - 10;
        if (spawnZ < 10) spawnZ = 10;
        if (spawnZ > CITY_SIZE - 10) spawnZ = CITY_SIZE - 10;

        if (!checkBuildingCollision(spawnX, spawnZ, 6.0f)) {
            validPosition = true;
        }
        attempts++;
    }

    if (validPosition) {
        float spawnSize = 2.0f + (float)(rand() % 150) / 100.0f;
        int spawnHp = 4 + (rand() % 6);
        float spawnSpeed = 2.5f + (float)(rand() % 350) / 100.0f;

        robots.push_back({
            spawnX, 0.0f, spawnZ,
            spawnSize, spawnHp, spawnSpeed,
            true, (float)(rand() % 360), 0, 0.0f, spawnX, spawnZ
        });
    }
}

// Fungsi untuk menembakkan peluru dari posisi pemain.
void shootBullet() {
    if (gameOver) return;

    Bullet b;
    float bulletSpawnOffset = 0.5f;
    float rightX_gun = -lookZ;
    float rightZ_gun = lookX;
    float gunOffsetForward = 0.6f;
    float gunOffsetRight = 0.20f;
    float gunOffsetDown = -0.25f;
    float barrelTipRelativeForward = 0.3f + 0.35f;
    float barrelOffsetYRelative = 0.015f;

    b.bx = cameraX + lookX * (gunOffsetForward + barrelTipRelativeForward + bulletSpawnOffset);
    b.by = cameraY + lookY * (gunOffsetForward + barrelTipRelativeForward + bulletSpawnOffset) +
          gunOffsetDown + barrelOffsetYRelative;
    b.bz = cameraZ + lookZ * (gunOffsetForward + barrelTipRelativeForward + bulletSpawnOffset);

    b.speed = 100.0f;
    b.dx = lookX * b.speed;
    b.dy = lookY * b.speed;
    b.dz = lookZ * b.speed;
    b.lifetime = 2.5f;

    bullets.push_back(b);
    muzzleFlashEffect.lifetime = 0.07f;
}

// Fungsi untuk memperbarui posisi dan status peluru.
void updateBullets(float deltaTime) {
    for (size_t i = 0; i < bullets.size(); ) {
        bullets[i].bx += bullets[i].dx * deltaTime;
        bullets[i].by += bullets[i].dy * deltaTime;
        bullets[i].bz += bullets[i].dz * deltaTime;
        bullets[i].lifetime -= deltaTime;

        if (bullets[i].lifetime <= 0) {
            bullets.erase(bullets.begin() + i);
        } else {
            if (bullets[i].bx < -CITY_SIZE || bullets[i].bx > CITY_SIZE * 2 ||
                bullets[i].bz < -CITY_SIZE || bullets[i].bz > CITY_SIZE * 2 ||
                bullets[i].by < -10 || bullets[i].by > CITY_SIZE) {
                bullets.erase(bullets.begin() + i);
            } else {
                i++;
            }
        }
    }

    if (muzzleFlashEffect.lifetime > 0.0f) {
        muzzleFlashEffect.lifetime -= deltaTime;
    }
}

// Fungsi untuk memperbarui logika dan pergerakan robot.
void updateRobots(float deltaTime) {
    if (gameOver) return;

    for (size_t i = 0; i < robots.size(); ++i) {
        if (robots[i].alive) {
            robots[i].stateTimer -= deltaTime;

            float dx = cameraX - robots[i].rx;
            float dz = cameraZ - robots[i].rz;
            float distToPlayer = sqrt(dx*dx + dz*dz);

            if (distToPlayer < 120.0f) {
                robots[i].state = 1;
                robots[i].targetX = cameraX;
                robots[i].targetZ = cameraZ;
                if (robots[i].stateTimer <= 0) {
                    robots[i].stateTimer = 0.5f;
                }
            }
            else if (robots[i].stateTimer <= 0) {
                robots[i].state = 2;
                float patrolAngle = (rand() % 360) * MPI / 180.0f;
                float patrolDist = 15.0f + (rand() % 20);

                robots[i].targetX = robots[i].rx + cos(patrolAngle) * patrolDist;
                robots[i].targetZ = robots[i].rz + sin(patrolAngle) * patrolDist;

                if (robots[i].targetX < 10) robots[i].targetX = 10;
                if (robots[i].targetX > CITY_SIZE - 10) robots[i].targetX = CITY_SIZE - 10;
                if (robots[i].targetZ < 10) robots[i].targetZ = 10;
                if (robots[i].targetZ > CITY_SIZE - 10) robots[i].targetZ = CITY_SIZE - 10;

                robots[i].stateTimer = 3.0f + (rand() % 30) / 10.0f;
            }

            if (robots[i].state == 1) {
                robots[i].targetX = cameraX;
                robots[i].targetZ = cameraZ;
            }

            float dirX = robots[i].targetX - robots[i].rx;
            float dirZ = robots[i].targetZ - robots[i].rz;
            float distanceToTarget = sqrt(dirX * dirX + dirZ * dirZ);

            if (distanceToTarget > 0.5f) {
                float normDirX = dirX / distanceToTarget;
                float normDirZ = dirZ / distanceToTarget;
                robots[i].rotationY = atan2(normDirX, normDirZ) * 180.0f / MPI;

                float moveThreshold = (robots[i].state == 1) ? robots[i].size * 0.8f : 2.0f;

                if (distanceToTarget > moveThreshold) {
                    float speedMultiplier = (robots[i].state == 1) ? 1.2f : 1.0f;
                    float moveX = normDirX * robots[i].moveSpeed * speedMultiplier * deltaTime;
                    float moveZ = normDirZ * robots[i].moveSpeed * speedMultiplier * deltaTime;

                    float nextRx = robots[i].rx + moveX;
                    float nextRz = robots[i].rz + moveZ;

                    if (!checkBuildingCollision(nextRx, nextRz, robots[i].size / 2.0f)) {
                        robots[i].rx = nextRx;
                        robots[i].rz = nextRz;
                    } else {
                        if (robots[i].state == 1) {
                            float avoidAngle = (rand() % 360) * MPI / 180.0f;
                            float tempTargetDist = 10.0f + (rand() % 5);
                            robots[i].targetX = robots[i].rx + cos(avoidAngle) * tempTargetDist;
                            robots[i].targetZ = robots[i].rz + sin(avoidAngle) * tempTargetDist;
                            robots[i].stateTimer = 0.5f;
                        } else {
                            robots[i].stateTimer = 0.1f;
                        }
                    }
                } else if (robots[i].state == 2) {
                    robots[i].stateTimer = 0.1f;
                }
            }

            float attackRange = robots[i].size * 0.8f + 1.5f;
            if (distToPlayer < attackRange && playerHealth > 0) {
                playerHealth -= 1;
                if (playerHealth <= 0) {
                    playerHealth = 0;
                    gameOver = true;
                }
            }
        }
    }
}

// Fungsi untuk memperbarui logika dan pergerakan mobil.
void updateCars(float deltaTime) {
    for(size_t i = 0; i < cars.size(); ++i) {
        if (streets.empty() || cars[i].currentStreetIndex < 0 ||
            (size_t)cars[i].currentStreetIndex >= streets.size()) continue;

        float prevX = cars[i].x;
        float prevZ = cars[i].z;

        const Street& currentStreet = streets[cars[i].currentStreetIndex];
        float streetDx = currentStreet.x2 - currentStreet.x1;
        float streetDz = currentStreet.z2 - currentStreet.z1;
        float streetLength = sqrt(streetDx * streetDx + streetDz * streetDz);

        if (streetLength < 0.1f) continue;

        float moveDist = cars[i].speed * deltaTime;
        cars[i].progressOnStreet += (cars[i].movingForward ? 1 : -1) * (moveDist / streetLength);

        if (cars[i].progressOnStreet > 1.0f || cars[i].progressOnStreet < 0.0f) {
            std::pair<int,int> endNodeGridCoords = cars[i].movingForward ?
                getGridCoordinates(currentStreet.x2, currentStreet.z2) :
                getGridCoordinates(currentStreet.x1, currentStreet.z1);

            if (intersections.count(endNodeGridCoords)) {
                const Intersection& nextIntersection = intersections.at(endNodeGridCoords);
                if (!nextIntersection.connectedStreets.empty()) {
                    int newStreetIdx = nextIntersection.connectedStreets[rand() % nextIntersection.connectedStreets.size()];

                    if ((size_t)newStreetIdx != (size_t)cars[i].currentStreetIndex) {
                        cars[i].currentStreetIndex = newStreetIdx;

                        float endNodeX = intersections.at(endNodeGridCoords).x;
                        float endNodeZ = intersections.at(endNodeGridCoords).z;

                        float distToStart = sqrt(pow(endNodeX - streets[newStreetIdx].x1, 2) +
                                               pow(endNodeZ - streets[newStreetIdx].z1, 2));
                        float distToEnd = sqrt(pow(endNodeX - streets[newStreetIdx].x2, 2) +
                                             pow(endNodeZ - streets[newStreetIdx].z2, 2));

                        if (distToStart < distToEnd) {
                            cars[i].progressOnStreet = 0.01f;
                            cars[i].movingForward = true;
                        } else {
                            cars[i].progressOnStreet = 0.99f;
                            cars[i].movingForward = false;
                        }
                    } else {
                        cars[i].movingForward = !cars[i].movingForward;
                        cars[i].progressOnStreet = std::max(0.01f, std::min(0.99f, cars[i].progressOnStreet));
                    }
                } else {
                    cars[i].movingForward = !cars[i].movingForward;
                    cars[i].progressOnStreet = std::max(0.01f, std::min(0.99f, cars[i].progressOnStreet));
                }
            } else {
                cars[i].movingForward = !cars[i].movingForward;
                cars[i].progressOnStreet = std::max(0.01f, std::min(0.99f, cars[i].progressOnStreet));
            }

            cars[i].progressOnStreet = std::max(0.0f, std::min(1.0f, cars[i].progressOnStreet));
        }

        const Street& finalStreet = streets[cars[i].currentStreetIndex];
        float finalStreetDx = finalStreet.x2 - finalStreet.x1;
        float finalStreetDz = finalStreet.z2 - finalStreet.z1;

        cars[i].x = finalStreet.x1 + cars[i].progressOnStreet * finalStreetDx;
        cars[i].z = finalStreet.z1 + cars[i].progressOnStreet * finalStreetDz;

        float finalStreetLength = sqrt(finalStreetDx * finalStreetDx + finalStreetDz * finalStreetDz);
        if(finalStreetLength > 0.1f) {
            float laneOffset = finalStreet.width * 0.28f;
            float perpX = -(finalStreetDz / finalStreetLength);
            float perpZ = (finalStreetDx / finalStreetLength);

            if (cars[i].movingForward) {
                cars[i].x += perpX * laneOffset;
                cars[i].z += perpZ * laneOffset;
                cars[i].direction = atan2(finalStreetDz, finalStreetDx);
            } else {
                cars[i].x -= perpX * laneOffset;
                cars[i].z -= perpZ * laneOffset;
                cars[i].direction = atan2(-finalStreetDz, -finalStreetDx);
            }
        }

        float dx_car_player = cars[i].x - cameraX;
        float dz_car_player = cars[i].z - cameraZ;
        float carLength = 3.8f * CAR_SCALE_FACTOR;
        float carWidth = 1.9f * CAR_SCALE_FACTOR;
        float collisionDistance = std::max(carLength, carWidth) / 2.0f + COLLISION_BUFFER * 0.5f;

        if (dx_car_player * dx_car_player + dz_car_player * dz_car_player < collisionDistance * collisionDistance) {
            playerHealth = 0;
            gameOver = true;
            std::cout << "Player hit by car! Game over." << std::endl;
            break;
        }

        if (prevX != cars[i].x || prevZ != cars[i].z) {
            float line_len_sq = (cars[i].x - prevX) * (cars[i].x - prevX) +
                              (cars[i].z - prevZ) * (cars[i].z - prevZ);

            if (line_len_sq > 0.001f) {
                float t = ((cameraX - prevX) * (cars[i].x - prevX) +
                         (cameraZ - prevZ) * (cars[i].z - prevZ)) / line_len_sq;

                if (t >= 0.0f && t <= 1.0f) {
                    float closest_x = prevX + t * (cars[i].x - prevX);
                    float closest_z = prevZ + t * (cars[i].z - prevZ);
                    float dist_sq = (cameraX - closest_x) * (cameraX - closest_x) +
                                  (cameraZ - closest_z) * (cameraZ - closest_z);

                    if (dist_sq < collisionDistance * collisionDistance) {
                        playerHealth = 0;
                        gameOver = true;
                        std::cout << "Player hit by fast-moving car! Game over." << std::endl;
                        break;
                    }
                }
            }
        }
    }
}

// Fungsi untuk memeriksa tabrakan mobil secara detail, mengembalikan status dan indeks mobil.
std::pair<bool, int> checkCarCollisionDetailed(float nextX, float nextZ, float buffer) {
    for (size_t i = 0; i < cars.size(); ++i) {
        float carLength = 3.8f;
        float carWidth = 1.9f;
        float dx_car = nextX - cars[i].x;
        float dz_car = nextZ - cars[i].z;
        float max_car_dim_check = std::max(carLength, carWidth) / 2.0f + buffer + 6.0f;

        if (dx_car * dx_car + dz_car * dz_car > max_car_dim_check * max_car_dim_check) continue;

        float maxDim = std::max(carLength, carWidth) / 2.0f + buffer;

        if (nextX >= cars[i].x - maxDim && nextX <= cars[i].x + maxDim &&
            nextZ >= cars[i].z - maxDim && nextZ <= cars[i].z + maxDim) {
            return std::make_pair(true, i);
        }
    }

    return std::make_pair(false, -1);
}

// Fungsi untuk memeriksa tabrakan mobil (versi sederhana).
bool checkCarCollision(float nextX, float nextZ, float buffer) {
    return checkCarCollisionDetailed(nextX, nextZ, buffer).first;
}

// Fungsi untuk memeriksa tabrakan antara pemain dan mobil.
void checkPlayerCarCollision() {
    std::pair<bool, int> result = checkCarCollisionDetailed(cameraX, cameraZ, COLLISION_BUFFER * 0.5f);
    bool collision = result.first;
    int carIndex = result.second;

    if (collision && carIndex >= 0) {
        playerHealth = 0;
        gameOver = true;
        std::cout << "Player hit by car! Game over." << std::endl;
    }
}

// Fungsi untuk memperbarui posisi dan orientasi pemain/kamera.
void updatePlayerAndCamera(float deltaTime) {
    if (gameOver) return;

    checkPlayerCarCollision();
    if (gameOver) return;

    lookX = sin(cameraYaw * MPI / 180.0f) * cos(cameraPitch * MPI / 180.0f);
    lookY = sin(cameraPitch * MPI / 180.0f);
    lookZ = -cos(cameraYaw * MPI / 180.0f) * cos(cameraPitch * MPI / 180.0f);

    float mag = sqrt(lookX * lookX + lookY * lookY + lookZ * lookZ);
    if (mag > 0.001f) {
        lookX /= mag;
        lookY /= mag;
        lookZ /= mag;
    }

    float currentDeltaMoveForward = 0, currentDeltaMoveSide = 0, currentDeltaMoveVertical = 0;

    if (keys['w'] || keys['W']) {
        currentDeltaMoveForward = 1.0f;
    }
    else if (keys['s'] || keys['S']) {
        currentDeltaMoveForward = -1.0f;
    }

    if (keys['a'] || keys['A']) {
        currentDeltaMoveSide = -1.0f;
    }
    else if (keys['d'] || keys['D']) {
        currentDeltaMoveSide = 1.0f;
    }

    float nextCamX = cameraX;
    float nextCamZ = cameraZ;
    float nextCamY = cameraY;

    if (std::abs(currentDeltaMoveForward) > 0.01f) {
        float forwardX_xz = sin(cameraYaw * MPI / 180.0f);
        float forwardZ_xz = -cos(cameraYaw * MPI / 180.0f);
        nextCamX += currentDeltaMoveForward * forwardX_xz * MOVE_SPEED * deltaTime;
        nextCamZ += currentDeltaMoveForward * forwardZ_xz * MOVE_SPEED * deltaTime;
    }

    if (std::abs(currentDeltaMoveSide) > 0.01f) {
        float rightX_xz = -lookZ;
        float rightZ_xz = lookX;
        nextCamX += currentDeltaMoveSide * rightX_xz * MOVE_SPEED * deltaTime;
        nextCamZ += currentDeltaMoveSide * rightZ_xz * MOVE_SPEED * deltaTime;
    }

    if (std::abs(currentDeltaMoveVertical) > 0.01f) {
        nextCamY += currentDeltaMoveVertical * MOVE_SPEED * deltaTime;
    }

    if (!checkBuildingCollision(nextCamX, nextCamZ, COLLISION_BUFFER) &&
        !checkCarCollision(nextCamX, nextCamZ, COLLISION_BUFFER) &&
        nextCamX >= COLLISION_BUFFER && nextCamX <= CITY_SIZE - COLLISION_BUFFER &&
        nextCamZ >= COLLISION_BUFFER && nextCamZ <= CITY_SIZE - COLLISION_BUFFER) {
        cameraX = nextCamX;
        cameraZ = nextCamZ;
    }

    cameraY = nextCamY;
    if (cameraY < 1.75f) cameraY = 1.75f;
    if (cameraY > CITY_SIZE / 3.0f) cameraY = CITY_SIZE / 3.0f;
}

// Fungsi untuk memeriksa berbagai jenis tabrakan dalam game.
void checkCollisions(float deltaTime) {
    if (gameOver) return;

    for (size_t i = 0; i < bullets.size(); ) {
        bool bulletRemovedThisIteration = false;
        Bullet& currentBullet = bullets[i];

        for (size_t j = 0; j < robots.size(); ++j) {
            if (robots[j].alive) {
                float halfSize = robots[j].size / 2.0f;
                float robotBodyBottomY = robots[j].ry;
                float robotBodyTopY = robots[j].ry + robots[j].size * 1.3f;

                if (currentBullet.bx >= robots[j].rx - halfSize &&
                    currentBullet.bx <= robots[j].rx + halfSize &&
                    currentBullet.by >= robotBodyBottomY &&
                    currentBullet.by <= robotBodyTopY &&
                    currentBullet.bz >= robots[j].rz - halfSize &&
                    currentBullet.bz <= robots[j].rz + halfSize) {

                    robots[j].hp--;
                    if (robots[j].hp <= 0) {
                        robots[j].alive = false;
                        killStreak++;
                        playerScore += 100 * killStreak;
                    }

                    bullets.erase(bullets.begin() + i);
                    bulletRemovedThisIteration = true;
                    break;
                }
            }
        }

        if (bulletRemovedThisIteration) continue;

        for (const auto& building : buildings) {
            float halfWidth = building.width / 2.0f;
            float halfDepth = building.depth / 2.0f;

            if (currentBullet.bx >= building.x - halfWidth &&
                currentBullet.bx <= building.x + halfWidth &&
                currentBullet.bz >= building.z - halfDepth &&
                currentBullet.bz <= building.z + halfDepth &&
                currentBullet.by >= 0.0f &&
                currentBullet.by <= building.height) {

                bullets.erase(bullets.begin() + i);
                bulletRemovedThisIteration = true;
                break;
            }
        }

        if (bulletRemovedThisIteration) continue;

        for (const auto& car : cars) {
            float car_center_y = car.y + (CAR_SCALE_FACTOR * 0.9f / 2.0f);
            float dx_bullet_car = currentBullet.bx - car.x;
            float dy_bullet_car = currentBullet.by - car_center_y;
            float dz_bullet_car = currentBullet.bz - car.z;
            float distSq_bullet_car = dx_bullet_car * dx_bullet_car +
                                    dy_bullet_car * dy_bullet_car +
                                    dz_bullet_car * dz_bullet_car;

            float car_approx_radius = CAR_SCALE_FACTOR * 1.5f;

            if (distSq_bullet_car < car_approx_radius * car_approx_radius) {
                bullets.erase(bullets.begin() + i);
                bulletRemovedThisIteration = true;
                break;
            }
        }

        if (bulletRemovedThisIteration) continue;
        i++;
    }

    robots.erase(
        std::remove_if(robots.begin(), robots.end(), [](const Robot& r) { return !r.alive; }),
        robots.end()
    );
}

// Fungsi untuk memeriksa apakah posisi bertabrakan dengan bangunan.
bool checkBuildingCollision(float nextX, float nextZ, float buffer) {
    for (const auto& building : buildings) {
        float dx_bld_col_check = nextX - building.x;
        float dz_bld_col_check = nextZ - building.z;
        float approx_dist_sq = dx_bld_col_check * dx_bld_col_check + dz_bld_col_check * dz_bld_col_check;
        float max_bld_dim_check = std::max(building.width, building.depth) / 2.0f + buffer + 25.0f;

        if (approx_dist_sq > max_bld_dim_check * max_bld_dim_check) continue;

        float halfWidth = building.width / 2.0f + buffer;
        float halfDepth = building.depth / 2.0f + buffer;

        if (nextX >= building.x - halfWidth && nextX <= building.x + halfWidth &&
            nextZ >= building.z - halfDepth && nextZ <= building.z + halfDepth) {
            return true;
        }
    }

    return false;
}

// Fungsi untuk menyiapkan pencahayaan dari senter pemain.
void setupPlayerLight() {
    if (gameOver) {
        glDisable(GL_LIGHT0);
        return;
    }

    glEnable(GL_LIGHT0);
    GLfloat light_ambient[] = {0.2f, 0.18f, 0.15f, 1.0f};
    GLfloat light_diffuse[] = {0.8f, 0.75f, 0.6f, 1.0f};
    GLfloat light_specular[] = {0.7f, 0.65f, 0.5f, 1.0f};
    GLfloat light_position[] = {cameraX + lookX * 0.2f, cameraY + lookY * 0.2f, cameraZ + lookZ * 0.2f, 1.0f};
    GLfloat spot_direction[] = {lookX, lookY, lookZ};

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 45.0f);
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spot_direction);
    glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 8.0f);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.4f);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.03f);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.007f);
}


// Fungsi untuk menyiapkan pencahayaan matahari sore (golden hour).
void setupGoldenHourSunLight() {
    GLfloat sunLight_pos[] = {0.8f, 0.35f, 0.5f, 0.0f};
    GLfloat sunLight_amb[] = {0.35f, 0.25f, 0.15f, 1.0f};
    GLfloat sunLight_dif[] = {0.9f, 0.65f, 0.3f, 1.0f};
    GLfloat sunLight_spec[] = {0.8f, 0.7f, 0.5f, 1.0f};

    glEnable(GL_LIGHT6);
    glLightfv(GL_LIGHT6, GL_POSITION, sunLight_pos);
    glLightfv(GL_LIGHT6, GL_AMBIENT, sunLight_amb);
    glLightfv(GL_LIGHT6, GL_DIFFUSE, sunLight_dif);
    glLightfv(GL_LIGHT6, GL_SPECULAR, sunLight_spec);
}
