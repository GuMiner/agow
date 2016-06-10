#pragma once
#include <string>
#include <vector>
#include <SFML\Window.hpp>
#include "Managers\ConfigManager.h"

class KeyBindingConfig : public ConfigManager
{
    public:
        KeyBindingConfig(const char* configName);

        static sf::Keyboard::Key MoveLeft;
        static sf::Keyboard::Key MoveRight;
        static sf::Keyboard::Key MoveForward;
        static sf::Keyboard::Key MoveBackward;

        static sf::Keyboard::Key MoveUp;
        static sf::Keyboard::Key MoveDown;
    protected:
    private:
        virtual bool LoadConfigValues(std::vector<std::string>& lines);
        virtual void WriteConfigValues();
};
