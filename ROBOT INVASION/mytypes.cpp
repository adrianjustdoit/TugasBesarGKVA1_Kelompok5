#include "mytypes.h"
#include "mydraw.h"
#include <GL/glut.h>
#include <cstring>

// Konstruktor untuk kelas Button.
Button::Button(float _x, float _y, float _w, float _h, std::string _text)
    : x(_x), y(_y), width(_w), height(_h), text(_text), isHovered(false) {}

// Fungsi untuk memperbarui posisi tombol.
void Button::updatePosition(float winWidth, float winHeight, float newX, float newY) {
    x = newX;
    y = newY;
}

// Fungsi untuk menggambar tombol.
void Button::draw() {
    if (isHovered) {
        glColor3f(0.4f, 0.4f, 0.8f);
    } else {
        glColor3f(0.3f, 0.3f, 0.6f);
    }

    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    glColor3f(0.8f, 0.8f, 0.8f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    float textWidth = 0;
    for(size_t i = 0; i < text.length(); ++i)
        textWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, text[i]);

    float textX = x + (width - textWidth) / 2.0f;
    float textY = y + (height - 18.0f) / 2.0f + 5;
    drawText(textX, textY, text.c_str());
}

// Fungsi untuk memeriksa apakah tombol diklik berdasarkan koordinat mouse.
bool Button::isClicked(int mouse_x_glut, int mouse_y_glut, int winHeight) {
    float mouse_y_gl = winHeight - mouse_y_glut;
    return (mouse_x_glut >= x && mouse_x_glut <= x + width &&
            mouse_y_gl >= y && mouse_y_gl <= y + height);
}

Button startGameButton(0, 0, 220, 50, "Mulai Permainan");
Button retryButton(0, 0, 180, 50, "Coba Lagi");
Button quitButtonHomeScreen(0, 0, 180, 50, "Keluar");
Button quitButtonGameOver(0, 0, 180, 50, "Keluar");
