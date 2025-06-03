#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>

#include "shader.hpp"

#define MAX_BONES 4 // Maximum number of bones that can influence a vertex

namespace Ember {

    struct Vertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
        glm::vec3 Tangent;
        glm::vec3 Bitangent;
        int m_BindIDs[MAX_BONES];   // Index of the bone that influences this vertex
        float m_Weights[MAX_BONES]; // Weight of the influence of the bone on this vertex
    };

    struct Texture {
        unsigned int id;
        std::string type; // "texture_diffuse", "texture_specular", etc.
        std::string path; // Path to the texture file
    };

    class Mesh {
    public:
        // mesh Data
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;
        unsigned int VAO;

        // Constructor
        Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);

        // render the mesh
        void Draw(Shader& shader);

    private:
        unsigned int VBO, EBO;
        // Initialize the mesh by setting up buffers and vertex attributes
        void setupMesh();

    };

} // namespace Ember