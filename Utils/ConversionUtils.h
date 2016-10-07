#pragma once
#include <string>
#include <GLFW\glfw3.h>
#include <glm\vec3.hpp>

class ConversionUtils
{
    public:
        static bool LoadBool(const std::string& line, bool& boolean);
        static bool LoadInt(const std::string& line, int& integer);
        static bool LoadFloat(const std::string& line, float& floatingPoint);
        static bool LoadKey(const std::string& line, int& key);
        static bool LoadVector(const std::string& line, glm::vec3& vector);
};
