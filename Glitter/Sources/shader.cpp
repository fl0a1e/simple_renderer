// Local Headers
#include "shader.hpp"

// Standard Headers
#include <cassert>
#include <fstream>
#include <memory>

namespace Ember {
    Shader& Shader::activate()
    {
        glUseProgram(mProgram);
        return *this;
    }

    void Shader::bind(unsigned int location, int value) { glUniform1i(location, value); }
    void Shader::bind(unsigned int location, float value) { glUniform1f(location, value); }
    void Shader::bind(unsigned int location, glm::vec2 value) { glUniform2f(location, value.x, value.y); }
    void Shader::bind(unsigned int location, glm::vec3 value) { glUniform3f(location, value.x, value.y, value.z); }
    void Shader::bind(unsigned int location, glm::vec4 value) { glUniform4f(location, value.x, value.y, value.z, value.w); }
    void Shader::bind(unsigned int location, glm::mat4 const& matrix) { glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix)); }


    Shader& Shader::attach(std::string const& filename)
    {
        // Load GLSL Shader Source from File
        std::string path = PROJECT_SOURCE_DIR "/Glitter/Shaders/";
        std::ifstream fd(path + filename);
        fprintf(stderr, "shader path: %s\n", (path + filename).c_str());
        auto src = std::string(std::istreambuf_iterator<char>(fd),
            (std::istreambuf_iterator<char>()));

        // Create a Shader Object
        const char* source = src.c_str();
        auto shader = create(filename);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, &mStatus);

        // Display the Build Log on Error
        if (mStatus == false)
        {
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &mLength);
            std::unique_ptr<char[]> buffer(new char[mLength]);
            glGetShaderInfoLog(shader, mLength, nullptr, buffer.get());
            fprintf(stderr, "%s\n%s", filename.c_str(), buffer.get());
        }

        // Attach the Shader and Free Allocated Memory
        glAttachShader(mProgram, shader);
        glDeleteShader(shader);
        return *this;
    }

    GLuint Shader::create(std::string const& filename)
    {
        auto index = filename.rfind(".");
        auto ext = filename.substr(index + 1);
        if (ext == "comp") return glCreateShader(GL_COMPUTE_SHADER);
        else if (ext == "frag") return glCreateShader(GL_FRAGMENT_SHADER);
        else if (ext == "geom") return glCreateShader(GL_GEOMETRY_SHADER);
        else if (ext == "vert") return glCreateShader(GL_VERTEX_SHADER);
        else                    return false;
    }

    Shader& Shader::link()
    {
        glLinkProgram(mProgram);
        glGetProgramiv(mProgram, GL_LINK_STATUS, &mStatus);
        if (mStatus == false)
        {
            glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &mLength);
            std::unique_ptr<char[]> buffer(new char[mLength]);
            glGetProgramInfoLog(mProgram, mLength, nullptr, buffer.get());
            fprintf(stderr, "%s", buffer.get());
        }
        assert(mStatus == true);
        return *this;
    }
}
    

