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


Camera camera(glm::vec3(0.0f, 5.0f, 3.0f));

float lastX = mWidth / 2.0f;
float lastY = mHeight / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

OpenGLState openglstate;

int main(int argc, char * argv[]) {
    // Load GLFW and Create a Window
    auto mWindow = Ember::LoadGLFW(mWidth, mHeight, "OpenGL");
    
    // Create Context and Load OpenGL Functions
    glfwSetCursorPosCallback(mWindow, mouse_callback);
    glfwSetScrollCallback(mWindow, scroll_callback);

    // imgui
    Initimgui(mWindow);
    
    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    //stbi_set_flip_vertically_on_load(true);
    
    // shader
    Ember::Shader shader;
    shader.attach("vs.vert"); shader.attach("ps.frag"); shader.link();
    // Model
    std::vector<Ember::Model> modelList;
    //Ember::Model ourModel(PROJECT_SOURCE_DIR "/Glitter/Assets/Models/backpack/backpack.obj");
    //Ember::Model tree1(PROJECT_SOURCE_DIR "/Glitter/Assets/Models/tree/Tree1_1.obj");
    //Ember::Model tree2(PROJECT_SOURCE_DIR "/Glitter/Assets/Models/tree/Tree2_1.obj");
    Ember::Model zzzshark(PROJECT_SOURCE_DIR "/Glitter/Assets/Models/zzzshark/zzzshark.obj");
    modelList.push_back(zzzshark);
    
    // configure global opengl state
    glEnable(GL_DEPTH_TEST);
    

    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) {
        // time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        Ember::processInput(mWindow, camera, deltaTime);
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        //ImGui::ShowDemoWindow(); // Show demo window! :)
        imguiShowUI();
        
        // ==== render ====
        // Background Fill Color
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear color & depth buffer
        setOpenglState(openglstate);

        shader.activate();
        //shader.bind("viewPos", camera.Position);
        // view/projection transformations
        shader.bind("projection", camera.GetPerspectiveMatrix(mWidth, mHeight));
        shader.bind("view", camera.GetViewMatrix());
        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(.02f, .02f, .02f));	// it's a bit too big for our scene, so scale it down
        shader.bind("model", model);
        // directional light
        //shader.bind("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
        //shader.bind("dirLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        //shader.bind("dirLight.diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
        //shader.bind("dirLight.specular", glm::vec3(0.5f, 0.5f, 0.5f));
        for (Ember::Model& m : modelList) {
            m.Draw(shader);
        }

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
    ImGui::Checkbox("glPolygonMode", &(openglstate.PolygonMode));
    ImGui::End();
}

void setOpenglState(const OpenGLState& s) {
    if (s.PolygonMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}