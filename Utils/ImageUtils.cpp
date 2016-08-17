#include <algorithm>
#include <stb\stb_image.h>
#include "Logger.h"
#include "ImageUtils.h"

// Copies a portion of the src image into the dest image (which is a sideLength square), optionally applying a rotation.
// 0 = no rotation, 1 = 90 degree CW rotation, 2 = 180, 3 = 270.
void ImageUtils::CopyImage(unsigned char* src, unsigned char* dest, int srcWidth, int xOffset, int yOffset, int sideLength, int rotation)
{
    for (int i = 0; i < sideLength; i++)
    {
        if (rotation == 0)
        {
            const unsigned char* startingLocation = src + ((i + yOffset) * srcWidth + xOffset) * 4;
            unsigned char* destinationLocation = dest + i * sideLength * 4;
            memcpy(destinationLocation, startingLocation, sizeof(unsigned char) * (sideLength * 4));
        }
        else if (rotation == 1)
        {
            const unsigned char* startingLocation = src + ((i + yOffset) * srcWidth + xOffset) * 4;
            for (int j = 0; j < sideLength; j++)
            {
                *(dest + (j * sideLength + i) * 4) = startingLocation[(sideLength - (j + 1)) * 4];
                *(dest + (j * sideLength + i) * 4 + 1) = startingLocation[(sideLength - (j + 1)) * 4 + 1];
                *(dest + (j * sideLength + i) * 4 + 2) = startingLocation[(sideLength - (j + 1)) * 4 + 2];
                *(dest + (j * sideLength + i) * 4 + 3) = startingLocation[(sideLength - (j + 1)) * 4 + 3];
            }
        }
        else if (rotation == 2)
        {
            const unsigned char* startingLocation = src + ((i + yOffset) * srcWidth + xOffset) * 4;
            unsigned char* destinationLocation = dest + (sideLength - (i + 1)) * sideLength * 4;
            
            for (int j = 0; j < sideLength; j++)
            {
                destinationLocation[(sideLength - (j + 1)) * 4] = startingLocation[j * 4];
                destinationLocation[(sideLength - (j + 1)) * 4 + 1] = startingLocation[j * 4 + 1];
                destinationLocation[(sideLength - (j + 1)) * 4 + 2] = startingLocation[j * 4 + 2];
                destinationLocation[(sideLength - (j + 1)) * 4 + 3] = startingLocation[j * 4 + 3];
            }
        }
        else if (rotation == 3)
        {
            const unsigned char* startingLocation = src + ((i + yOffset) * srcWidth + xOffset) * 4;
            for (int j = 0; j < sideLength; j++)
            {
                *(dest + ((sideLength - (j + 1)) * sideLength + (sideLength - (i + 1))) * 4) = startingLocation[(sideLength - (j + 1)) * 4];
                *(dest + ((sideLength - (j + 1)) * sideLength + (sideLength - (i + 1))) * 4 + 1) = startingLocation[(sideLength - (j + 1)) * 4 + 1];
                *(dest + ((sideLength - (j + 1)) * sideLength + (sideLength - (i + 1))) * 4 + 2) = startingLocation[(sideLength - (j + 1)) * 4 + 2];
                *(dest + ((sideLength - (j + 1)) * sideLength + (sideLength - (i + 1))) * 4 + 3) = startingLocation[(sideLength - (j + 1)) * 4 + 3];
            }
        }
    }
}

// Gets an image from a file.. Returns true (and populates data with the RGBA image, width and height with the width and height) when successful.
bool ImageUtils::GetRawImage(const char* filename, unsigned char** data, int* width, int* height)
{
    // Load in the image
    int channels = 0;
    *data = stbi_load(filename, width, height, &channels, STBI_rgb_alpha);
    if (*data && width && height)
    {
        return true;
    }
    else
    {
        std::stringstream errStream;
        errStream << "Failed to load image:" << stbi_failure_reason();
        Logger::LogError(errStream.str().c_str());
        return false;
    }
}

// Frees an image loaded from GetRawImage above.
void ImageUtils::FreeRawImage(unsigned char* imageData)
{
    stbi_image_free(imageData);
}