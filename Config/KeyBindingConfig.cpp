#include "KeyBindingConfig.h"

// TODO make these letters and do the mapping here.
int KeyBindingConfig::MoveLeft;
int KeyBindingConfig::MoveRight;
int KeyBindingConfig::MoveForward;
int KeyBindingConfig::MoveBackward;
int KeyBindingConfig::MoveUp;
int KeyBindingConfig::MoveDown;

bool KeyBindingConfig::LoadConfigValues(std::vector<std::string>& configFileLines)
{
    return (ReadKey(configFileLines, MoveLeft, "Error decoding the move left key!") &&
        ReadKey(configFileLines, MoveRight, "Error reading in the move right key!") &&
        ReadKey(configFileLines, MoveForward, "Error reading in the move forward key!") &&
        ReadKey(configFileLines, MoveBackward, "Error reading in the move backward key!") &&
        ReadKey(configFileLines, MoveUp, "Error reading in the move up key!") &&
        ReadKey(configFileLines, MoveDown, "Error reading in the move down key!"));
}

void KeyBindingConfig::WriteConfigValues()
{
    WriteKey("MoveLeft", MoveLeft);
    WriteKey("MoveRight", MoveRight);
    WriteKey("MoveForward", MoveForward);
    WriteKey("MoveBackward", MoveBackward);
    WriteKey("MoveUp", MoveUp);
    WriteKey("MoveDown", MoveDown);
}

KeyBindingConfig::KeyBindingConfig(const char* configName)
    : ConfigManager(configName)
{
}
