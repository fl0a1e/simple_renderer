#pragma once

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <string>
#include <iostream>

#include "mesh.hpp"
#include "shader.hpp"

// ==== Assimp ====
// scene->mRootNode - is the root of a N-tree structure that contains all the meshes in the model
// scene->mMeshes[] - is a vector of all the meshes in the model
// scene->mMaterials[] - is a vector of all the materials in the model
// scene->mFlags - is a bitfield that contains flags about the scene


namespace Ember {

    class Model {
    public:
        // Model Data
        std::vector<Mesh> meshes;
        std::string directory; // Directory of the model file
        bool gammaCorrection; // Whether to apply gamma correction
        std::vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.

        // Constructor
        Model(const std::string& path, bool gamma = false);

        // Render the model
        void Draw(Shader& shader);

    private:
        // Load the model from file and store the meshes in the meshes vector
        void loadModel(const std::string& path);

        // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
        void processNode(aiNode* node, const aiScene* scene);

        // processes a mesh and converts it to our Mesh class
        Mesh processMesh(aiMesh* mesh, const aiScene* scene);

        // checks all material textures of a given type and loads the textures if they're not loaded yet.
        // the required info is returned as a Texture struct.
        std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

        unsigned int TextureFromFile(const char* path, const std::string& directory);
        
    };

} // namespace Ember