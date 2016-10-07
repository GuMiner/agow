#pragma once
#include <string>
#include <vector>
#include "Managers\ConfigManager.h"

class KeyBindingConfig : public ConfigManager
{
    public:
        KeyBindingConfig(const char* configName);

        static int MoveLeft;
        static int MoveRight;
        static int MoveForward;
        static int MoveBackward;

        static int MoveUp;
        static int MoveDown;
    protected:
    private:
        virtual bool LoadConfigValues(std::vector<std::string>& lines);
        virtual void WriteConfigValues();
};
