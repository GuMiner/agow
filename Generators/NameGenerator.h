#pragma once
#include <string>
#include <vector>

// Generates reasonably-sounding random names.
class NameGenerator
{
    static std::vector<std::string> firstNames;
    static std::vector<std::string> lastNames;

    static bool namesLoaded;
    static void LoadNames();
public:
    static std::string GetRandomName();
};

