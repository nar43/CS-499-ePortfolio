/* This file originated from website LearnOpenGL.com, which distributes the code
with the following information regarding licensing:

All code samples, unless explicitly stated otherwise, are licensed under the terms 
of the CC BY-NC 4.0 license as published by Creative Commons, either version 4 of 
the License, or (at your option) any later version. You can find a human-readable format of the license

https://creativecommons.org/licenses/by-nc/4.0/

and the full license

https://creativecommons.org/licenses/by-nc/4.0/legalcode
*/

#ifndef CAMERA_H
#define CAMERA_H

// Camera utility header — no GL loader required here.

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <vector>

// Camera movement options
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera {
public:
    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // Euler Angles
    float Yaw;
    float Pitch;
    // Camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // Constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = YAW,
        float pitch = PITCH)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
        MovementSpeed(SPEED),
        MouseSensitivity(SENSITIVITY),
        Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;

        // Initialize caching values so first call recomputes
        _lastComputedYaw = Yaw + 1.0f;
        _lastComputedPitch = Pitch + 1.0f;
        updateCameraVectors();
    }

    // Returns the view matrix
    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // Process keyboard input
    void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
        if (direction == UP)
            Position += WorldUp * velocity;
        if (direction == DOWN)
            Position -= WorldUp * velocity;
    }

    // Process mouse movement (note: uses plain bool to avoid GL headers here)
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        if (constrainPitch) {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }
        updateCameraVectors();
    }

    // Process mouse scroll
    void ProcessMouseScroll(float yoffset) {
        Zoom -= yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

private:
    // --- Cache fields (private) ---
    float _lastComputedYaw;
    float _lastComputedPitch;
    glm::vec3 _cachedFront;
    glm::vec3 _cachedRight;
    glm::vec3 _cachedUp;

    // Optimized: only recalc when yaw/pitch changed
    void updateCameraVectors() {
        // If orientation unchanged, reuse cached vectors
        if (Yaw == _lastComputedYaw && Pitch == _lastComputedPitch) {
            Front = _cachedFront;
            Right = _cachedRight;
            Up = _cachedUp;
            return;
        }

        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);

        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));

        // Update cache
        _cachedFront = Front;
        _cachedRight = Right;
        _cachedUp = Up;
        _lastComputedYaw = Yaw;
        _lastComputedPitch = Pitch;
    }
};

#endif // CAMERA_H
