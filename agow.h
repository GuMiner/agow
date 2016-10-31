#pragma once
#include <SFML\System.hpp>
#include <vector>
#include <glm\mat4x4.hpp>
#include "Config\GraphicsConfig.h"
#include "Config\KeyBindingConfig.h"
#include "Config\PhysicsConfig.h"
#include "Data\Model.h"
#include "Generators\BuildingGenerator.h"
#include "Generators\RockGenerator.h"
#include "Generators\SignGenerator.h"
#include "Managers\FontManager.h"
#include "Managers\ImageManager.h"
#include "Managers\ModelManager.h"
#include "Managers\NpcManager.h"
#include "Managers\RegionManager.h"
#include "Managers\ShaderManager.h"
#include "Math\PhysicsOps.h"
#include "Utils\Constants.h"
#include "Vehicles\Car.h"

#include "Physics.h"
#include "DialogPane.h"
#include "Events.h"
#include "Input.h"
#include "Player.h"
#include "PhysicsDebugDrawer.h"
#include "Scenery.h"
#include "Statistics.h"

// Handles startup and the base graphics rendering loop.
class agow
{
    // Configuration
    GraphicsConfig graphicsConfig;
    KeyBindingConfig keyBindingConfig;
    PhysicsConfig physicsConfig;

    // Managers
    FontManager fontManager;
    ImageManager imageManager;
    ModelManager modelManager;
    NpcManager npcManager;
    RegionManager regionManager;
    ShaderManager shaderManager;

    // Game data
    Physics physics;
    PhysicsDebugDrawer debugDrawer;
    DialogPane dialogPane;
    Player player;
    Statistics statistics;
    Scenery scenery;

    // TODO test code remove (need to redesign this extensively when it's not so late).
    Car testCar;

    Events events;
    
    // Physics setup.
    Constants::Status LoadPhysics();
    void UnloadPhysics();

    // Logs graphical settings so we have an idea of the OpenGL capabilities of the running machine.
    void LogGraphicsSettings();

    // Handles GUI-based events, such as closing the application, resizing the window, etc.
    void HandleEvents(GLFWwindow* window, bool& focusPaused, bool& escapePaused);
    
    // Performs game updates.
    void Update(float currentGameTime, float frameTime);

    // Renders the scene.
    void Render(GLFWwindow* window, glm::mat4& viewMatrix);

public:
    // Used just for data storage.
    static Constants Constant;
    static PhysicsOps PhysicsOp;

    // Initializes agow and any construction-time setup (such as threads).
    agow();

    // Initializes data that can be setup before an OpenGL context is created.
    Constants::Status Initialize();

    // Initializes generic OpenGL data after an OpenGL context is created.
    Constants::Status LoadGraphics();

    // Loads in assets after an OpenGL context is created.
    Constants::Status LoadAssets();

    // Runs the game loop.
    Constants::Status Run();

    // Unloads any OpenGL assets that were statically loaded.
    void UnloadGraphics();

    // Deinitializes data that must be freed before program exit.
    void Deinitialize();
};

