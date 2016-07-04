#include <iostream>
#include <stb\stb_image.h>
#include "ImageUtils.h"

ImageUtils::ImageUtils()
{
}

bool ImageUtils::LoadImage(const char* filename, int *width, int *height, unsigned char** data)
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
        std::cout << "Failed to load image " << filename << ": " << stbi_failure_reason();
        
        return false;
    }
}

void ImageUtils::FreeImage(unsigned char* data)
{
    stbi_image_free(data);
}

ImageUtils::~ImageUtils()
{
}
