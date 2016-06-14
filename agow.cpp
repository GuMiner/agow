#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include "Math\MatrixOps.h"
#include "Math\VecOps.h"
#include "Utils\Logger.h"
#include "agow.h"
#include "version.h"

// Imports so that they're actually readable and not hidden away in a config file.
#pragma comment(lib, "opengl32")

#ifndef _DEBUG
    #pragma comment(lib, "lib/glew32.lib")
    #pragma comment(lib, "lib/sfml-audio")
    #pragma comment(lib, "lib/sfml-system")
    #pragma comment(lib, "lib/sfml-window")
    #pragma comment(lib, "lib/sfml-graphics")
    #pragma comment(lib, "lib/BulletCollision")
    #pragma comment(lib, "lib/BulletDynamics")
    #pragma comment(lib, "lib/LinearMath")
#else
    #pragma comment(lib, "lib/glew32d.lib")
    #pragma comment(lib, "lib/sfml-audio-d")
    #pragma comment(lib, "lib/sfml-system-d")
    #pragma comment(lib, "lib/sfml-window-d")
    #pragma comment(lib, "lib/sfml-graphics-d")
    #pragma comment(lib, "lib/BulletCollision_Debug")
    #pragma comment(lib, "lib/BulletDynamics_Debug")
    #pragma comment(lib, "lib/LinearMath_Debug")
#endif

// Static definitions.
Constants agow::Constant;
MathOps agow::MathOp;
PhysicsOps agow::PhysicsOp;

agow::agow()
    : graphicsConfig("config/graphics.txt"), keyBindingConfig("config/keyBindings.txt"), physicsConfig("config/physics.txt"),
      imageManager(), modelManager(&imageManager), 
      scenery(&modelManager, &imageManager),
      viewer()
{
}

Constants::Status agow::LoadPhysics()
{
    collisionConfiguration = new btDefaultCollisionConfiguration();
    collisionDispatcher = new btCollisionDispatcher(collisionConfiguration);
    broadphaseCollisionDetector = new btDbvtBroadphase();
    constraintSolver = new btSequentialImpulseConstraintSolver();
    dynamicsWorld = new btDiscreteDynamicsWorld(collisionDispatcher, broadphaseCollisionDetector,
        constraintSolver, collisionConfiguration);

    dynamicsWorld->setGravity(btVector3(0, 0, -9.80f));


    // TODO ideally this is a dictionary somewhere.
    // TODO all of the new objects should be deleted, which they are not.
    // In the long term, most of the collision will be player / terrain / box items.
    collisionShapes.push_back(new btBoxShape(btVector3(1000.0f, 1000.0f, 10.0f)));
    collisionShapes.push_back(new btBoxShape(btVector3(50.0f, 50.0f, 50.0f)));

    // Add a static ground shape
    
    // inertia is zero as this is immobile.
    btTransform pos;
    pos.setIdentity();
    pos.setOrigin(btVector3(0, 0, -9.80f));

    btDefaultMotionState *motionState = new btDefaultMotionState(pos);
    btRigidBody::btRigidBodyConstructionInfo ground(0.0f, motionState, collisionShapes[0]);
    dynamicsWorld->addRigidBody(new btRigidBody(ground));

    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 40; j++)
        {
            btTransform pos;
            pos.setIdentity();
            pos.setOrigin(btVector3(111 * i, 20 + j * 15, 5 + 111 * j));

            btVector3 localInertia;
            collisionShapes[1]->calculateLocalInertia(20.0f, localInertia);
            btDefaultMotionState *motionState = new btDefaultMotionState(pos);
            btRigidBody::btRigidBodyConstructionInfo object(20.0f, motionState, collisionShapes[1], localInertia);
            
            btRigidBody* body = new btRigidBody(object);
            movingTestBodies.push_back(body);

            dynamicsWorld->addRigidBody(body);
        }
    }

    return Constants::Status::OK;
}

void agow::UnloadPhysics()
{
    // Delete collision shapes
    for (int i = 0; i < collisionShapes.size(); i++)
    {
        delete collisionShapes[i];
    }

    // Delete basic setup of physics
    delete dynamicsWorld;
    delete constraintSolver;
    delete broadphaseCollisionDetector;
    delete collisionDispatcher;
    delete collisionConfiguration;
}

void agow::LogGraphicsSettings()
{
    Logger::Log("OpenGL vendor: ", glGetString(GL_VENDOR), ", version ", glGetString(GL_VERSION), ", renderer ", glGetString(GL_RENDERER));
    Logger::Log("OpenGL extensions: ", glGetString(GL_EXTENSIONS));

    GLint maxTextureUnits, maxUniformBlockSize;
    GLint maxVertexUniformBlocks, maxFragmentUniformBlocks;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTextureUnits);
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &maxVertexUniformBlocks);
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &maxFragmentUniformBlocks);
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize);

    Logger::Log("Max Texture Units: ", ", Max Uniform Size: ", (maxUniformBlockSize/1024), " kB");
    Logger::Log("Max Vertex Uniform Blocks: ", maxVertexUniformBlocks, ", Max Fragment Uniform Blocks: ", maxFragmentUniformBlocks);
}

void agow::UpdatePerspective(unsigned int width, unsigned int height)
{
    // Letterboxing is done at the top and bottom.
    float necessaryWidth = (float)height * Constants::ASPECT;
    if (necessaryWidth > width)
    {
        // Letterbox the top and the bottom of the screen so that the aspect ratio is met
        float effectiveHeight = (float)width / Constants::ASPECT;
        float heightDelta = ((float)height - effectiveHeight) / 2.0f;
        glViewport(0, (int)heightDelta, (GLsizei)width, (GLsizei)effectiveHeight);
    }
    else
    {
        // Letterbox the left and the right so that the aspect ratio is met.
        float widthDelta = ((float)width - necessaryWidth) / 2.0f;
        glViewport((GLint)widthDelta, (GLint)0, (GLsizei)necessaryWidth, (GLsizei)height);
    }
}

// Performs initialization that can be done without a GPU context.
Constants::Status agow::Initialize()
{
    Logger::Log("Loading graphics config file...");
    if (!graphicsConfig.ReadConfiguration())
    {
        Logger::Log("Bad graphics config file!");
        return Constants::Status::BAD_CONFIG;
    }

    Logger::Log("Loading key binding config file...");
    if (!keyBindingConfig.ReadConfiguration())
    {
        Logger::Log("Bad key binding config file!");
        return Constants::Status::BAD_CONFIG;
    }

    Logger::Log("Loading physics config file...");
    if (!physicsConfig.ReadConfiguration())
    {
        Logger::Log("Bad physics config file!");
        return Constants::Status::BAD_CONFIG;
    }

    Logger::Log("Configuration loaded!");

    Logger::Log("Loading physics...");
    Constants::Status status = LoadPhysics();
    if (status != Constants::Status::OK)
    {
        Logger::Log("Bad physics loading!");
        return status;
    }
    Logger::Log("Physics loaded!");

    return Constants::Status::OK;
}

Constants::Status agow::LoadGraphics()
{
    // Setup GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        Logger::LogError("GLEW startup failure: ", err, ".");
        return Constants::Status::BAD_GLEW;
    }

    // Log graphics information for future reference
    LogGraphicsSettings();

    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable line and polygon smoothing
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);

    // Multisample if available
    glEnable(GL_MULTISAMPLE);

    // Let OpenGL shaders determine point sizes.
    glEnable(GL_PROGRAM_POINT_SIZE);

    // Disable face culling so that see-through flat objects and stuff at 1.0 (cube map, text) work.
    glDisable(GL_CULL_FACE);
    glFrontFace(GL_CW);

    // Cutout faces that are hidden by other faces.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Ensure cube maps (skybox) are seemless and clamp to the edges 
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return LoadAssets();
}

Constants::Status agow::LoadAssets()
{
    // Scenery
    Logger::Log("Scenery loading...");
    if (!scenery.Initialize(shaderManager))
    {
        Logger::Log("Bad scenery");
        return Constants::Status::BAD_SCENERY;
    }

    Logger::Log("Scenery loading done!");

    // Fonts
    Logger::Log("Font loading...");
    if (!fontManager.LoadFont(&shaderManager, "fonts/DejaVuSans.ttf"))
    {
        return Constants::Status::BAD_FONT;
    }

    Logger::Log("Font loading done!");

    // Statistics
    Logger::Log("Statistics loading...");
    if (!statistics.Initialize(&fontManager))
    {
        return Constants::Status::BAD_STATS;
    }

    // Load a cube for testing.
    testCubeModel = modelManager.LoadModel("models/cube");

    // Now that *all* the models have loaded, prepare for rendering models by initializing OpenGL and sending the model data to OpenGL
    Logger::Log("Sending model VAO to OpenGL...");
    if (!modelManager.InitializeOpenGlResources(shaderManager))
    {
        return Constants::Status::BAD_SHADERS;
    }

    modelManager.ResetOpenGlModelData();

    return Constants::Status::OK;
}

void agow::HandleEvents(sf::RenderWindow& window, bool& alive, bool& focusPaused, bool& escapePaused)
{
    // Handle all events.
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            alive = false;
        }
        else if (event.type == sf::Event::LostFocus)
        {
            focusPaused = true;
        }
        else if (event.type == sf::Event::GainedFocus)
        {
            focusPaused = false;
        }
        else if (event.type == sf::Event::Resized)
        {
            UpdatePerspective(event.size.width, event.size.height);
        }
        else if (event.type == sf::Event::KeyReleased)
        {
            if (event.key.code == sf::Keyboard::Escape)
            {
                // The Escape key is the consistent pause / unpause
                escapePaused = !escapePaused;
            }
            else if (event.key.code == sf::Keyboard::Q)
            {
                alive = false;
            }
        }
        else if (event.type == sf::Event::MouseButtonPressed)
        {
            // if (event.mouseButton.button == sf::Mouse::Left
        }
    }
}

void agow::Update(float currentGameTime)
{
    viewer.InputUpdate();

    // Update useful statistics that are fancier than the standard GUI
    statistics.UpdateRunTime(currentGameTime);
    statistics.UpdateViewPos(viewer.GetViewPosition(), viewer.GetViewOrientation().asMatrix());
}

void agow::Render(sf::RenderWindow& window, vec::mat4& viewMatrix)
{
    vec::mat4 projectionMatrix = Constants::PerspectiveMatrix * viewMatrix;

    // Clear the screen (and depth buffer) before any rendering begins.
    const GLfloat color[] = { 0, 0, 0, 1 };
    const GLfloat one = 1.0f;
    glClearBufferfv(GL_COLOR, 0, color);
    glClearBufferfv(GL_DEPTH, 0, &one);

    // Render the scenery
    scenery.Render(viewMatrix, projectionMatrix);

    // Render all the moving objects
    for (int i = 0; i < movingTestBodies.size(); i++)
    {
        btRigidBody* body = movingTestBodies[i];
        btTransform worldTransform;
        body->getMotionState()->getWorldTransform(worldTransform);

        vec::quaternion rotation = VecOps::Convert(worldTransform.getRotation());
        vec::mat4 mvMatrix = MatrixOps::Translate(VecOps::Convert(worldTransform.getOrigin())) * rotation.asMatrix();
        
        if (worldTransform.getOrigin().getZ() < 5.0f)
        {
            worldTransform.setIdentity();
            worldTransform.setOrigin(btVector3(MathOps::Rand() * 800.0f, MathOps::Rand() * 800.0f, 1000.0f));
            body->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
            body->activate(true); // Only if it stops moving. It technically doesn't here.
            body->setWorldTransform(worldTransform);
            body->getMotionState()->setWorldTransform(worldTransform);
        }

        modelManager.RenderModel(projectionMatrix, testCubeModel, mvMatrix, false);
    }

    // Renders the statistics. Note that this just takes the perspective matrix, not accounting for the viewer position.
    statistics.RenderStats(Constants::PerspectiveMatrix);
}

Constants::Status agow::Run()
{
    // 24 depth bits, 8 stencil bits, 8x AA, major version 4.
    Logger::Log("Graphics Initializing...");
    sf::ContextSettings contextSettings = sf::ContextSettings(24, 8, 8, 4, 0);

    sf::Uint32 style = GraphicsConfig::IsFullscreen ? sf::Style::Fullscreen : sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close;
    sf::RenderWindow window(sf::VideoMode(GraphicsConfig::ScreenWidth, GraphicsConfig::ScreenHeight), "Advanced Graphics-Open World", style, contextSettings);

    // Now that we have an OpenGL Context, load our graphics.
    Constants::Status firstTimeSetup = LoadGraphics();
    if (firstTimeSetup != Constants::Status::OK)
    {
        return firstTimeSetup;
    }

    Logger::Log("Graphics Initialized!");

    sf::Clock clock;
    sf::Clock guiClock;
    sf::Time clockStartTime;
    bool alive = true;
    bool focusPaused = false;
    bool escapePaused = false;
    vec::mat4 viewMatrix;
    while (alive)
    {
        clockStartTime = clock.getElapsedTime();
        viewMatrix = viewer.GetViewOrientation().asMatrix() * MatrixOps::Translate(-viewer.GetViewPosition());

        HandleEvents(window, alive, focusPaused, escapePaused);
        Update(clock.getElapsedTime().asSeconds());
        dynamicsWorld->stepSimulation(1.0f / 60.0f, 10);

        // Render, only if non-paused.
        if (!focusPaused && !escapePaused)
        {
            Render(window, viewMatrix);

            // Display what we rendered.
            UpdatePerspective(window.getSize().x, window.getSize().y);
            window.display();
        }

        // Delay to run approximately at our maximum framerate.
        sf::Int64 sleepDelay = (1000000 / Constants::MAX_FRAMERATE) - clock.getElapsedTime().asMicroseconds() - clockStartTime.asMicroseconds();
        sf::sleep(sf::microseconds(sleepDelay));
    }

    return Constants::Status::OK;
}

void agow::Deinitialize()
{
    UnloadPhysics();
}

// Runs the main application.
int main(int argc, char* argv[])
{
    std::cout << "agow Start!" << std::endl;

    // Startup 'static' stuff
    agow::Constant = Constants();
    agow::MathOp = MathOps();
    agow::PhysicsOp = PhysicsOps();

    Logger::Setup();

    Logger::Log("agow ", AutoVersion::MAJOR_VERSION, ".", AutoVersion::MINOR_VERSION, ".");

    Constants::Status runStatus;
    std::unique_ptr<agow> agow(new agow());

    // Run the application.
    runStatus = agow->Initialize();
    if (runStatus == Constants::Status::OK)
    {
        runStatus = agow->Run();
        agow->Deinitialize();
    }
    else
    {
        Logger::LogError("Could not initialize AGOW!");
    }

    // Wait before closing for display purposes.
    Logger::Log("Application End!");
    Logger::Shutdown();
    std::cout << "agow End!" << std::endl;

    sf::sleep(sf::milliseconds(1000));

    // Log is auto-shutdown.
    return (int)runStatus;
}
