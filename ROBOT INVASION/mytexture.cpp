#include "gldefs.h"
#include "mytexture.h"
#include "gamestate.h"
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>

extern "C" {
    extern unsigned char *stbi_load(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
    extern void stbi_image_free(void *retval_from_stbi_load);
    extern char *stbi_failure_reason(void);
}

// Fungsi untuk memuat gambar dari file BMP.
int ImageLoad(const char *filename, Image *image, bool expectAlpha) {
    std::cout << "Attempting to load: " << filename << std::endl;

    FILE *file;
    if ((file = fopen(filename, "rb")) == NULL) {
        std::cerr << "ERROR: File not found: " << filename << std::endl;
        return 0;
    }

    unsigned char header[54];
    if (fread(header, 1, 54, file) != 54 || header[0] != 'B' || header[1] != 'M') {
        std::cerr << "ERROR: Invalid BMP file format: " << filename << std::endl;
        fclose(file);
        return 0;
    }

    fseek(file, 18, SEEK_SET);

    unsigned short planes;
    unsigned short bpp;
    unsigned long size;

    if ((fread(&image->sizeX, 4, 1, file)) != 1) {
        std::cerr << "ERROR: Failed to read width in " << filename << std::endl;
        fclose(file);
        return 0;
    }
    if ((fread(&image->sizeY, 4, 1, file)) != 1) { fclose(file); return 0; }

    if ((fread(&planes, 2, 1, file)) != 1) { fclose(file); return 0; }
    if (planes != 1) { fclose(file); return 0; }
    if ((fread(&bpp, 2, 1, file)) != 1) { fclose(file); return 0; }

    int bytesPerPixel = bpp / 8;
    if (!(bpp == 24 && !expectAlpha) && !(bpp == 32 && expectAlpha) && !(bpp==24 && expectAlpha)){
    }
    if (bpp == 32 && expectAlpha) image->hasAlpha = true;
    else if (bpp == 24 && expectAlpha) { image->hasAlpha = false; bytesPerPixel = 3;}
    else if (bpp == 24 && !expectAlpha) { image->hasAlpha = false; bytesPerPixel = 3;}
    else {bytesPerPixel = 3; image->hasAlpha = false;}

    size = image->sizeX * image->sizeY * bytesPerPixel;
    fseek(file, 24, SEEK_CUR);
    if (bpp == 32) fseek(file, -4, SEEK_CUR);

    image->data = (char *) malloc(size);
    if (image->data == NULL) { fclose(file); return 0; }
    if ((fread(image->data, size, 1, file)) != 1) { free(image->data); fclose(file); return 0; }

    char temp;
    for (int i = 0; i < size; i += bytesPerPixel) {
        temp = image->data[i];
        image->data[i] = image->data[i + 2];
        image->data[i + 2] = temp;
    }

    fclose(file);
    return 1;
}

// Fungsi pembantu untuk memuat file BMP ke dalam struct Image.
Image* loadBMP(const char* filename, bool expectAlpha) {
    Image* image = (Image*)malloc(sizeof(Image));
    if (image == NULL) { return NULL; }
    if (!ImageLoad(filename, image, expectAlpha)) {
        free(image);
        return NULL;
    }
    return image;
}

// Fungsi untuk mengubah ukuran data tekstur ke dimensi pangkat dua terdekat.
unsigned char* resizeTextureToPowerOfTwo(const char* srcData, int srcWidth, int srcHeight, int* newWidth, int* newHeight, int bytesPerPixel) {
    *newWidth = 1;
    while (*newWidth < srcWidth) *newWidth *= 2;

    *newHeight = 1;
    while (*newHeight < srcHeight) *newHeight *= 2;

    unsigned char* newData = new unsigned char[(*newWidth) * (*newHeight) * bytesPerPixel];

    for (int y = 0; y < *newHeight; y++) {
        for (int x = 0; x < *newWidth; x++) {
            int srcX = (x * srcWidth) / *newWidth;
            int srcY = (y * srcHeight) / *newHeight;

            for (int b = 0; b < bytesPerPixel; b++) {
                newData[(y * (*newWidth) + x) * bytesPerPixel + b] =
                    srcData[(srcY * srcWidth + srcX) * bytesPerPixel + b];
            }
        }
    }

    return newData;
}

// Fungsi untuk memuat tekstur tanah.
void loadGroundTexture() {
    std::cout << "Starting to load ground texture..." << std::endl;

    Image* image = loadBMP("ground.bmp");
    if (image == NULL) {
        std::cerr << "FAILED to load ground.bmp, creating fallback texture" << std::endl;
        glGenTextures(1, &groundTexture);
        glBindTexture(GL_TEXTURE_2D, groundTexture);

        const int texSize = 64;
        unsigned char* checkData = new unsigned char[texSize * texSize * 3];
        for(int y = 0; y < texSize; y++) {
            for(int x = 0; x < texSize; x++) {
                unsigned char color = ((x & 8) == 0) ^ ((y & 8) == 0) ? 150 : 100;
                checkData[(y * texSize + x) * 3 + 0] = color;
                checkData[(y * texSize + x) * 3 + 1] = color;
                checkData[(y * texSize + x) * 3 + 2] = color;
            }
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texSize, texSize, 0, GL_RGB, GL_UNSIGNED_BYTE, checkData);
        delete[] checkData;

        std::cout << "Fallback ground texture created." << std::endl;
        return;
    }

    try {
        std::cout << "Image loaded, size: " << image->sizeX << "x" << image->sizeY << std::endl;

        std::cout << "Generating texture..." << std::endl;
        glGenTextures(1, &groundTexture);

        std::cout << "Binding texture..." << std::endl;
        glBindTexture(GL_TEXTURE_2D, groundTexture);

        std::cout << "Setting texture parameters..." << std::endl;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        bool isPowerOfTwo = (image->sizeX & (image->sizeX - 1)) == 0 && (image->sizeY & (image->sizeY - 1)) == 0;
        if (!isPowerOfTwo) {
            std::cout << "Resizing non-power-of-two texture..." << std::endl;
            int newWidth, newHeight;
            unsigned char* resizedData = resizeTextureToPowerOfTwo(
                image->data, image->sizeX, image->sizeY, &newWidth, &newHeight, 3);

            gluBuild2DMipmaps(GL_TEXTURE_2D, 3, newWidth, newHeight, GL_RGB, GL_UNSIGNED_BYTE, resizedData);

            delete[] resizedData;
        } else {
            gluBuild2DMipmaps(GL_TEXTURE_2D, 3, image->sizeX, image->sizeY, GL_RGB, GL_UNSIGNED_BYTE, image->data);
        }

        std::cout << "Freeing image resources..." << std::endl;
        free(image->data);
        free(image);
        std::cout << "Ground texture loaded successfully." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in loadGroundTexture: " << e.what() << std::endl;
        if (image) {
            if (image->data) free(image->data);
            free(image);
        }
    }
    catch (...) {
        std::cerr << "Unknown exception in loadGroundTexture" << std::endl;
        if (image) {
            if (image->data) free(image->data);
            free(image);
        }
    }
}

// Fungsi untuk memuat tekstur robot.
void loadRobotTexture() {
    std::cout << "Attempting to load robot texture using stb_image..." << std::endl;

    int width, height, channels;
    unsigned char *data = stbi_load("robot_texture.png", &width, &height, &channels, 0);

    if (data == NULL) {
        std::cout << "PNG not found, trying BMP format..." << std::endl;
        data = stbi_load("robot_texture.bmp", &width, &height, &channels, 0);
    }

    if (data == NULL) {
        std::cerr << "FAILED to load robot texture using stb_image: " << stbi_failure_reason() << std::endl;
        robotTexture = 0;
        return;
    }

    std::cout << "Robot texture loaded successfully: " << width << "x" << height << ", channels: " << channels << std::endl;

    glGenTextures(1, &robotTexture);
    glBindTexture(GL_TEXTURE_2D, robotTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLenum format;
    if (channels == 4) {
        format = GL_RGBA;
    } else if (channels == 3) {
        format = GL_RGB;
    } else if (channels == 1) {
        format = GL_LUMINANCE;
    } else {
        std::cerr << "Unsupported number of channels: " << channels << std::endl;
        stbi_image_free(data);
        robotTexture = 0;
        return;
    }

    gluBuild2DMipmaps(GL_TEXTURE_2D, format, width, height, format, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);

    std::cout << "Robot texture created with stb_image." << std::endl;
}

// Fungsi untuk memuat tekstur kilatan senjata.
void loadMuzzleFlashTexture() {
    std::cout << "Attempting to load muzzle_flash.png using stb_image..." << std::endl;

    int width, height, channels;
    unsigned char *data = stbi_load("muzzle_flash.png", &width, &height, &channels, STBI_rgb_alpha);

    if (data == NULL) {
        std::cerr << "FAILED to load muzzle_flash.png using stb_image: " << stbi_failure_reason() << std::endl;
        muzzleFlashTexture = 0;
        return;
    }

    std::cout << "Muzzle_flash.png loaded successfully: " << width << "x" << height << ", channels: " << channels << " (forced RGBA)" << std::endl;

    glGenTextures(1, &muzzleFlashTexture);
    glBindTexture(GL_TEXTURE_2D, muzzleFlashTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);

    std::cout << "Muzzle flash texture created with stb_image." << std::endl;
}

// Fungsi untuk menginisialisasi semua tekstur yang dibutuhkan game.
void initTextures() {
    std::cout << "Before loading textures..." << std::endl;

    GLboolean textureEnabled = glIsEnabled(GL_TEXTURE_2D);

    try {
        loadGroundTexture();
        loadMuzzleFlashTexture();
        loadRobotTexture();

        texturesAllValid = (groundTexture != 0) &&
                          (muzzleFlashTexture != 0) && (robotTexture != 0);

        std::cout << "Textures loaded successfully: " << (texturesAllValid ? "YES" : "NO") << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception during texture loading: " << e.what() << std::endl;
        texturesAllValid = false;
    }

    if (textureEnabled)
        glEnable(GL_TEXTURE_2D);
    else
        glDisable(GL_TEXTURE_2D);

    std::cout << "After loading textures..." << std::endl;
}
