#ifndef _CAMERA_HPP_
#define _CAMERA_HPP_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "player.h"
#include "defs.h"

#include <vector>

// Default camera values
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // camera options
    float Zoom;

    Player* player;

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f))
    {
        Position = position;
        Front = front;
        WorldUp = up;
        Zoom = ZOOM;
        updateCameraVectors();
    }

    // Camera(glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), Player* player = nullptr)
    // {
    //     WorldUp = up;
    //     Zoom = ZOOM;
    //     follow(player);
    //     updateCamera();
    // }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void follow(Player* player) {
        this->player = player;
    }

    void updateCamera() {
        glm::vec3 offset = CAMERA_TO_PLAYER_OFFSET;
        glm::vec3 position = player->position;
        glm::vec3 front    = player->front;
        glm::vec3 right    = player->right;
        this->Position = position + offset - ((2.5f - front.y) * front) + (0.3f * right);
        this->Front    = front;
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};

#endif