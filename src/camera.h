#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "vstd/vtypes.h"
#include "vstd/vmath.h"
enum e_CAMERA_MOVEMENT
{
	FORWARD, 
	BACKWARD,
	LEFT,
	RIGHT
};

enum e_CAMERA_MODE
{
    FREE,
    FPS
};

constexpr auto YAW = -90.0f;
constexpr auto PITCH = 0.0f;
constexpr auto SPEED = 2.5f;
constexpr auto SENSITIVITY = 0.1f;
constexpr auto ZOOM = 45.0f;
constexpr auto MAX_ZOOM = 45.0f;



class Camera
{
public: 
	vec3 Position;
	vec3 Front;
	vec3 Up;
	vec3 Right;
	vec3 WorldUp;

	f32 Yaw;
	f32 Pitch;

	f32 MovementSpeed;
	f32 MouseSensitivity;
	f32 Zoom;
    e_CAMERA_MODE Mode;

	Camera
	(
		vec3 position = vec3(0.0f, 0.0f, 0.0f), 
		vec3 up = vec3(0.0f, 1.0f, 0.0f), 
		f32 yaw = YAW, float pitch = PITCH, e_CAMERA_MODE mode = FPS
	) : 
		Position(position),
		Front(0.0f, 0.0f, -1.0f), 
		WorldUp(up), Yaw(yaw), Pitch(pitch),
		MovementSpeed(SPEED), 
		MouseSensitivity(SENSITIVITY), 
		Zoom(ZOOM), Mode(mode)
		{
           
			UpdateCameraVectors();
		}

	glm::mat4 GetViewMatrix()
	{

		return glm::lookAt( glm::vec3(Position.x, Position.y, Position.z),
                            glm::vec3(Position.x + Front.x, Position.y + Front.y, Position.z + Front.z),
                            glm::vec3(Up.x, Up.y, Up.z));
	}
    void ProcessKeyboard(e_CAMERA_MOVEMENT direction, float deltaTime)
    {
        vec3 NewFront = Front;
        vec3 NewRight = Right;
        if (Mode == FPS)
        {
            NewFront = Vec3Normalize({ Front.x, 0.0f, Front.z });
            NewRight = Vec3Normalize(Vec3Cross(NewFront, WorldUp));
        }
        

        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Vec3xScalar(NewFront, velocity);
        if (direction == BACKWARD)
            Position -= Vec3xScalar(NewFront, velocity);
        if (direction == LEFT)
            Position -= Vec3xScalar(NewRight, velocity);
        if (direction == RIGHT)
            Position += Vec3xScalar(NewRight, velocity);
    }

    void ProcessMouseMovement(f32 xoffset, f32 yoffset, bool constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw   += xoffset;
        Pitch += yoffset;

        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        UpdateCameraVectors();
    }

    void ProcessMouseScroll(f32 yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > MAX_ZOOM)
            Zoom = MAX_ZOOM;
    }
private:
    void UpdateCameraVectors()
    {
        vec3 CurrFront;
        CurrFront.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        CurrFront.y = sin(glm::radians(Pitch));
        CurrFront.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = Vec3Normalize(CurrFront);

        Right = Vec3Normalize(Vec3Cross(Front, WorldUp));
        Up    = Vec3Normalize(Vec3Cross(Right, Front));
    }
};





#endif
