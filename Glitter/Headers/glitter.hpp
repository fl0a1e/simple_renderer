// Preprocessor Directives
#pragma once

// System Headers
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <btBulletDynamicsCommon.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Reference: https://github.com/nothings/stb/blob/master/stb_image.h#L4
// To use stb_image, add this in *one* C++ source file.
//#define STB_IMAGE_IMPLEMENTATION
//#define STBI_WINDOWS_UTF8
//#include <stb_image.h>

// Custom Headers
#include "shader.hpp"
#include "camera.hpp"
#include "utils.hpp"
#include "model.hpp"

// ToDo
// import spdlog to use as a logger

// Define Some Constants
const int mWidth = 1280;
const int mHeight = 800;
