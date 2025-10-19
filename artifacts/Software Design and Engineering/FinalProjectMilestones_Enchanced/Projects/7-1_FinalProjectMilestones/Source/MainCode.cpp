// maincode.cpp
// Enhanced for CS499 Milestone Two: improved error handling, safer cleanup,
// and additional logging to demonstrate software engineering improvements.

#include <iostream>         // cerr, cout
#include <cstdlib>          // EXIT_FAILURE, EXIT_SUCCESS
#include <memory>           // smart pointers
#include <string>

#include <GL/glew.h>        // GLEW library
#include "GLFW/glfw3.h"     // GLFW library

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SceneManager.h"
#include "ViewManager.h"
#include "ShapeMeshes.h"
#include "ShaderManager.h"

// Macro for window title (const so it can be referenced)
static const char* const WINDOW_TITLE = "7-1 FinalProjectMilestones (Enhanced)";

// Use raw globals as before for simplicity, but initialize to nullptr
GLFWwindow* g_Window = nullptr;
SceneManager* g_SceneManager = nullptr;
ShaderManager* g_ShaderManager = nullptr;
ViewManager* g_ViewManager = nullptr;

/* Forward declarations */
bool InitializeGLFW();
bool InitializeGLEW();

/* Safe delete helper */
template<typename T>
void SafeDelete(T*& p) {
    if (p) {
        delete p;
        p = nullptr;
    }
}

int main(int argc, char* argv[])
{
    // Initialize GLFW and bail out early on failure
    if (!InitializeGLFW()) {
        std::cerr << "ERROR: GLFW initialization failed" << std::endl;
        return EXIT_FAILURE;
    }

    // Create managers (allocate)
    try {
        g_ShaderManager = new ShaderManager();
        g_ViewManager = new ViewManager(g_ShaderManager);
    }
    catch (const std::bad_alloc& e) {
        std::cerr << "ERROR: allocation failed: " << e.what() << std::endl;
        SafeDelete(g_ShaderManager);
        SafeDelete(g_ViewManager);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // Create the main display window
    g_Window = g_ViewManager->CreateDisplayWindow(WINDOW_TITLE);
    if (g_Window == nullptr) {
        std::cerr << "ERROR: Failed to create main GLFW window" << std::endl;
        SafeDelete(g_ViewManager);
        SafeDelete(g_ShaderManager);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // Initialize GLEW
    if (!InitializeGLEW()) {
        std::cerr << "ERROR: GLEW initialization failed" << std::endl;
        SafeDelete(g_SceneManager);
        SafeDelete(g_ViewManager);
        SafeDelete(g_ShaderManager);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // Load shaders (LoadShaders may internally log failures)
    try {
        g_ShaderManager->LoadShaders(
            "../../Utilities/shaders/vertexShader.glsl",
            "../../Utilities/shaders/fragmentShader.glsl");
        g_ShaderManager->use();
    }
    catch (...) {
        std::cerr << "ERROR: exception while loading shaders" << std::endl;
        // Continue but the shader manager should log; you might want to exit
    }

    // Prepare scene manager
    try {
        g_SceneManager = new SceneManager(g_ShaderManager);
        g_SceneManager->PrepareScene();
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: preparing scene failed: " << e.what() << std::endl;
        SafeDelete(g_SceneManager);
    }

    std::cout << "\n*** KEY FUNCTIONS: ***\n";
    std::cout << "ESC - close the window and exit\n";
    std::cout << "W - zoom in\t" << "S - zoom out\n";
    std::cout << "A - pan left\t" << "D - pan right\n";
    std::cout << "Q - pan up\t" << "E - pan down\n";
    std::cout << "1 - front view (ortho)\n";
    std::cout << "2 - side view (ortho)\n";
    std::cout << "3 - top view (ortho)\n";
    std::cout << "4 - perspective view\n";

    // Main loop
    while (g_Window && !glfwWindowShouldClose(g_Window))
    {
        // Enable depth testing and blending as needed
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Clear the frame and z buffers
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Prepare view and projection
        if (g_ViewManager) {
            g_ViewManager->PrepareSceneView();
        }

        // Render scene safely
        if (g_SceneManager) {
            g_SceneManager->RenderScene();
        }

        // Buffer swap and events
        glfwSwapBuffers(g_Window);
        glfwPollEvents();
    }

    // Cleanup (safe)
    SafeDelete(g_SceneManager);
    SafeDelete(g_ViewManager);
    SafeDelete(g_ShaderManager);

    // Terminate GLFW
    glfwTerminate();

    return EXIT_SUCCESS;
}

/* Initialize GLFW with error checks */
bool InitializeGLFW()
{
    if (!glfwInit()) {
        std::cerr << "glfwInit() failed" << std::endl;
        return false;
    }

#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    return true;
}

/* Initialize GLEW */
bool InitializeGLEW()
{
    GLenum GLEWInitResult = glewInit();
    if (GLEW_OK != GLEWInitResult) {
        std::cerr << "GLEW Error: " << glewGetErrorString(GLEWInitResult) << std::endl;
        return false;
    }
    // GLEW: end -------------------------------

    // Displays a successful OpenGL initialization message
    std::cout << "INFO: OpenGL Successfully Initialized\n";
    std::cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << "\n";
    return true;
}
