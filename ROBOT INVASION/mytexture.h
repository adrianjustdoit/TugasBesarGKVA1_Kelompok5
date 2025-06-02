#ifndef MYTEXTURE_H
#define MYTEXTURE_H

#include "mytypes.h"

// Image loading functions
int ImageLoad(const char *filename, Image *image, bool expectAlpha = false);
Image* loadBMP(const char* filename, bool expectAlpha = false);
unsigned char* resizeTextureToPowerOfTwo(const char* srcData, int srcWidth, int srcHeight, int* newWidth, int* newHeight, int bytesPerPixel);

// Texture loading functions
void loadGroundTexture();
void loadRobotTexture();
void loadMuzzleFlashTexture();
void initTextures();

#endif 
