#include "model.hpp"


// Reference: https://github.com/nothings/stb/blob/master/stb_image.h#L4
// To use stb_image, add this in *one* C++ source file.
#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include <stb_image.h>

// ==== Assimp ====
// scene->mRootNode - is the root of a N-tree structure that contains all the meshes in the model
// scene->mMeshes[] - is a vector of all the meshes in the model
// scene->mMaterials[] - is a vector of all the materials in the model
// scene->mFlags - is a bitfield that contains flags about the scene


namespace Ember {
        
    Model::Model(const std::string& path, bool gamma) : gammaCorrection(gamma) {
        loadModel(path);
    }

        
    void Model::Draw(Shader& shader) {
        for (Mesh& mesh : meshes) {
            mesh.Draw(shader); // drawcall  = meshes.size()
        }
    }
        
    void Model::loadModel(const std::string& path) {
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // Check if the import was successful
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
            return;
        }
        // retrieve the directory of the model file
        directory = path.substr(0, path.find_last_of('/'));
        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

        
    void Model::processNode(aiNode* node, const aiScene* scene) {
        // process all the meshes located at this node
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

        
    Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
        // data to fill
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        // walk through each of the mesh's vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            glm::vec3 vector; // we declare a placeholder vector
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // normals
            if (mesh->HasNormals()) {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            else {
                vertex.Normal = glm::vec3(0.0f, 0.0f, 0.0f);
            }
            // texture coordinates
            if (mesh->mTextureCoords[0]) { // does the mesh contain texture coordinates?
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we only use the first set of texture coordinates.
                // use models where a vertex can have multiple texture coordinates with caution, as this is not a common use case.
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
                // tangent
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
                // bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
            else {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }
            vertices.push_back(vertex);
        }

        // walk through each of the mesh's faces (a face is a polygon, usually a triangle)
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        // 1. diffuse maps
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
		std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());


        // return a mesh object with vertices, indices, and textures
        return Mesh(vertices, indices, textures);
    }

    std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName) {
        std::vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if (!skip)
            {   // if texture hasn't been loaded already, load it
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
            }
        }
        return textures;

    }

    unsigned int TextureFromFile(const char* path, const std::string& directory)
    {
        std::string filename = std::string(path);
        filename = directory + '/' + filename;

        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
        }

        return textureID;
    }

} // namespace Ember