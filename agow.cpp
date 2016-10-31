#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm\gtc\quaternion.hpp>
#include "Weapons\PlasmaWeapon.h"
#include "Utils\Logger.h"
#include "Map.h"
#include "agow.h"
#include "version.h"

// Imports so that they're actually readable and not hidden away in a config file.
#pragma comment(lib, "opengl32")
#pragma comment(lib, "lib/glfw3.lib")
#pragma comment(lib, "lib/glew32.lib")
#pragma comment(lib, "lib/sfml-audio")
#pragma comment(lib, "lib/sfml-system")
#pragma comment(lib, "lib/BulletCollision")
#pragma comment(lib, "lib/BulletDynamics")
#pragma comment(lib, "lib/LinearMath")

// Static definitions.
Constants agow::Constant;
PhysicsOps agow::PhysicsOp;

agow::agow()
    : graphicsConfig("config/graphics.txt"), keyBindingConfig("config/keyBindings.txt"), physicsConfig("config/physics.txt"),
      physics(), shaderManager(), imageManager(), modelManager(&imageManager),
      regionManager(&shaderManager, &modelManager, &physics, "ContourTiler/rasters", glm::ivec2(5, 17), glm::ivec2(40, 52), 10), // All pulled from the Contour tiler, TODO configurable, make distance ~10
      scenery(), player(&modelManager, &physics), npcManager(&player, &physics)
{
}

Constants::Status agow::LoadPhysics()
{
    if (!debugDrawer.LoadBasics(&shaderManager))
    {
        return Constants::Status::BAD_PHYSICS;
    }

    if (!physics.LoadPhysics(&debugDrawer))
    {
        return Constants::Status::BAD_PHYSICS;
    }

    RockGenerator rockGenerator;
    PhysicsGenerator::AddCollisionModels(rockGenerator.GetModelPoints(&modelManager));

    SignGenerator signGenerator;
    PhysicsGenerator::AddCollisionModels(signGenerator.GetModelPoints(&modelManager));

    npcManager.LoadNpcPhysics(&physics, &regionManager);
    
    glm::vec2 spawnPoint = Map::GetPoint(Map::PLAYER);
    float spawnHeight = 2.0f + regionManager.GetPointHeight(&physics, spawnPoint);
    player.LoadPlayerPhysics(&physics, glm::vec3(spawnPoint.x, spawnPoint.y, spawnHeight), 70);

    testCar.offset.x = (spawnPoint.x + 5.0f);
    testCar.offset.y = (spawnPoint.y + 5.0f);
    testCar.offset.z = (spawnHeight + 3.0f);
    testCar.SetupPhysics(&physics);

    return Constants::Status::OK;
}

void agow::UnloadPhysics()
{
    // Delete our test data.
    regionManager.CleanupPhysics(&physics);
    npcManager.UnloadNpcPhysics(&physics);
    player.UnloadPlayerPhysics(&physics);
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

    Logger::Log("Max Texture Units: ", maxTextureUnits, ", Max Uniform Size: ", (maxUniformBlockSize/1024), " kB");
    Logger::Log("Max Vertex Uniform Blocks: ", maxVertexUniformBlocks, ", Max Fragment Uniform Blocks: ", maxFragmentUniformBlocks);
    Logger::Log("Max Texture Size: ", maxTextureSize);
}

// Performs initialization that can be done without a GPU context.
Constants::Status agow::Initialize()
{
    Input::SetupErrorCallback();

    // Setup GLFW
    if (!glfwInit())
    {
        Logger::LogError("GLFW startup failure");
        return Constants::Status::BAD_GLFW;
    }

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
    // Log graphics information for future reference
    LogGraphicsSettings();

    // TODO configurable.
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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

    Logger::Log("Loading dialog shaders...");
    if (!dialogPane.LoadBasics(&fontManager, &shaderManager))
    {
        return Constants::Status::BAD_DIALOG;
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

    Logger::Log("Key NPC loading...");
    npcManager.LoadGraphics(&fontManager);

    Logger::Log("Custom weapons loading...");
    if (!PlasmaWeapon::LoadGraphics(&shaderManager))
    {
        return Constants::Status::BAD_SHADERS;
    }

    Logger::Log("Building generator loading...");
    if (!BuildingGenerator::LoadBuilder("AI/lowDensityBuildingTree.txt", "AI/highDensityBuildingTree.txt"))
    {
        return Constants::Status::BAD_CONFIG;
    }

    Logger::Log("Building model loading...");
    if (!BuildingGenerator::LoadBuildingModels(&modelManager))
    {
        return Constants::Status::BAD_MODEL;
    }

    Logger::Log("Rock loading...");
    if (!RockGenerator::LoadModels(&modelManager))
    {
        return Constants::Status::BAD_MODEL;
    }

    Logger::Log("Sign loading...");
    if (!SignGenerator::LoadModels(&modelManager))
    {
        return Constants::Status::BAD_MODEL;
    }

    Logger::Log("Car loading...");
    if (!Car::LoadModels(&modelManager))
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

    modelManager.FinalizeLoadedModels();

    Logger::Log("Loading physics...");
    Constants::Status status = LoadPhysics();
    if (status != Constants::Status::OK)
    {
        return status;
    }

    StyleText styleText;
    styleText.speakerName = "Synthia Superfast";
    styleText.color = glm::vec3(1.0f, 1.0f, 0.0f);
    styleText.effect = StyleText::Effect::NORMAL;
    styleText.text = std::string("This is a very long test string that will probably be paged into several substrings but I don't really know as it is only for testing. but I don't really know as it is only for testing. but I don't really know as it is only for testing. but I don't really know as it is only for testing. but I don't really know as it is only for testing. but I don't really know as it is only for testing.");
    dialogPane.QueueText(styleText);

    styleText.speakerName = "Georgia Grandisimo";
    styleText.color = glm::vec3(1.0f, 0.0f, 1.0f);
    styleText.effect = StyleText::Effect::MINI;
    styleText.text = std::string("This is a very long test string that will probably be paged into several substrings but I don't really know as it is only for testing but I don't really know as it is only for testing. but I don't really know as it is only for testing. but I don't really know as it is only for testing. but I don't really know as it is only for testing. but I don't really know as it is only for testing.");
    dialogPane.QueueText(styleText);

    styleText.speakerName = "Alberta Awsaleisa";
    styleText.color = glm::vec3(0.0f, 1.0f, 1.0f);
    styleText.effect = StyleText::Effect::ITALICS;
    styleText.text = std::string("This is a very long test string that will probably be paged into several substrings but I don't really know as it is only for testing.  but I don't really know as it is only for testing. but I don't really know as it is only for testing. but I don't really know as it is only for testing. but I don't really know as it is only for testing.");
    dialogPane.QueueText(styleText);

    // TODO test code remove.
    events.AddPeriodicEvent(TypedCallback<EventType>(EventType::SKY_FLAVOR_RANDOM_COLOR_CHANGE, &scenery), 1.0f, false);

    return Constants::Status::OK;
}

void agow::HandleEvents(GLFWwindow* window, bool& focusPaused, bool& escapePaused)
{
    glfwPollEvents();
    focusPaused = !Input::hasFocus;
    escapePaused = Input::IsKeyTyped(GLFW_KEY_ESCAPE);

    // TODO move to more relevant areas.
    // TODO configurable.
    if (Input::IsKeyTyped(GLFW_KEY_I))
    {
        glm::vec3 pos = player.GetPosition();
        glm::vec2 orientation = player.Get2DOrientation();
        std::cout << "[" << pos.x << " " << pos.y << " " << pos.z << "] " << orientation.x << " | " << orientation.y << std::endl;
    }

    if (Input::IsKeyTyped(GLFW_KEY_N))
    {
        dialogPane.Advance();
    }

    if (Input::IsKeyTyped(GLFW_KEY_C))
    {
        npcManager.Converse(&events, &dialogPane);
    }

    if (Input::IsKeyTyped(GLFW_KEY_T))
    {
        std::cout << "Reload status: " << regionManager.GetTerrainManager().ReloadTerrainShader() << std::endl;
    }
}

bool fillMode = true;
void agow::Update(float currentGameTime, float frameTime)
{
    player.Update(frameTime, regionManager.GetPointTerrainType(&physics, player.GetTerrainPosition()));

    npcManager.Update(currentGameTime, frameTime);
    
    if (Input::IsKeyTyped(GLFW_KEY_B))
    {
        if (debugDrawer.ShouldRender())
        {
            debugDrawer.setDebugMode(btIDebugDraw::DebugDrawModes::DBG_NoDebug);
        }
        else
        {
            // WIREFRAME will crash the debug drawer as it's way, way too much to draw.
            debugDrawer.setDebugMode(btIDebugDraw::DebugDrawModes::DBG_DrawAabb);
        }
    }

    if (Input::IsKeyTyped(GLFW_KEY_G))
    {
        if (fillMode)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        fillMode = !fillMode;
    }

    scenery.Update(frameTime);

    regionManager.UpdateVisibleRegion(player.GetPosition(), player.Get2DOrientation(), &physics);
    regionManager.SimulateVisibleRegions(currentGameTime, frameTime);

    // Update useful statistics that are fancier than the standard GUI
    statistics.UpdateRunTime(currentGameTime, frameTime);
    statistics.UpateSector(regionManager.GetCurrentCenterTile(player.GetPosition()));

    // Update all events we have
    events.UpdateEvents(currentGameTime, frameTime);
}

void agow::Render(GLFWwindow* window, glm::mat4& viewMatrix)
{
    glm::mat4 projectionMatrix = Constants::PerspectiveMatrix * viewMatrix;
    glm::mat4 rotationOnlyMatrix = Constants::PerspectiveMatrix * glm::mat4_cast(player.GetViewOrientation());

    // Clear the screen (and depth buffer) before any rendering begins.
    const GLfloat color[] = { 0, 0, 0, 1 };
    const GLfloat one = 1.0f;
    glClearBufferfv(GL_COLOR, 0, color);
    glClearBufferfv(GL_DEPTH, 0, &one);

    // Render the scenery
    scenery.Render(rotationOnlyMatrix, player.GetPosition());

    // Render our ground, and any derivative items from that.
    regionManager.RenderRegions(Constants::PerspectiveMatrix, player.GetPosition(), player.Get2DOrientation(), viewMatrix);

    modelManager.FinalizeRender(projectionMatrix);

    // Render the NPCs
    npcManager.Render(&fontManager, &modelManager, projectionMatrix);

    // Player rendering
    player.Render(&modelManager, projectionMatrix);

    // TODO test code
    testCar.Render(&modelManager, projectionMatrix);

    // Renders the statistics. Note that this just takes the perspective matrix, not accounting for the viewer position.
    statistics.RenderStats(Constants::PerspectiveMatrix);

    // Render our dialog pane.
    dialogPane.Render(Constants::PerspectiveMatrix);

    if (debugDrawer.ShouldRender())
    {
        debugDrawer.Render(projectionMatrix);
    }
}

Constants::Status agow::Run()
{
    // 24 depth bits, 8 stencil bits, 8x AA, major version 4.
    Logger::Log("Graphics Initializing...");
    
    GLFWmonitor* monitor = GraphicsConfig::IsFullscreen ? glfwGetPrimaryMonitor() : nullptr;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    glfwWindowHint(GLFW_STENCIL_BITS, 16);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(GraphicsConfig::ScreenWidth, GraphicsConfig::ScreenHeight, "Advanced Graphics-Open World", monitor, nullptr);
    if (!window)
    {
        Logger::LogError("Could not create the GLFW window!");
        return Constants::Status::BAD_GLFW;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    Input::Setup(window, GraphicsConfig::ScreenWidth, GraphicsConfig::ScreenHeight);

    // Setup GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        Logger::LogError("GLEW startup failure: ", err, ".");
        return Constants::Status::BAD_GLEW;
    }

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
    bool focusPaused = false;
    bool escapePaused = false;
    glm::mat4 viewMatrix;
    while (!glfwWindowShouldClose(window))
    {
        clockStartTime = clock.getElapsedTime();
        viewMatrix = player.GetViewMatrix();

        float frameTime = std::min(frameClock.restart().asSeconds(), 0.06f);
        HandleEvents(window, focusPaused, escapePaused);
        
        // Run the game and render if not paused.
        if (!focusPaused && !escapePaused)
        {
            float gameTime = clock.getElapsedTime().asSeconds();
            
            // Update physics.
            physics.Step(frameTime);
            Update(gameTime, frameTime);

            Render(window, viewMatrix);
            glfwSwapBuffers(window);
        }

        // Delay to run approximately at our maximum framerate.
        sf::Int64 sleepDelay = ((long)1e6 / Constants::MAX_FRAMERATE) - (long)(frameTime * 1e6);
        if (sleepDelay > 0)
        {
            sf::sleep(sf::microseconds(sleepDelay));
        }
    }

    UnloadGraphics();
    glfwDestroyWindow(window);
    window = nullptr;

    return Constants::Status::OK;
}

void agow::UnloadGraphics()
{
    PlasmaWeapon::UnloadGraphics();
}

void agow::Deinitialize()
{
    UnloadPhysics();
    glfwTerminate();
}

// Runs the main application.
int main(int argc, char* argv[])
{
    std::cout << "agow Start!" << std::endl;

    // Startup 'static' stuff
    agow::Constant = Constants();
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
