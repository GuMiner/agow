#pragma once
class ImageUtils
{
public:
    ImageUtils();
    static bool LoadImage(const char* filename, int* width, int* height, unsigned char** data);
    static void FreeImage(unsigned char* data);
    ~ImageUtils();
};

