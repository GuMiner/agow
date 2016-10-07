#pragma once
#include <set>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Math\Vec.h"

class Input
{
    // The window.
    static GLFWwindow* window;

    // List of pressed keys.
    static std::set<int> pressedKeys;

    // The list of keys that are pressed that were checked.
    static std::set<int> pressedKeysTypeChecked;

    // Same as the key sets, but for mouse buttons.
    static std::set<int> pressedMouseButtons;
    static std::set<int> pressedMouseButtonsTypeChecked;

    // Logs any errors from GLFW
    static void LogGlfwErrors(int error, const char* description);

    // Handles GLFW key callbacks.
    static void GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    // Handles GLFW mouse button callbacks.
    static void GlfwMouseButtonCallbacks(GLFWwindow* window, int button, int action, int mods);

    // Handles GLFW window close callbacks.
    static void GlfwWindowCloseCallbacks(GLFWwindow* window);

    // Handles GLFW window focus callbacks.
    static void GlfwWindowFocusCallbacks(GLFWwindow* window, int focused);

    // Handles GLFW window resizing callbacks.
    static void GlfwWindowResizeCallbacks(GLFWwindow* window, int width, int height);

public:
    static bool hasFocus;

    static void SetupErrorCallback();

    // Sets callbacks this class uses
    static void Setup(GLFWwindow* window, int width, int height);

    // Returns true if a key was pressed, false otherwise.
    static bool IsKeyPressed(int keyId);

    // Returns true if a key was pressed and then false until the key is released.
    static bool IsKeyTyped(int keyId);

    // Same as IsKeyPressed ans IsKeyTyped, but with mouse buttons, respectively.
    static bool IsMouseButtonPressed(int mouseButton);
    static bool IsMouseButtonClicked(int mouseButton);

    static vec::vec2i GetMousePos();
};

