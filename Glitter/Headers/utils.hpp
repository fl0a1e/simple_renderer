#pragma once

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cstdlib>

#include "camera.hpp"

namespace Ember {
	GLFWwindow* LoadGLFW(const int mWidth, const int mHeight, const char* title);
	void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	void processInput(GLFWwindow* window, Camera& camera, float deltaTime);
}



