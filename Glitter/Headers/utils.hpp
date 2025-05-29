#pragma once

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera.hpp"

namespace Ember {
	void LoadGLFW();
	void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	void processInput(GLFWwindow* window, Camera& camera, float deltaTime);
}



