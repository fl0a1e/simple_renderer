// Local Headers
#include "glitter.hpp"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Standard Headers
#include <cstdio>
#include <cstdlib>

// opengl state
struct OpenGLState {
    bool PolygonMode = false;
};


void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void Initimgui(GLFWwindow* window);
void imguiTerminate();
void imguiShowUI();
void setOpenglState(const OpenGLState& s);
void RenderQuad();
void RenderCube();


Camera camera(glm::vec3(0.0f, 2.5f, 10.0f));

float lastX = mWidth / 2.0f;
float lastY = mHeight / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

OpenGLState openglstate;

int main(int argc, char* argv[]) {
    // Load GLFW and Create a Window
    auto mWindow = Ember::LoadGLFW(mWidth, mHeight, "OpenGL");

    // Create Context and Load OpenGL Functions
    glfwSetCursorPosCallback(mWindow, mouse_callback);
    glfwSetScrollCallback(mWindow, scroll_callback);

    // imgui
    Initimgui(mWindow);

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    // stbi_set_flip_vertically_on_load(true);

    // shader
    //Ember::Shader shader;
    //Ember::Shader showDepthShader;
    Ember::Shader shaderGeometryPass;
    Ember::Shader shaderLightingPass;
    Ember::Shader shaderLightBox;
    Ember::Shader debugshader;
    //shader.attach("vs.vert"); shader.attach("ps.frag"); shader.link();
    //showDepthShader.attach("vs.vert"); showDepthShader.attach("depth.frag"); showDepthShader.link();
    shaderGeometryPass.attach("g_buffer.vert"); shaderGeometryPass.attach("g_buffer.frag"); shaderGeometryPass.link();
    shaderLightingPass.attach("deferred_shading.vert"); shaderLightingPass.attach("deferred_shading.frag"); shaderLightingPass.link();
    shaderLightBox.attach("light_cube.vert"); shaderLightBox.attach("light_cube.frag"); shaderLightBox.link();
    debugshader.attach("debug_pos.vert"); debugshader.attach("debug_pos.frag"); debugshader.link();

    // Set samplers
    // deferred_shading需要采样纹理
    shaderLightingPass.activate();
    shaderLightingPass.bind("gPosition", 0);
    shaderLightingPass.bind("gNormal", 1);
    shaderLightingPass.bind("gAlbedoSpec", 2);
    debugshader.activate();
    debugshader.bind("gPosition", 0);

    // Model
    std::vector<Ember::Model> modelList;
    // Ember::Model backpack(PROJECT_SOURCE_DIR "/Glitter/Assets/Models/backpack/backpack.obj");
    //Ember::Model tree1(PROJECT_SOURCE_DIR "/Glitter/Assets/Models/tree/Tree1_1.obj");
    //Ember::Model tree2(PROJECT_SOURCE_DIR "/Glitter/Assets/Models/tree/Tree2_1.obj");
    Ember::Model zzzshark(PROJECT_SOURCE_DIR "/Glitter/Assets/Models/zzzshark/zzzshark.obj");
    //Ember::Model starwar(PROJECT_SOURCE_DIR "/Glitter/Assets/Models/starwar/77826.obj");
    modelList.push_back(zzzshark); //modelList.push_back(starwar);
    //modelList.push_back(tree2);
    // modelList.push_back(backpack);

    std::vector<glm::vec3> objectPositions;
    objectPositions.push_back(glm::vec3(-3.0, -3.0, -3.0));
    objectPositions.push_back(glm::vec3(0.0, -3.0, -3.0));
    objectPositions.push_back(glm::vec3(3.0, -3.0, -3.0));
    objectPositions.push_back(glm::vec3(-3.0, -3.0, 0.0));
    objectPositions.push_back(glm::vec3(0.0, -3.0, 0.0));
    objectPositions.push_back(glm::vec3(3.0, -3.0, 0.0));
    objectPositions.push_back(glm::vec3(-3.0, -3.0, 3.0));
    objectPositions.push_back(glm::vec3(0.0, -3.0, 3.0));
    objectPositions.push_back(glm::vec3(3.0, -3.0, 3.0));

    // lights
    // 定义与着色器匹配的对齐结构体
    struct Light {
        glm::vec3 Position;
        float padding1;  // 对应着色器中的padding1
        glm::vec3 Color;
        float Linear;
        float Quadratic;
        glm::vec2 padding2; // 对应着色器中的padding2
    };

    // 创建UBO并绑定到绑定点0
    const GLuint NR_LIGHTS = 800;
    GLuint lightUBO;
    glGenBuffers(1, &lightUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, lightUBO);
    glBufferData(GL_UNIFORM_BUFFER, NR_LIGHTS * sizeof(Light), NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, lightUBO); // 绑定到0号点
    // 更新光源数据
    std::vector<Light> lights(NR_LIGHTS);
    std::vector<glm::vec3> lightPositions;
    std::vector<glm::vec3> lightColors;
    srand(20);
    for (GLuint i = 0; i < NR_LIGHTS; i++) {
        // 随机扰动，-3.0-3.0
        GLfloat xPos = ((rand() % 100) / 100.f) * 6.0 - 3.0;  // 大坑啊这个整数除法
        GLfloat yPos = ((rand() % 100) / 100.f) * 6.0 - 3.0;
        GLfloat zPos = ((rand() % 100) / 100.f) * 6.0 - 3.0;
        lights[i].Position = glm::vec3(xPos, yPos, zPos);
        // 随机颜色, 0.5-1.0
        GLfloat rColor = ((rand() % 100) / 200.f) + 0.5;
        GLfloat gColor = ((rand() % 100) / 200.f) + 0.5;
        GLfloat bColor = ((rand() % 100) / 200.f) + 0.5;
        lights[i].Color = glm::vec3(rColor, gColor, bColor);
    }
    // 初始化光源数据
    glBindBuffer(GL_UNIFORM_BUFFER, lightUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, NR_LIGHTS * sizeof(Light), lights.data());



    // GBuffer
    // 3 textures
    // 1. Positions(RGB) 2. Normals(RGB) 3. Color(RGB) + Specular(A)
    GLuint gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    GLuint gPosition, gNormal, gAlbedoSpec; // 3 textures
    // Position(RGB) buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, mWidth, mHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    // Normal(RGB) buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, mWidth, mHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    // Color(RGB) + Specular(A) buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);
    // create and attach depth buffer (renderbuffer)
    GLuint rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, mWidth, mHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Game Loop
    while (glfwWindowShouldClose(mWindow) == false) {
        // time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Check & call events
        Ember::processInput(mWindow, camera, deltaTime);

        // configure global opengl state
        glEnable(GL_DEPTH_TEST);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        //ImGui::ShowDemoWindow(); // Show demo window! :)
        imguiShowUI();

        setOpenglState(openglstate);

        // ==== render ====
        // 1. Geometry Pass: render scene's geometry/color data into gbuffer
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear buffer
        shaderGeometryPass.activate();
        shaderGeometryPass.bind("projection", camera.GetPerspectiveMatrix(mWidth, mHeight));
        shaderGeometryPass.bind("view", camera.GetViewMatrix());
        for (int i = 0; i < objectPositions.size(); ++i) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, objectPositions[i]);
            model = glm::scale(model, glm::vec3(.025f));
            shaderGeometryPass.bind("model", model);
            modelList[0].Draw(shaderGeometryPass);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. Lighting Pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content.
        // render to screen quad
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderLightingPass.activate();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
        
        // send light relevant uniforms
        for (GLuint i = 0; i < lightPositions.size(); i++) {
            shaderLightingPass.bind("lights[" + std::to_string(i) + "].Position", lights[i].Position);
            shaderLightingPass.bind("lights[" + std::to_string(i) + "].Color", lights[i].Color);
            // Update attenuation parameters and calculate radius
            const GLfloat constant = 1.0; // Note that we don't send this to the shader, we assume it is always 1.0 (in our case)
            const GLfloat linear = 0.7;
            const GLfloat quadratic = 1.8;
            shaderLightingPass.bind("lights[" + std::to_string(i) + "].Linear", linear);
            shaderLightingPass.bind("lights[" + std::to_string(i) + "].Quadratic", quadratic);
        }
        shaderLightingPass.bind("viewPos", camera.Position);
		// 渲染到屏幕四边形
        RenderQuad();

		// 一样的道理，如果要单独显示某个gbuffer的纹理，可以用下面的代码
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            shaderLightingPass.activate();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gPosition);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, gNormal);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);

            // send light relevant uniforms
            for (GLuint i = 0; i < lightPositions.size(); i++) {
                shaderLightingPass.bind("lights[" + std::to_string(i) + "].Position", lights[i].Position);
                shaderLightingPass.bind("lights[" + std::to_string(i) + "].Color", lights[i].Color);
                // Update attenuation parameters and calculate radius
                const GLfloat constant = 1.0; // Note that we don't send this to the shader, we assume it is always 1.0 (in our case)
                const GLfloat linear = 0.7;
                const GLfloat quadratic = 1.8;
                shaderLightingPass.bind("lights[" + std::to_string(i) + "].Linear", linear);
                shaderLightingPass.bind("lights[" + std::to_string(i) + "].Quadratic", quadratic);
            }
            shaderLightingPass.bind("viewPos", camera.Position);
            // 渲染到四边形
            RenderQuad();
        }

        // === 前向渲染pass，正向渲染所有光立方体 ===
		
        // 小心处理depth, 否则会被覆盖
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
		// blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
        glBlitFramebuffer(0, 0, mWidth, mHeight, 0, 0, mWidth, mHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        // 标准前向流程
        shaderLightBox.activate();
        shaderLightBox.bind("projection", camera.GetPerspectiveMatrix(mWidth, mHeight));
        shaderLightBox.bind("view", camera.GetViewMatrix());
        for (GLuint i = 0; i < NR_LIGHTS; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, lights[i].Position);
            model = glm::scale(model, glm::vec3(.25f));
            shaderLightBox.bind("model", model);
            shaderLightBox.bind("lightColor", lights[i].Color);
            RenderCube();
        }

        // debug
        glDisable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        debugshader.activate();
		glViewport(0, 0, mWidth / 4, mHeight / 4);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        RenderQuad();
        glViewport(0, mHeight / 4, mWidth / 4, mHeight / 4);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        RenderQuad();
        glViewport(0, mHeight / 2, mWidth / 4, mHeight / 4);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
        RenderQuad();

        glViewport(0, 0, mWidth, mHeight);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }
    imguiTerminate();
    glfwTerminate();
    return EXIT_SUCCESS;
  }

// RenderQuad() Renders a 1x1 quad in NDC, best used for framebuffer color targets
// and post-processing effects.
GLuint quadVAO = 0;
GLuint quadVBO;
void RenderQuad()
{
    if (quadVAO == 0)
    {
        GLfloat quadVertices[] = {
            // Positions        // Texture Coords
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // Setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

// RenderCube() Renders a 1x1 3D cube in NDC.
GLuint cubeVAO = 0;
GLuint cubeVBO = 0;
void RenderCube()
{
    // Initialize (if necessary)
    if (cubeVAO == 0)
    {
        GLfloat vertices[] = {
            // Back face
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
            0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
            0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
            0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,  // top-right
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  // bottom-left
            -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,// top-left
            // Front face
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
            0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
            0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,  // top-right
            0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
            -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // top-left
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom-left
            // Left face
            -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
            -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-left
            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
            -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
            // Right face
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
            0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right         
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-right
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // top-left
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left     
            // Bottom face
            -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
            0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,// bottom-left
            0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
            -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
            -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            // Top face
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,// top-left
            0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
            0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right     
            0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,// top-left
            -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // Fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // Link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // Render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}


void Initimgui(GLFWwindow* window) {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
}

void imguiTerminate() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void imguiShowUI() {
	ImGui::Begin("Scene Info");
    ImGui::SetWindowFontScale(1.5f);
    ImGui::Text("frame(ms) - %.4f", deltaTime*1000.0f);
    ImGui::Text("fps - %d", static_cast<int>(1.f / deltaTime));
    ImGui::Text("viewPos - (%.2f, %.2f, %.2f)", static_cast<float>(camera.Position.x), static_cast<float>(camera.Position.y), static_cast<float>(camera.Position.z));
    ImGui::Checkbox("glPolygonMode", &(openglstate.PolygonMode));
    ImGui::End();
}

void setOpenglState(const OpenGLState& s) {
    if (s.PolygonMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

