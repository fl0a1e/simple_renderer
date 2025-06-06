#pragma once

// System Headers
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;


class Camera {
public:
	// position - direct - Up - right
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	float CamerNear;
	float CamerFar;

	// euler Angles£¬ÉãÏñ»ú²»¿¼ÂÇÐý×ª
	float Pitch; // ¸©Ñö½Ç
	float Yaw; // Æ«º½½Ç

	// options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetPerspectiveMatrix(const int width, const int height) const;

	// process mouse input
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
	void ProcessMouseScroll(float yoffset);

	// process keyboard input
	void ProcessKeyboard(Camera_Movement direction, float deltaTime);

private:
	// calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors();
};