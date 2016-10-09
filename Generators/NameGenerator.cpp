#include <glm\gtc\random.hpp>
#include "NameGenerator.h"

bool NameGenerator::namesLoaded = false;

void NameGenerator::LoadNames()
{
    // TODO configurable.
    firstNames.push_back("Amy");
    firstNames.push_back("Arthur");
    firstNames.push_back("Betty");
    firstNames.push_back("Bob");
    firstNames.push_back("Catie");
    firstNames.push_back("Cory");
    firstNames.push_back("Dorthy");
    firstNames.push_back("Dave");
    firstNames.push_back("Erica");
    firstNames.push_back("Eric");
    firstNames.push_back("Francesca");
    firstNames.push_back("Ford");
    firstNames.push_back("Georgia");
    firstNames.push_back("George");
    firstNames.push_back("Hannah");
    firstNames.push_back("Harry");
    firstNames.push_back("Ira");
    firstNames.push_back("Ike");
    firstNames.push_back("Julia");
    firstNames.push_back("James");
    firstNames.push_back("Kathy");
    firstNames.push_back("Kris");
    firstNames.push_back("Laura");
    firstNames.push_back("Lewis");
    firstNames.push_back("Megan");
    firstNames.push_back("Morris");
    firstNames.push_back("Nora");
    firstNames.push_back("Ned");
    // TODO continue filling out the alphabet.

    lastNames.push_back("Christenson");
    lastNames.push_back("Ford");
    lastNames.push_back("Gelation");
    lastNames.push_back("Morrison");
    lastNames.push_back("Prethis");
    lastNames.push_back("Smith");
    lastNames.push_back("Xie");
    // TODO think up more last names.
}

std::string NameGenerator::GetRandomName()
{
    if (!namesLoaded)
    {
        LoadNames();
        namesLoaded = true;
    }

    return firstNames[glm::linearRand(0, (int)firstNames.size() - 1)] +
        std::string(" ") +
        lastNames[glm::linearRand(0, (int)lastNames.size() - 1)];
}