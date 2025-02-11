#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW             =  -90.0f;
const float PITCH           =  0.0f;
const float SPEED           =  5.0f;
const float SENSITIVITY     =  0.001f;
// Default bobbing values
const float BOBBING_SIZE = 0.125f;
const float BOBBING_SPEED = 5.0f;
const glm::vec3 BOBBING_VEC = glm::vec3(0);

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
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    // Bobbing values
    float bobbingSize           = BOBBING_SIZE;
    float bobbingSpeed          = BOBBING_SPEED;
    glm::vec3 previousBobbing   = BOBBING_VEC;

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;

        // calculating camera bobbing
        float time = glfwGetTime();
        float cosBobbing = cos(time*bobbingSpeed)*bobbingSize;
        float sinBobbing = glm::abs(sin(time*bobbingSpeed)*bobbingSize);
        glm::vec3 bobbing = glm::vec3 (cosBobbing*sin(glm::radians(Yaw)),sinBobbing,(1-cosBobbing)*cos(glm::radians(Yaw)));
        glm::vec3 deltaBobbing = previousBobbing - bobbing;
        previousBobbing = bobbing;

        if (direction == FORWARD)
            Position += Front * velocity + deltaBobbing;
        if (direction == BACKWARD)
            Position -= Front * velocity + deltaBobbing;
        if (direction == LEFT)
            Position -= Right * velocity + deltaBobbing;
        if (direction == RIGHT)
            Position += Right * velocity + deltaBobbing;
        // camera position boundaries
        if(Position.x > 74.0f)
            Position.x = 74.0f;
        if(Position.x < -74.0f)
            Position.x = -74.0f;
        if(Position.z > 74.0f)
            Position.z = 74.0f;
        if(Position.z < -74.0f)
            Position.z = -74.0f;
        Position.y = 0.0f + previousBobbing.y;
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw   += xoffset;
        Pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // changing to running/walking speed
    void speedUp(){
        MovementSpeed = SPEED * 2.5f;
        bobbingSpeed = BOBBING_SPEED * 2.5f;
        bobbingSize = BOBBING_SIZE * 1.3f;
    }
    void slowDown(){
        MovementSpeed = SPEED;
        bobbingSpeed = BOBBING_SPEED;
        bobbingSize = BOBBING_SIZE;
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};
#endif