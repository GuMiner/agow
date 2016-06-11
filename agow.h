#pragma once
#include <Bullet\btBulletDynamicsCommon.h>
#include <SFML\System.hpp>
#include <vector>
#include "Config\GraphicsConfig.h"
#include "Config\KeyBindingConfig.h"
#include "Config\PhysicsConfig.h"
#include "Managers\FontManager.h"
#include "Managers\ImageManager.h"
#include "Managers\ModelManager.h"
#include "Managers\ShaderManager.h"
#include "Math\MathOps.h"
#include "Math\PhysicsOps.h"
#include "Utils\Constants.h"

#include "Scenery.h"
#include "Statistics.h"
#include "Viewer.h"

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
    ShaderManager shaderManager;

    // Game data
    Statistics statistics;
    Viewer viewer;
    Scenery scenery;

    // Bullet Physics
    btCollisionConfiguration *collisionConfiguration;
    btCollisionDispatcher *collisionDispatcher;
    btBroadphaseInterface *broadphaseCollisionDetector;
    btConstraintSolver *constraintSolver;
    btDiscreteDynamicsWorld *dynamicsWorld;
    
    btAlignedObjectArray<btCollisionShape*> collisionShapes;
    
    unsigned int testCubeModel;
    btAlignedObjectArray<btRigidBody*> movingTestBodies;
    
    // Physics setup.
    Constants::Status LoadPhysics();
    void UnloadPhysics();

    // Logs graphical settings so we have an idea of the OpenGL capabilities of the running machine.
    void LogGraphicsSettings();

    // Updates the camera perspective when the screen width / height has changed.
    void UpdatePerspective(unsigned int width, unsigned int height);

    // Handles GUI-based events, such as closing the application, resizing the window, etc.
    void HandleEvents(sf::RenderWindow& window, bool& alive, bool& focusPaused, bool& escapePaused);
    
    // Performs game updates.
    void Update(float currentGameTime);

    // Renders the scene.
    void Render(sf::RenderWindow& window, vec::mat4& viewMatrix);

public:
    // Used just for data storage.
    static Constants Constant;
    static MathOps MathOp;
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

    // Deinitializes data that must be freed before program exit.
    void Deinitialize();
};

