#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include "Math\MatrixOps.h"
#include "Utils\Logger.h"
#include "Map.h"
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
      physics(), shaderManager(), imageManager(), modelManager(&imageManager),
      regionManager(&shaderManager, &modelManager, &physics, "ContourTiler/rasters", 1000, vec::vec2i(5, 17), vec::vec2i(40, 52), 15), // All pulled from the Contour tiler, TODO move to config, make distance ~10
      scenery(),
      player(), // TODO configurable
      gearScientist("James Blanton", "Giver of yer gear.", NPC::Shape::DIAMOND, vec::vec4(0.0f, 1.0f, 0.10f, 0.80f), NPC::INVULNERABLE),
      intellScientist("Aaron Krinst", "Giver of yer data.", NPC::Shape::DIAMOND, vec::vec4(0.0f, 0.20f, 1.0f, 0.70f), NPC::INVULNERABLE),
      generalMilitary("Barry Ingleson", "Nominal strategy director.", NPC::Shape::CUBOID, vec::vec4(1.0f, 0.10f, 0.0f, 0.90f), NPC::INVULNERABLE),
      sergeantMilitary("Oliver Yttrisk", "Battle assistant extraordinaire.", NPC::Shape::CUBOID, vec::vec4(1.0f, 0.50f, 0.0f, 0.50f), NPC::INVULNERABLE)
{
}

Constants::Status agow::LoadPhysics()
{
    RockGenerator rockGenerator;
    if (!physics.LoadPhysics(rockGenerator.GetModelPoints(&modelManager)))
    {
        return Constants::Status::BAD_PHYSICS;
    }

    // TODO configurable.
    vec::vec2 gearSciPos = Map::GetPoint(Map::POI::GEAR_SCIENTIST);
    vec::vec2 intelSciPos = Map::GetPoint(Map::POI::INTELLIGENCE_SCIENTIST);
    vec::vec2 generalMilPos = Map::GetPoint(Map::POI::GENERAL_MILITARY);
    vec::vec2 sergeantMilPos = Map::GetPoint(Map::POI::SERGEANT_MILITARY);
    gearScientist.LoadNpcPhysics(physics, vec::vec3(gearSciPos.x, gearSciPos.y, 2 + regionManager.GetPointHeight(physics.DynamicsWorld, gearSciPos)), 100);
    intellScientist.LoadNpcPhysics(physics, vec::vec3(intelSciPos.x, intelSciPos.y, 2 + regionManager.GetPointHeight(physics.DynamicsWorld, intelSciPos)), 90);
    generalMilitary.LoadNpcPhysics(physics, vec::vec3(generalMilPos.x, generalMilPos.y, 2 + regionManager.GetPointHeight(physics.DynamicsWorld, generalMilPos)), 80);
    sergeantMilitary.LoadNpcPhysics(physics, vec::vec3(sergeantMilPos.x, sergeantMilPos.y, 2 + regionManager.GetPointHeight(physics.DynamicsWorld, sergeantMilPos)), 65);
    
    vec::vec2 spawnPoint = Map::GetPoint(Map::PLAYER);
    player.LoadPlayerPhysics(physics, vec::vec3(spawnPoint.x, spawnPoint.y, 200), 70);

    return Constants::Status::OK;
}

void agow::UnloadPhysics()
{
    // Delete our test data.
    regionManager.CleanupPhysics(physics.DynamicsWorld);

    gearScientist.UnloadNpcPhysics(physics);
    intellScientist.UnloadNpcPhysics(physics);
    generalMilitary.UnloadNpcPhysics(physics);
    sergeantMilitary.UnloadNpcPhysics(physics);
    player.UnloadPlayerPhysics(physics);

    for (const PhysicalModel& model : testCubes)
    {
        physics.DynamicsWorld->removeRigidBody(model.rigidBody);
        physics.DeleteBody(model.rigidBody);
    }

    physics.UnloadPhysics();
}

void agow::LogGraphicsSettings()
{
    Logger::Log("OpenGL vendor: ", glGetString(GL_VENDOR), ", version ", glGetString(GL_VERSION), ", renderer ", glGetString(GL_RENDERER));
    Logger::Log("OpenGL extensions: ", glGetString(GL_EXTENSIONS));

    GLint maxTextureUnits, maxUniformBlockSize;
    GLint maxVertexUniformBlocks, maxFragmentUniformBlocks;
    GLint maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTextureUnits);
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &maxVertexUniformBlocks);
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &maxFragmentUniformBlocks);
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

    Logger::Log("Max Texture Units: ", ", Max Uniform Size: ", (maxUniformBlockSize/1024), " kB");
    Logger::Log("Max Vertex Uniform Blocks: ", maxVertexUniformBlocks, ", Max Fragment Uniform Blocks: ", maxFragmentUniformBlocks);
    Logger::Log("Max Texture Size: ", maxTextureSize);
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

    // Enable line, but not polygon smoothing.
    glEnable(GL_LINE_SMOOTH);

    // Let OpenGL shaders determine point sizes.
    glEnable(GL_PROGRAM_POINT_SIZE);

    // Disable face culling so that see-through flat objects and stuff at 1.0 (cube map, text) work.
    glDisable(GL_CULL_FACE);
    glFrontFace(GL_CW);

    // Cutout faces that are hidden by other faces.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    return LoadAssets();
}

Constants::Status agow::LoadAssets()
{
    // Scenery
    Logger::Log("Scenery loading...");
    if (!scenery.Initialize(shaderManager))
    {
        return Constants::Status::BAD_SCENERY;
    }

    // Fonts
    Logger::Log("Font loading...");
    if (!fontManager.LoadFont(&shaderManager, "fonts/DejaVuSans.ttf"))
    {
        return Constants::Status::BAD_FONT;
    }

    // Statistics
    Logger::Log("Statistics loading...");
    if (!statistics.Initialize(&fontManager))
    {
        return Constants::Status::BAD_STATS;
    }

    Logger::Log("Region graphics loading...");
    if (!regionManager.InitializeGraphics())
    {
        return Constants::Status::BAD_TERRAIN;
    }

    Logger::Log("NPC model loading...");
    if (!NPC::LoadNpcModels(&modelManager))
    {
        return Constants::Status::BAD_MODEL;
    }

    gearScientist.LoadGraphics(&fontManager);
    intellScientist.LoadGraphics(&fontManager);
    generalMilitary.LoadGraphics(&fontManager);
    sergeantMilitary.LoadGraphics(&fontManager);

    Logger::Log("Rock loading...");
    if (!RockGenerator::LoadModels(&modelManager))
    {
        return Constants::Status::BAD_MODEL;
    }

    Logger::Log("Player model loading...");
    if (!player.LoadPlayerModel(&modelManager))
    {
        return Constants::Status::BAD_MODEL;
    }

    // Now that *all* the models have loaded, prepare for rendering models by initializing OpenGL and sending the model data to OpenGL
    Logger::Log("Sending model VAO to OpenGL...");
    if (!modelManager.InitializeOpenGlResources(shaderManager))
    {
        return Constants::Status::BAD_SHADERS;
    }

    modelManager.ResetOpenGlModelData();

    Logger::Log("Loading physics...");
    Constants::Status status = LoadPhysics();
    if (status != Constants::Status::OK)
    {
        return status;
    }

    // TODO test code remove.
    events.AddPeriodicEvent(TypedCallback<EventType>(EventType::SKY_FLAVOR_RANDOM_COLOR_CHANGE, &scenery), 1.0f, false);

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

            // TODO remove 
            if (event.key.code == sf::Keyboard::I)
            {
                vec::vec3 pos = player.GetViewPosition();
                std::cout << "[" << pos.x << " " << pos.y << " " << pos.z << "]" << std::endl;
            }
        }
        else if (event.type == sf::Event::MouseButtonPressed)
        {
            // if (event.mouseButton.button == sf::Mouse::Left
        }
    }
}

bool wasPressed = false;
void agow::Update(float currentGameTime, float frameTime)
{
    player.Update(frameTime);

    gearScientist.Update(currentGameTime, frameTime);
    intellScientist.Update(currentGameTime, frameTime);
    generalMilitary.Update(currentGameTime, frameTime);
    sergeantMilitary.Update(currentGameTime, frameTime);
    
    // TODO test code
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
    {
        player.Warp(&regionManager, physics.DynamicsWorld, player.GetTerrainPosition());
    }

    // TODO test code
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F) && !wasPressed)
    {
        // Fire a cube for collision tests.
        vec::vec3 pos = player.GetViewPosition() + 5.0f * player.GetViewOrientation().forwardVector();
        vec::vec3 vel = 40.0f * player.GetViewOrientation().forwardVector();

        PhysicalModel model;
        BasicPhysics::CShape shape;
        RockGenerator rockGenerator;
        rockGenerator.GetRandomRockModel(&model.modelId, &shape);

        model.rigidBody = physics.GetDynamicBody(shape, btVector3(pos.x, pos.y, pos.z), 20.0f);
        model.rigidBody->setLinearVelocity(btVector3(vel.x, vel.y, vel.z));
        testCubes.push_back(model);

        physics.DynamicsWorld->addRigidBody(model.rigidBody);
        wasPressed = true;
    }
    else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F) && wasPressed)
    {
        wasPressed = false;
    }

    regionManager.UpdateVisibleRegion(player.GetViewPosition(), physics.DynamicsWorld);
    regionManager.SimulateVisibleRegions(currentGameTime, frameTime);

    // Update useful statistics that are fancier than the standard GUI
    statistics.UpdateRunTime(currentGameTime, frameTime);
    statistics.UpateSector(regionManager.GetCurrentCenterTile(player.GetViewPosition()));

    // Update all events we have
    events.UpdateEvents(currentGameTime, frameTime);

    // Update physics.
    physics.Step(frameTime);
}

void agow::Render(sf::RenderWindow& window, vec::mat4& viewMatrix)
{
    vec::mat4 projectionMatrix = Constants::PerspectiveMatrix * viewMatrix;
    vec::mat4 rotationOnlyMatrix = Constants::PerspectiveMatrix * player.GetViewOrientation().asMatrix();

    // Clear the screen (and depth buffer) before any rendering begins.
    const GLfloat color[] = { 0, 0, 0, 1 };
    const GLfloat one = 1.0f;
    glClearBufferfv(GL_COLOR, 0, color);
    glClearBufferfv(GL_DEPTH, 0, &one);

    // Render the scenery
    scenery.Render(rotationOnlyMatrix);

    // Render our ground, and any derivative items from that.
    regionManager.RenderRegions(Constants::PerspectiveMatrix, viewMatrix);

    // Render all the moving objects
    for (unsigned int i = 0; i < testCubes.size(); i++)
    {
        vec::mat4 mvMatrix = BasicPhysics::GetBodyMatrix(testCubes[i].rigidBody);
        modelManager.RenderModel(projectionMatrix, testCubes[i].modelId, mvMatrix, false);
    }

    // Render the key NPCs
    gearScientist.Render(&fontManager, &modelManager, projectionMatrix);
    intellScientist.Render(&fontManager, &modelManager, projectionMatrix);
    generalMilitary.Render(&fontManager, &modelManager, projectionMatrix);
    sergeantMilitary.Render(&fontManager, &modelManager, projectionMatrix);

    // Player rendering
    player.Render(&modelManager, projectionMatrix);

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
    sf::Clock frameClock;
    sf::Time clockStartTime;
    bool alive = true;
    bool focusPaused = false;
    bool escapePaused = false;
    vec::mat4 viewMatrix;
    while (alive)
    {
        clockStartTime = clock.getElapsedTime();
        viewMatrix = player.GetViewOrientation().asMatrix() * MatrixOps::Translate(-player.GetViewPosition());

        float frameTime = frameClock.restart().asSeconds();
        HandleEvents(window, alive, focusPaused, escapePaused);
        
        // Run the game and render if not paused.
        if (!focusPaused && !escapePaused)
        {
            float gameTime = clock.getElapsedTime().asSeconds();
            Update(gameTime, frameTime);

            Render(window, viewMatrix);

            // Display what we rendered.
            UpdatePerspective(window.getSize().x, window.getSize().y);
            window.display();
        }

        // Delay to run approximately at our maximum framerate.
        sf::Int64 sleepDelay = ((long)1e6 / Constants::MAX_FRAMERATE) - (long)(frameTime * 1e6);
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
        Logger::LogError("Could not initialize agow: ", runStatus);
    }

    // Wait before closing for display purposes.
    Logger::Log("Application End!");
    Logger::Shutdown();
    std::cout << "agow End!" << std::endl;

    sf::sleep(sf::milliseconds(1000));

    // Log is auto-shutdown.
    return (int)runStatus;
}
