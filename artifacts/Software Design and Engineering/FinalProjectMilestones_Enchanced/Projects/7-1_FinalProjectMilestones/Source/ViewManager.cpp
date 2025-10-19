// ViewManager.cpp (enhanced)
// Improvements: defensive checks for null camera, documented callbacks,
// improved projection clamping, and frame-time safety.

#include "ViewManager.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace
{
    const int WINDOW_WIDTH = 1000;
    const int WINDOW_HEIGHT = 800;
    const char* g_ViewName = "view";
    const char* g_ProjectionName = "projection";

    // camera pointer shared for the callbacks
    Camera* g_pCamera = nullptr;

    // mouse bookkeeping
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f;
    float gLastFrame = 0.0f;

    bool bOrthographicProjection = false;
}

/* Constructor */
ViewManager::ViewManager(ShaderManager* pShaderManager)
    : m_pShaderManager(pShaderManager),
    m_pWindow(nullptr)
{
    g_pCamera = new Camera();
    // default camera parameters
    g_pCamera->Position = glm::vec3(0.0f, 5.5f, 8.0f);
    g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
    g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
    g_pCamera->Zoom = 80.0f;
}

/* Destructor */
ViewManager::~ViewManager()
{
    m_pShaderManager = nullptr;
    m_pWindow = nullptr;
    if (g_pCamera) {
        delete g_pCamera;
        g_pCamera = nullptr;
    }
}

/* Create window and register callbacks */
GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, windowTitle, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);

    // set callback functions; note: static free function uses global g_pCamera
    glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);

    // capture the cursor for camera rotation
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // enable blending for transparency (set defaults)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_pWindow = window;
    return window;
}

/* Mouse position callback — defensive checks for camera existence */
void ViewManager::Mouse_Position_Callback(GLFWwindow* /*window*/, double xMousePos, double yMousePos)
{
    if (!g_pCamera) return; // defensive

    if (gFirstMouse) {
        gLastX = static_cast<float>(xMousePos);
        gLastY = static_cast<float>(yMousePos);
        gFirstMouse = false;
    }

    float xOffset = static_cast<float>(xMousePos) - gLastX;
    float yOffset = gLastY - static_cast<float>(yMousePos); // reversed

    gLastX = static_cast<float>(xMousePos);
    gLastY = static_cast<float>(yMousePos);

    // guard small offsets
    if (fabsf(xOffset) < 0.0001f && fabsf(yOffset) < 0.0001f) return;

    g_pCamera->ProcessMouseMovement(xOffset, yOffset);
}

/* Poll and process keyboard events; changed to query only when window valid */
void ViewManager::ProcessKeyboardEvents()
{
    if (!m_pWindow) return;
    if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_pWindow, true);
    }

    if (!g_pCamera) return;

    if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS) g_pCamera->ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS) g_pCamera->ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS) g_pCamera->ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS) g_pCamera->ProcessKeyboard(RIGHT, gDeltaTime);
    if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS) g_pCamera->ProcessKeyboard(UP, gDeltaTime);
    if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS) g_pCamera->ProcessKeyboard(DOWN, gDeltaTime);

    // change projections / camera presets
    if (glfwGetKey(m_pWindow, GLFW_KEY_1) == GLFW_PRESS) {
        bOrthographicProjection = true;
        g_pCamera->Position = glm::vec3(0.0f, 4.0f, 10.0f);
        g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
        g_pCamera->Front = glm::vec3(0.0f, 0.0f, -1.0f);
    }
    if (glfwGetKey(m_pWindow, GLFW_KEY_2) == GLFW_PRESS) {
        bOrthographicProjection = true;
        g_pCamera->Position = glm::vec3(10.0f, 4.0f, 0.0f);
        g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
        g_pCamera->Front = glm::vec3(-1.0f, 0.0f, 0.0f);
    }
    if (glfwGetKey(m_pWindow, GLFW_KEY_3) == GLFW_PRESS) {
        bOrthographicProjection = true;
        g_pCamera->Position = glm::vec3(0.0f, 7.0f, 0.0f);
        g_pCamera->Up = glm::vec3(-1.0f, 0.0f, 0.0f);
        g_pCamera->Front = glm::vec3(0.0f, -1.0f, 0.0f);
    }
    if (glfwGetKey(m_pWindow, GLFW_KEY_4) == GLFW_PRESS) {
        bOrthographicProjection = false;
        g_pCamera->Position = glm::vec3(0.0f, 5.5f, 8.0f);
        g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
        g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
        g_pCamera->Zoom = 80.0f;
    }
}

/* Prepare view and projection matrices every frame; performs clamping and sanity checks */
void ViewManager::PrepareSceneView()
{
    if (!m_pShaderManager) return;

    // per-frame timing
    float currentFrame = static_cast<float>(glfwGetTime());
    gDeltaTime = currentFrame - gLastFrame;
    gLastFrame = currentFrame;

    // guard a reasonable delta (avoid huge jumps)
    if (gDeltaTime < 0.0f) gDeltaTime = 0.0f;
    if (gDeltaTime > 1.0f) gDeltaTime = 1.0f;

    ProcessKeyboardEvents();

    if (!g_pCamera) return;

    glm::mat4 view = g_pCamera->GetViewMatrix();

    // projection
    glm::mat4 projection;
    if (!bOrthographicProjection) {
        // Perspective -- clamp field of view to reasonable values
        float zoom = g_pCamera->Zoom;
        if (zoom < 1.0f) zoom = 1.0f;
        if (zoom > 120.0f) zoom = 120.0f;
        projection = glm::perspective(glm::radians(zoom),
            static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT),
            0.1f, 100.0f);
    }
    else {
        // Orthographic with aspect ratio handling
        double scale = 1.0;
        if (WINDOW_WIDTH > WINDOW_HEIGHT) {
            scale = static_cast<double>(WINDOW_HEIGHT) / static_cast<double>(WINDOW_WIDTH);
            projection = glm::ortho(-5.0f, 5.0f, static_cast<float>(-5.0f * scale), static_cast<float>(5.0f * scale), 0.1f, 100.0f);
        }
        else if (WINDOW_WIDTH < WINDOW_HEIGHT) {
            scale = static_cast<double>(WINDOW_WIDTH) / static_cast<double>(WINDOW_HEIGHT);
            projection = glm::ortho(static_cast<float>(-5.0f * scale), static_cast<float>(5.0f * scale), -5.0f, 5.0f, 0.1f, 100.0f);
        }
        else {
            projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
        }
    }

    // Set uniforms
    m_pShaderManager->setMat4Value(g_ViewName, view);
    m_pShaderManager->setMat4Value(g_ProjectionName, projection);
    m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
}
