#include "gldefs.h"
#include "mygenerate.h"
#include "mydraw.h"
#include "gamestate.h"
#include "const.h"
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cstring>
#include <cmath>
#include <iostream>
#include <cstdio>

// Fungsi untuk menggambar loading screen
void drawLoadingScreen() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);

    glColor3f(0.1f, 0.1f, 0.2f);
    glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(windowWidth, 0);
        glVertex2f(windowWidth, windowHeight);
        glVertex2f(0, windowHeight);
    glEnd();

    const char* titleText = "ROBOT INVASION";
    float titleWidth = 0;
    for(size_t i = 0; i < strlen(titleText); ++i)
        titleWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, titleText[i]);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(windowWidth / 2.0f - titleWidth / 2.0f, windowHeight * 0.6f, titleText);

    const char* loadingText = "Loading...";
    float loadingTextWidth = 0;
    for(size_t i = 0; i < strlen(loadingText); ++i)
        loadingTextWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, loadingText[i]);
    drawText(windowWidth / 2.0f - loadingTextWidth / 2.0f, windowHeight * 0.45f, loadingText);

    float barWidth = windowWidth * 0.6f;
    float barHeight = 30.0f;
    float barX = (windowWidth - barWidth) / 2.0f;
    float barY = windowHeight * 0.4f - barHeight / 2.0f;

    glColor3f(0.7f, 0.7f, 0.7f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(barX, barY);
        glVertex2f(barX + barWidth, barY);
        glVertex2f(barX + barWidth, barY + barHeight);
        glVertex2f(barX, barY + barHeight);
    glEnd();

    glColor3f(0.3f, 0.6f, 1.0f);
    glBegin(GL_QUADS);
        glVertex2f(barX + 2, barY + 2);
        glVertex2f(barX + 2 + (barWidth - 4) * loadingProgress, barY + 2);
        glVertex2f(barX + 2 + (barWidth - 4) * loadingProgress, barY + barHeight - 2);
        glVertex2f(barX + 2, barY + barHeight - 2);
    glEnd();

    char stepInfo[100];
    const char* stepNames[] = {"Initializing", "Loading textures", "Generating city", "Creating buildings", "Finishing up"};
    sprintf(stepInfo, "%s (%d/%d)",
            stepNames[std::min(currentLoadingStep, 4)],
            currentLoadingStep + 1,
            (int)LOADING_STEPS);

    float stepInfoWidth = 0;
    for(size_t i = 0; i < strlen(stepInfo); ++i)
        stepInfoWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, stepInfo[i]);

    glColor3f(0.8f, 0.8f, 0.8f);
    drawText(windowWidth / 2.0f - stepInfoWidth / 2.0f, barY - 25.0f, stepInfo);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// Fungsi untuk menggambar kubus dengan tekstur.
void drawTexturedCube(float width, float height, float depth, GLuint textureID, float texScaleS, float texScaleT) {
    float w = width / 2.0f;
    float h = height / 2.0f;
    float d = depth / 2.0f;

    if (textureID != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glColor3f(1.0f, 1.0f, 1.0f);
    } else {
        glDisable(GL_TEXTURE_2D);
    }

    glBegin(GL_QUADS);
    glNormal3f(0.0, 0.0, 1.0);
    if(textureID != 0) glTexCoord2f(0.0, 0.0);
    glVertex3f(-w, -h, d);
    if(textureID != 0) glTexCoord2f(texScaleS, 0.0);
    glVertex3f(w, -h, d);
    if(textureID != 0) glTexCoord2f(texScaleS, texScaleT);
    glVertex3f(w, h, d);
    if(textureID != 0) glTexCoord2f(0.0, texScaleT);
    glVertex3f(-w, h, d);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(0.0, 0.0, -1.0);
    if(textureID != 0) glTexCoord2f(texScaleS, 0.0);
    glVertex3f(-w, -h, -d);
    if(textureID != 0) glTexCoord2f(texScaleS, texScaleT);
    glVertex3f(-w, h, -d);
    if(textureID != 0) glTexCoord2f(0.0, texScaleT);
    glVertex3f(w, h, -d);
    if(textureID != 0) glTexCoord2f(0.0, 0.0);
    glVertex3f(w, -h, -d);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(0.0, 1.0, 0.0);
    if(textureID != 0) glTexCoord2f(0.0, texScaleT);
    glVertex3f(-w, h, -d);
    if(textureID != 0) glTexCoord2f(0.0, 0.0);
    glVertex3f(-w, h, d);
    if(textureID != 0) glTexCoord2f(texScaleS, 0.0);
    glVertex3f(w, h, d);
    if(textureID != 0) glTexCoord2f(texScaleS, texScaleT);
    glVertex3f(w, h, -d);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(0.0, -1.0, 0.0);
    if(textureID != 0) glTexCoord2f(texScaleS, texScaleT);
    glVertex3f(-w, -h, -d);
    if(textureID != 0) glTexCoord2f(0.0, texScaleT);
    glVertex3f(w, -h, -d);
    if(textureID != 0) glTexCoord2f(0.0, 0.0);
    glVertex3f(w, -h, d);
    if(textureID != 0) glTexCoord2f(texScaleS, 0.0);
    glVertex3f(-w, -h, d);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(1.0, 0.0, 0.0);
    if(textureID != 0) glTexCoord2f(texScaleS, 0.0);
    glVertex3f(w, -h, -d);
    if(textureID != 0) glTexCoord2f(texScaleS, texScaleT);
    glVertex3f(w, h, -d);
    if(textureID != 0) glTexCoord2f(0.0, texScaleT);
    glVertex3f(w, h, d);
    if(textureID != 0) glTexCoord2f(0.0, 0.0);
    glVertex3f(w, -h, d);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(-1.0, 0.0, 0.0);
    if(textureID != 0) glTexCoord2f(0.0, 0.0);
    glVertex3f(-w, -h, -d);
    if(textureID != 0) glTexCoord2f(texScaleS, 0.0);
    glVertex3f(-w, -h, d);
    if(textureID != 0) glTexCoord2f(texScaleS, texScaleT);
    glVertex3f(-w, h, d);
    if(textureID != 0) glTexCoord2f(0.0, texScaleT);
    glVertex3f(-w, h, -d);
    glEnd();

    if (textureID != 0) {
        glDisable(GL_TEXTURE_2D);
    }
}

// Fungsi untuk menggambar lengan dan senjata pemain.
void drawPlayerArmAndGun() {
    if (gameOver) return;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glLoadIdentity();

    float offsetX = 0.18f;
    float offsetY = -0.25f;
    float offsetZ = -0.7f;
    glTranslatef(offsetX, offsetY, offsetZ);

    glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(-cameraPitch, 1.0f, 0.0f, 0.0f);

    glColor3f(0.12f, 0.12f, 0.12f);
    glPushMatrix();
    glScalef(0.07f, 0.09f, 0.40f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glColor3f(0.08f, 0.08f, 0.08f);
    glPushMatrix();
    glTranslatef(0.0f, 0.01f, 0.32f);
    GLUquadric* quad = gluNewQuadric();
    gluCylinder(quad, 0.018, 0.015, 0.25, 8, 3);
    gluDeleteQuadric(quad);
    glPopMatrix();

    glColor3f(0.1f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, -0.08f, -0.05f);
    glScalef(0.05f, 0.15f, 0.08f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glColor3f(0.05f, 0.05f, 0.05f);
    glPushMatrix();
    glTranslatef(0.0f, 0.055f, 0.05f);
    glScalef(0.012f, 0.025f, 0.08f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPopMatrix();
}

// Fungsi untuk menggambar efek kilatan 
void drawMuzzleFlash() {
    if (muzzleFlashEffect.lifetime <= 0.0f || muzzleFlashTexture == 0) return;

    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_POLYGON_BIT | GL_CURRENT_BIT);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, muzzleFlashTexture);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    float alpha = muzzleFlashEffect.lifetime / 0.07f;
    glColor4f(1.0f, 1.0f, 1.0f, alpha);

    float muzzleSize = 0.4f + (1.0f - alpha) * 0.6f;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glLoadIdentity();

    float gunViewOffsetX = 0.18f;
    float gunViewOffsetY = -0.28f;
    float gunViewOffsetZ = -0.55f;
    glTranslatef(gunViewOffsetX, gunViewOffsetY, gunViewOffsetZ);

    glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(-cameraPitch, 1.0f, 0.0f, 0.0f);

    float barrelTipLocalX = 0.0f;
    float barrelTipLocalY = 0.01f;
    float barrelTipLocalZ = 0.57f;

    float flashClearance = 0.05f;
    glTranslatef(barrelTipLocalX, barrelTipLocalY, barrelTipLocalZ + flashClearance);

    float modelviewMatrix[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelviewMatrix);
    for(int i=0; i<3; i++) {
        for(int j=0; j<3; j++) {
            if(i==j) modelviewMatrix[i*4+j] = 1.0f;
            else modelviewMatrix[i*4+j] = 0.0f;
        }
    }
    glLoadMatrixf(modelviewMatrix);

    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex3f(-muzzleSize/2.0f, -muzzleSize/2.0f, 0.0f);
    glTexCoord2f(1,0); glVertex3f( muzzleSize/2.0f, -muzzleSize/2.0f, 0.0f);
    glTexCoord2f(1,1); glVertex3f( muzzleSize/2.0f,  muzzleSize/2.0f, 0.0f);
    glTexCoord2f(0,1); glVertex3f(-muzzleSize/2.0f,  muzzleSize/2.0f, 0.0f);
    glEnd();

    glPopMatrix();
    glPopAttrib();
}

// Fungsi untuk menggambar satu robot.
void drawRobot(const Robot& robot) {
    glPushMatrix();
    glTranslatef(robot.rx, robot.ry, robot.rz);
    glRotatef(robot.rotationY, 0.0f, 1.0f, 0.0f);
    float s = robot.size;

    if (robotTexture) glColor3f(1.0f,1.0f,1.0f); else glColor3f(0.35f, 0.35f, 0.4f);
    glPushMatrix();
    glTranslatef(-s*0.18f, s*0.35f, 0.0f);
    drawTexturedCube(s*0.18f, s*0.7f, s*0.18f, robotTexture);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(s*0.18f, s*0.35f, 0.0f);
    drawTexturedCube(s*0.18f, s*0.7f, s*0.18f, robotTexture);
    glPopMatrix();

    if (robotTexture) glColor3f(1.0f,1.0f,1.0f); else glColor3f(0.45f, 0.45f, 0.55f);
    glPushMatrix();
    glTranslatef(0.0f, s*0.7f + s*0.3f, 0.0f);
    drawTexturedCube(s*0.45f, s*0.6f, s*0.35f, robotTexture);
    glPopMatrix();

    if (robotTexture) glColor3f(1.0f,1.0f,1.0f); else glColor3f(0.55f, 0.15f, 0.15f);
    glPushMatrix();
    glTranslatef(0.0f, s*0.7f + s*0.6f + s*0.18f, 0.0f);
    drawTexturedCube(s*0.3f, s*0.3f, s*0.3f, robotTexture);
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.05f, 0.7f, 0.7f);
    glPushMatrix();
    glTranslatef(-s*0.06f, s*0.06f, s*0.151f);
    glutSolidSphere(s*0.05f, 8, 8);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(s*0.06f, s*0.06f, s*0.151f);
    glutSolidSphere(s*0.05f, 8, 8);
    glPopMatrix();
    glPopMatrix();

    if (robotTexture) glColor3f(1.0f,1.0f,1.0f); else glColor3f(0.30f, 0.30f, 0.40f);
    glPushMatrix();
    glTranslatef(-s*0.28f, s*0.7f + s*0.3f, 0.0f);
    glRotatef(sin(glutGet(GLUT_ELAPSED_TIME)*0.0025f + robot.rx)*25.f + 10.f, 1,0,0);
    drawTexturedCube(s*0.12f, s*0.55f, s*0.12f, robotTexture);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(s*0.28f, s*0.7f + s*0.3f, 0.0f);
    glRotatef(sin(glutGet(GLUT_ELAPSED_TIME)*0.0025f + robot.rz + 0.5f)*25.f - 10.f, 1,0,0);
    drawTexturedCube(s*0.12f, s*0.55f, s*0.12f, robotTexture);
    glPopMatrix();

    glPopMatrix();
}

// Fungsi untuk menggambar semua robot yang ada.
void drawRobots() {
    for (size_t i = 0; i < robots.size(); ++i) {
        if (robots[i].alive) {
            float dx = robots[i].rx - cameraX;
            float dz = robots[i].rz - cameraZ;
            if (dx*dx + dz*dz < 300.0f * 300.0f) {
                drawRobot(robots[i]);
            }
        }
    }
}

// Fungsi untuk menggambar semua peluru yang aktif.
void drawBullets() {
    if (bullets.empty()) return;

    glDisable(GL_TEXTURE_2D);

    for (size_t i = 0; i < bullets.size(); ++i) {
        glPushMatrix();
        glTranslatef(bullets[i].bx, bullets[i].by, bullets[i].bz);

        float vx = bullets[i].dx;
        float vy = bullets[i].dy;
        float vz = bullets[i].dz;
        float speed = bullets[i].speed;

        if (speed > 0.001f) {
            float angleY = atan2(vx, vz) * 180.0f / MPI;
            float horizontalDist = sqrt(vx*vx + vz*vz);
            float angleX = atan2(-vy, horizontalDist) * 180.0f / MPI;

            glRotatef(angleY, 0.0f, 1.0f, 0.0f);
            glRotatef(angleX, 1.0f, 0.0f, 0.0f);
        }

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, bulletCasingColor);
        GLUquadric* quadric = gluNewQuadric();
        gluCylinder(quadric, BULLET_RADIUS, BULLET_RADIUS, BULLET_CASING_LENGTH, 8, 1);

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, bulletTipColor);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, BULLET_CASING_LENGTH);
        glutSolidCone(BULLET_RADIUS, BULLET_TIP_LENGTH, 8, 1);
        glPopMatrix();

        gluDeleteQuadric(quadric);
        glPopMatrix();
    }
}

// Fungsi untuk menggambar tanah dengan tekstur.
void drawTexturedGround() {
    const float GROUND_PLANE_SIZE = CITY_SIZE * 1.5f;

    const float texCoordMax = (2.0f * GROUND_PLANE_SIZE) / DESIRED_GROUND_TILE_WORLD_SIZE;

    if (groundTexture != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, groundTexture);
        glColor3f(1.0f, 1.0f, 1.0f);
    } else {
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.2f, 0.4f, 0.15f);
    }

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);

    if (groundTexture != 0) glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-GROUND_PLANE_SIZE, GROUND_Y_LEVEL, -GROUND_PLANE_SIZE);

    if (groundTexture != 0) glTexCoord2f(texCoordMax, 0.0f);
    glVertex3f( GROUND_PLANE_SIZE, GROUND_Y_LEVEL, -GROUND_PLANE_SIZE);

    if (groundTexture != 0) glTexCoord2f(texCoordMax, texCoordMax);
    glVertex3f( GROUND_PLANE_SIZE, GROUND_Y_LEVEL,  GROUND_PLANE_SIZE);

    if (groundTexture != 0) glTexCoord2f(0.0f, texCoordMax);
    glVertex3f(-GROUND_PLANE_SIZE, GROUND_Y_LEVEL,  GROUND_PLANE_SIZE);
    glEnd();

    if (groundTexture != 0) {
        glDisable(GL_TEXTURE_2D);
    }
}

// Fungsi untuk menggambar teks pada layar.
void drawText(float x, float y, const char *text) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);

    glRasterPos2f(x, y);
    for (size_t i = 0; i < strlen(text); ++i)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// Fungsi untuk menggambar crosshair di tengah layar.
void drawCrosshair() {
    if (gameOver) return;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);

    float centerX = windowWidth / 2.0f;
    float centerY = windowHeight / 2.0f;
    float crosshairSize = 8.0f;
    float gap = 3.0f;

    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(centerX - crosshairSize - gap, centerY);
    glVertex2f(centerX - gap, centerY);
    glVertex2f(centerX + gap, centerY);
    glVertex2f(centerX + crosshairSize + gap, centerY);
    glVertex2f(centerX, centerY + gap);
    glVertex2f(centerX, centerY + crosshairSize + gap);
    glVertex2f(centerX, centerY - gap);
    glVertex2f(centerX, centerY - crosshairSize - gap);
    glEnd();
    glLineWidth(1.0f);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// Fungsi untuk menggambar satu bangunan.
void drawBuilding(const Building& building) {
    float dx_bld_draw = building.x - cameraX;
    float dz_bld_draw = building.z - cameraZ;
    float distSquared = dx_bld_draw * dx_bld_draw + dz_bld_draw * dz_bld_draw;
    const float BUILDING_MAIN_CULL_DIST_SQ = 1800.0f * 1800.0f;

    if (distSquared > BUILDING_MAIN_CULL_DIST_SQ) return;

    glDisable(GL_TEXTURE_2D);
    glColor3f(building.r, building.g, building.b);
    glPushMatrix();
    glTranslatef(building.x, building.height / 2.0f, building.z);
    glScalef(building.width, building.height, building.depth);
    glutSolidCube(1.0f);
    glPopMatrix();

    const float WINDOW_CULL_DIST_SQ = 900.0f * 900.0f;
    if (distSquared > WINDOW_CULL_DIST_SQ) return;

    glDisable(GL_LIGHTING);
    float windowR = std::min(1.0f, 0.85f + (building.r * 0.1f));
    float windowG = std::min(1.0f, 0.75f + (building.g * 0.1f));
    float windowB = std::min(1.0f, 0.6f + (building.b * 0.05f));
    glColor3f(windowR, windowG, windowB);

    int floorStep = (distSquared > 500.0f * 500.0f) ? 3 : 1;
    int windowStep = (distSquared > 500.0f * 500.0f) ? 2 : 1;
    float halfWidth = building.width / 2.0f;
    float halfDepth = building.depth / 2.0f;
    float xMin = building.x - halfWidth;
    float xMax = building.x + halfWidth;
    float zMin = building.z - halfDepth;
    float zMax = building.z + halfDepth;
    float windowHeight_ = 2.2f;
    float windowWidth_ = 1.8f;
    float floorHeight_ = 4.5f;

    for (int floor_idx = 0; floor_idx < building.numFloors; floor_idx += floorStep) {
        float yPos = (floor_idx * floorHeight_) + (floorHeight_ / 2.0f);
        if (yPos + windowHeight_/2.0f > building.height) continue;

        int windowsX = std::max(1, (int)(building.width / (windowWidth_ * 1.8f)));
        int windowsZ = std::max(1, (int)(building.depth / (windowWidth_ * 1.8f)));

        for (int i = 0; i < windowsX; i += windowStep) {
            if (!isWindowLit(building.seed, floor_idx, i, 0) && !isWindowLit(building.seed, floor_idx, i, 1)) continue;

            float xPosWin = xMin + (i + 0.5f) * (building.width / windowsX);

            if (isWindowLit(building.seed, floor_idx, i, 0)) {
                glBegin(GL_QUADS);
                glVertex3f(xPosWin - windowWidth_/2, yPos - windowHeight_/2, zMax + 0.01f);
                glVertex3f(xPosWin + windowWidth_/2, yPos - windowHeight_/2, zMax + 0.01f);
                glVertex3f(xPosWin + windowWidth_/2, yPos + windowHeight_/2, zMax + 0.01f);
                glVertex3f(xPosWin - windowWidth_/2, yPos + windowHeight_/2, zMax + 0.01f);
                glEnd();
            }

            if (isWindowLit(building.seed, floor_idx, i, 1)) {
                glBegin(GL_QUADS);
                glVertex3f(xPosWin - windowWidth_/2, yPos - windowHeight_/2, zMin - 0.01f);
                glVertex3f(xPosWin + windowWidth_/2, yPos - windowHeight_/2, zMin - 0.01f);
                glVertex3f(xPosWin + windowWidth_/2, yPos + windowHeight_/2, zMin - 0.01f);
                glVertex3f(xPosWin - windowWidth_/2, yPos + windowHeight_/2, zMin - 0.01f);
                glEnd();
            }
        }

        for (int i = 0; i < windowsZ; i += windowStep) {
            if (!isWindowLit(building.seed, floor_idx, i, 2) && !isWindowLit(building.seed, floor_idx, i, 3)) continue;

            float zPosWin = zMin + (i + 0.5f) * (building.depth / windowsZ);

            if (isWindowLit(building.seed, floor_idx, i, 2)) {
                glBegin(GL_QUADS);
                glVertex3f(xMax + 0.01f, yPos - windowHeight_/2, zPosWin - windowWidth_/2);
                glVertex3f(xMax + 0.01f, yPos - windowHeight_/2, zPosWin + windowWidth_/2);
                glVertex3f(xMax + 0.01f, yPos + windowHeight_/2, zPosWin + windowWidth_/2);
                glVertex3f(xMax + 0.01f, yPos + windowHeight_/2, zPosWin - windowWidth_/2);
                glEnd();
            }

            if (isWindowLit(building.seed, floor_idx, i, 3)) {
                glBegin(GL_QUADS);
                glVertex3f(xMin - 0.01f, yPos - windowHeight_/2, zPosWin - windowWidth_/2);
                glVertex3f(xMin - 0.01f, yPos - windowHeight_/2, zPosWin + windowWidth_/2);
                glVertex3f(xMin - 0.01f, yPos + windowHeight_/2, zPosWin + windowWidth_/2);
                glVertex3f(xMin - 0.01f, yPos + windowHeight_/2, zPosWin - windowWidth_/2);
                glEnd();
            }
        }
    }

    glEnable(GL_LIGHTING);
}

// Fungsi untuk menggambar satu ruas jalan.
void drawStreet(const Street& street, bool printDebug) {
    float midX = (street.x1 + street.x2) / 2.0f;
    float midZ = (street.z1 + street.z2) / 2.0f;
    float dx_cam_street = midX - cameraX;
    float dz_cam_street = midZ - cameraZ;
    float distSquared_street = dx_cam_street * dx_cam_street + dz_cam_street * dz_cam_street;
    const float STREET_MAIN_CULL_DIST_SQ = 2200.0f * 2200.0f;

    if (distSquared_street > STREET_MAIN_CULL_DIST_SQ) return;

    glDisable(GL_TEXTURE_2D);

    float dx_street_vec = street.x2 - street.x1;
    float dz_street_vec = street.z2 - street.z1;
    float length = sqrt(dx_street_vec * dx_street_vec + dz_street_vec * dz_street_vec);

    if (length < 0.01f || street.width <= 0.01f) {
        return;
    }

    float dirX = dx_street_vec / length;
    float dirZ = dz_street_vec / length;
    float perpX = -dirZ;
    float perpZ = dirX;
    float halfWidth = street.width / 2.0f;

    glColor3f(0.12f, 0.12f, 0.15f);

    float y_surface = 0.05f;

    float P1x = street.x1 + perpX * halfWidth;
    float P1z = street.z1 + perpZ * halfWidth;
    float P2x = street.x1 - perpX * halfWidth;
    float P2z = street.z1 - perpZ * halfWidth;
    float P3x = street.x2 - perpX * halfWidth;
    float P3z = street.z2 - perpZ * halfWidth;
    float P4x = street.x2 + perpX * halfWidth;
    float P4z = street.z2 + perpZ * halfWidth;

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(P1x, y_surface, P1z);
    glVertex3f(P4x, y_surface, P4z);
    glVertex3f(P3x, y_surface, P3z);
    glVertex3f(P2x, y_surface, P2z);
    glEnd();

    const float STREET_DETAIL_CULL_DIST_SQ = 800.0f * 800.0f;
    if (distSquared_street > STREET_DETAIL_CULL_DIST_SQ) return;

    glDisable(GL_LIGHTING);
    glColor3f(0.8f, 0.8f, 0.2f);
    glLineWidth(3.0f);

    int numDashes = (int)(length / 12.0f);
    if (length > 0.01f && numDashes == 0) numDashes = 1;
    float dashLen = 6.0f;

    glBegin(GL_LINES);
    for (int i = 0; i < numDashes; i += 2) {
        float segmentLength = (numDashes == 0) ? length : length / (float)numDashes;
        float startPos = i * segmentLength;
        float endPos = startPos + dashLen;
        float t1 = startPos / length;
        float t2 = std::min(1.0f, endPos / length);

        if (length <= dashLen) {
            t1 = 0.0f;
            t2 = 1.0f;
            if (i > 0) break;
        }
        else if (t1 >= t2 && i == 0) {
            t1 = 0.0f;
            t2 = std::min(1.0f, dashLen / length);
        }
        else if (t1 >= t2) {
            continue;
        }

        glVertex3f(street.x1 + t1 * dx_street_vec, y_surface + 0.05f, street.z1 + t1 * dz_street_vec);
        glVertex3f(street.x1 + t2 * dx_street_vec, y_surface + 0.05f, street.z1 + t2 * dz_street_vec);
    }
    glEnd();

    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
}

// Fungsi untuk menggambar persimpangan jalan.
void drawIntersection(const Intersection& intersection) {
    float dx_int_draw = intersection.x - cameraX;
    float dz_int_draw = intersection.z - cameraZ;
    const float INTERSECTION_CULL_DIST_SQ = 1800.0f * 1800.0f;

    if (dx_int_draw*dx_int_draw + dz_int_draw*dz_int_draw > INTERSECTION_CULL_DIST_SQ) return;

    glDisable(GL_TEXTURE_2D);
    glColor3f(0.15f, 0.15f, 0.18f);

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 1.0f, 0.0f);
    float intersectionY = 0.06f;
    glVertex3f(intersection.x, intersectionY, intersection.z);
    float radius = STREET_WIDTH * 0.75f;
    int segments = 16;

    for (int i = 0; i <= segments; i++) {
        float angle_ = i * 2.0f * MPI / segments;
        glVertex3f(intersection.x + radius * cos(angle_), intersectionY, intersection.z + radius * sin(angle_));
    }
    glEnd();
}

// Fungsi untuk menggambar satu lampu jalan.
void drawStreetLight(const StreetLight& light, bool activateOpenGLLightSource, GLenum glLightID) {
    float dx_sl_draw = light.x - cameraX;
    float dz_sl_draw = light.z - cameraZ;

    if (dx_sl_draw * dx_sl_draw + dz_sl_draw * dz_sl_draw > STREETLIGHT_VISUAL_CULL_DIST_SQ) return;

    glDisable(GL_TEXTURE_2D);
    glColor3f(0.25f, 0.25f, 0.3f);

    glPushMatrix();
    glTranslatef(light.x, light.y / 2.0f, light.z);
    glScalef(0.25f, light.y, 0.25f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glColor3f(0.35f, 0.35f, 0.35f);
    glPushMatrix();
    glTranslatef(light.x, light.y + 0.4f, light.z);
    glutSolidSphere(0.5f, 8, 8);
    glPopMatrix();

    if (activateOpenGLLightSource) {
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        glColor4f(1.0f, 1.0f, 0.8f, 0.2f);
        glPushMatrix();
        glTranslatef(light.x, light.y + 0.4f, light.z);
        glutSolidSphere(5.5f, 12, 12);
        glPopMatrix();

        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
    }

    if (activateOpenGLLightSource && glLightID >= GL_LIGHT1 && glLightID <= GL_LIGHT7) {
        GLfloat lightPos[] = {light.x, light.y + 0.3f, light.z, 1.0f};
        GLfloat lightAmb[] = {0.0f, 0.0f, 0.0f, 1.0f};
        GLfloat lightDif[] = {0.95f, 0.95f, 0.8f, 1.0f};
        GLfloat lightSpec[] = {0.5f, 0.5f, 0.4f, 1.0f};

        glEnable(glLightID);
        glLightfv(glLightID, GL_POSITION, lightPos);
        glLightfv(glLightID, GL_AMBIENT, lightAmb);
        glLightfv(glLightID, GL_DIFFUSE, lightDif);
        glLightfv(glLightID, GL_SPECULAR, lightSpec);
        glLightf(glLightID, GL_CONSTANT_ATTENUATION, 0.25f);
        glLightf(glLightID, GL_LINEAR_ATTENUATION, 0.020f);
        glLightf(glLightID, GL_QUADRATIC_ATTENUATION, 0.004f);
    }
}

// Fungsi untuk menggambar satu mobil.
void drawCar(const Car& car) {
    float dx_cam_car_draw = car.x - cameraX;
    float dz_cam_car_draw = car.z - cameraZ;
    const float CAR_CULL_DIST_SQ = 1200.0f * 1200.0f;

    if (dx_cam_car_draw * dx_cam_car_draw + dz_cam_car_draw * dz_cam_car_draw > CAR_CULL_DIST_SQ) return;

    glDisable(GL_TEXTURE_2D);
    glPushMatrix();

    float originalBodyHeight = 0.9f;
    float scaledBodyHalfHeight = (CAR_SCALE_FACTOR * originalBodyHeight) / 2.0f;
    glTranslatef(car.x, car.y + scaledBodyHalfHeight, car.z);

    glRotatef(car.direction * 180.0f / MPI, 0, 1, 0);

    glColor3f(car.r, car.g, car.b);
    glPushMatrix();
    glScalef(CAR_SCALE_FACTOR * 2.8f, CAR_SCALE_FACTOR * originalBodyHeight, CAR_SCALE_FACTOR * 1.4f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glColor3f(car.r * 0.7f, car.g * 0.7f, car.b * 0.7f);
    glPushMatrix();
    glTranslatef(CAR_SCALE_FACTOR * 0.25f, CAR_SCALE_FACTOR * 0.8f, CAR_SCALE_FACTOR * 0.0f);
    glScalef(CAR_SCALE_FACTOR * 1.4f, CAR_SCALE_FACTOR * 0.7f, CAR_SCALE_FACTOR * 1.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    const float HEADLIGHT_DIST_SQ = 150.0f * 150.0f;
    if (dx_cam_car_draw * dx_cam_car_draw + dz_cam_car_draw * dz_cam_car_draw < HEADLIGHT_DIST_SQ) {
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 0.8f);

        glPushMatrix();
        glTranslatef(CAR_SCALE_FACTOR * 1.35f, CAR_SCALE_FACTOR * 0.15f, CAR_SCALE_FACTOR * 0.55f);
        glutSolidSphere(CAR_SCALE_FACTOR * 0.12f, 6, 6);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(CAR_SCALE_FACTOR * 1.35f, CAR_SCALE_FACTOR * 0.15f, CAR_SCALE_FACTOR * -0.55f);
        glutSolidSphere(CAR_SCALE_FACTOR * 0.12f, 6, 6);
        glPopMatrix();

        glEnable(GL_LIGHTING);
    }

    glPopMatrix();
}

// Fungsi untuk menggambar minimap.
void drawMinimap() {
    glPushAttrib(GL_VIEWPORT_BIT | GL_SCISSOR_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT |
                GL_TRANSFORM_BIT | GL_LINE_BIT | GL_POINT_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport((GLint)minimapScreenX, (GLint)minimapScreenY,
               (GLsizei)minimapScreenWidth, (GLsizei)minimapScreenHeight);
    glScissor((GLint)minimapScreenX, (GLint)minimapScreenY,
             (GLsizei)minimapScreenWidth, (GLsizei)minimapScreenHeight);
    glEnable(GL_SCISSOR_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    float mapVisibleRadius = minimapWorldExtent / 2.0f;
    gluOrtho2D(-mapVisibleRadius, mapVisibleRadius, -mapVisibleRadius, mapVisibleRadius);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_FOG);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glColor4f(0.15f, 0.15f, 0.15f, 0.85f);
    int circle_points = 60;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0f, 0.0f);
    for (int i = 0; i <= circle_points; i++) {
        float angle = 2.0f * MPI * float(i) / float(circle_points);
        glVertex2f(mapVisibleRadius * cosf(angle), mapVisibleRadius * sinf(angle));
    }
    glEnd();

    glColor4f(0.7f, 0.7f, 0.7f, 0.9f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < circle_points; i++) {
        float angle = 2.0f * MPI * float(i) / float(circle_points);
        glVertex2f(mapVisibleRadius * cosf(angle), mapVisibleRadius * sinf(angle));
    }
    glEnd();
    glLineWidth(1.0f);

    glRotatef(cameraYaw - 90.0f, 0.0f, 0.0f, 1.0f);

    glTranslatef(-cameraX, -cameraZ, 0.0f);

    glColor3f(0.4f, 0.4f, 0.45f);
    float mapStreetVisualWidth = minimapWorldExtent * 0.008f;
    if (mapVisibleRadius < minimapWorldExtent / 2.0f) {
        mapStreetVisualWidth = mapVisibleRadius * 2.0f * 0.008f;
    }

    float hw = mapStreetVisualWidth / 2.0f;

    for (const auto& street : streets) {
        float dx = street.x2 - street.x1;
        float dz = street.z2 - street.z1;
        float len = sqrt(dx * dx + dz * dz);
        if (len == 0) continue;
        float perpX = -dz / len;
        float perpZ = dx / len;

        glBegin(GL_QUADS);
        glVertex2f(street.x1 + perpX * hw, street.z1 + perpZ * hw);
        glVertex2f(street.x1 - perpX * hw, street.z1 - perpZ * hw);
        glVertex2f(street.x2 - perpX * hw, street.z2 - perpZ * hw);
        glVertex2f(street.x2 + perpX * hw, street.z2 + perpZ * hw);
        glEnd();
    }

    glColor3f(1.0f, 0.0f, 0.0f);
    glPointSize(6.0f);
    glEnable(GL_POINT_SMOOTH);
    glBegin(GL_POINTS);
    for (const auto& robot : robots) {
        if (robot.alive) {
            glVertex2f(robot.rx, robot.rz);
        }
    }
    glEnd();
    glPointSize(1.0f);
    glDisable(GL_POINT_SMOOTH);

    glPopMatrix();

    float playerArrowSize = mapVisibleRadius * 0.07f;

    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_TRIANGLES);
        glVertex2f(0.0f, 0.60f * playerArrowSize);
        glVertex2f(-0.40f * playerArrowSize, -0.40f * playerArrowSize);
        glVertex2f(0.40f * playerArrowSize, -0.40f * playerArrowSize);
    glEnd();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glDisable(GL_SCISSOR_TEST);
    glPopAttrib();

    glViewport(0, 0, windowWidth, windowHeight);
}
