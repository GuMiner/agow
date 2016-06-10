#pragma once
#include <GL\glew.h>

// Miscellaneous image utils. Unless otherwise specified, these methods *do not have input checking*. Be careful!
class ImageUtils
{
public:
    // Copies a portion of the src image into the dest image (which is a sideLength square), optionally applying a rotation.
    // 0 = no rotation, 1 = 90 degree CW rotation, 2 = 180, 3 = 270.
    static void CopyImage(unsigned char* src, unsigned char* dest, int srcWidth, int xOffset, int yOffset, int sideLength, int rotation);

    // Gets an image from a file.. Returns true (and populates data with the RGBA image, width and height with the width and height) when successful.
    // For standard textures, use the ImageManager class instead of these APIs
    static bool GetRawImage(const char* filename, unsigned char** data, int* width, int* height);
    
    // Frees an image loaded from GetRawImage above.
    static void FreeRawImage(unsigned char* imageData);
};
