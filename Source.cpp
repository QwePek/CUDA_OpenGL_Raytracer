#include "pch.h"
#include "GL/glew.h"
#include <GLFW/glfw3.h>

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "src/Rendering/Renderer.h"
#include "src/Rendering/VertexBuffer.h"
#include "src/Rendering/VertexBufferLayout.h"
#include "src/Rendering/IndexBuffer.h"
#include "src/Rendering/Texture.h"
#include "src/Utils/Utils.h"
#include "src/Raytracing/HittableList.h"
#include "src/Raytracing/Objects/Sphere.h"
#include "src/Camera.h"
#include "src/Raytracing/Materials/Lambertian.h"
#include "src/Raytracing/Materials/Metal.h"

float deltaTime = 0.0f;
float lastFrame = 0.0f;

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    if (!glfwInit())
    {
        std::cout << "Failed to initialize glfwInit()" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //Essential window calculations
    Camera cam(16.0f / 9.0f, 1920, 20, 10);
    glm::u32vec2 imgSize = cam.getImageSize();
    GLFWwindow* window = glfwCreateWindow(imgSize.x, imgSize.y, "Raytracing", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;

        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
        std::cout << "Error initializing GLEW" << std::endl;

    glViewport(0, 0, imgSize.x, imgSize.y);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glEnable(GL_DEPTH_TEST);

    //imgui
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();
    ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextAlign, ImVec2(0.5f, 0.5f));


    //Creating raytracing image plane
    Shader sh("src/Rendering/Shaders/shader.shader");

    //RAYTRACING CODE
    //World objects
    HittableList worldObjects;

    std::shared_ptr<Material> groundMat = std::make_shared<Materials::Lambertian>(glm::vec3(0.8f, 0.8f, 0.0f));
    std::shared_ptr<Material> centerMat = std::make_shared<Materials::Lambertian>(glm::vec3(0.1f, 0.2f, 0.5f));
    std::shared_ptr<Material> leftMat   = std::make_shared<Materials::Metal>(glm::vec3(0.8f, 0.8f, 0.8f), 0.3);
    std::shared_ptr<Material> rightMat  = std::make_shared<Materials::Metal>(glm::vec3(0.8f, 0.6f, 0.2f), 0.0);

    worldObjects.add(std::make_shared<Sphere>(glm::vec3(0.0f, -100.5f, -1.0f), 100.0f, groundMat));
    worldObjects.add(std::make_shared<Sphere>(glm::vec3(0.0f,    0.0f, -1.2f), 0.5f  , centerMat));
    worldObjects.add(std::make_shared<Sphere>(glm::vec3(-1.0f,   0.0f, -1.0f), 0.5f  , leftMat));
    worldObjects.add(std::make_shared<Sphere>(glm::vec3(1.0f,    0.0f, -1.0f), 0.5f  , rightMat));

    //Texture generation
    cam.render(worldObjects);
    std::vector<dataPixels> pixels = cam.getPixelData();

    Texture tx((unsigned char*)pixels.data(), imgSize.x, imgSize.y);


    //DRAWING utils and preparations
    glm::vec2 size(1.0f, 1.0f);
    std::vector<uint32_t> indices = { 0,1,2, 0,2,3 };
    std::vector<float> planeVert = {
        //Position                 //Texture coords
        +size.x, +size.y, 0.0f,    1.0f, 0.0f,
        +size.x, -size.y, 0.0f,    1.0f, 1.0f,
        -size.x, -size.y, 0.0f,    0.0f, 1.0f,
        -size.x, +size.y, 0.0f,    0.0f, 0.0f,
    };

    VertexBuffer vb(planeVert.data(), planeVert.size() * sizeof(float));
    VertexArray va;
    IndexBuffer ib(indices.data(), indices.size());
    VertexBufferLayout layout;
    layout.Push<float>(3);
    layout.Push<float>(2);

    va.addBuffer(vb, layout);
   
    ib.unbind();
    va.unbind();
    vb.unbind();

    Renderer renderer;
    sh.unbind();

    while (!glfwWindowShouldClose(window))
    {
        float currFrame = static_cast<float>(glfwGetTime());
        deltaTime = currFrame - lastFrame;
        lastFrame = currFrame;

        renderer.clear();

        //ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //Update
        //ImGui::ShowDemoWindow();

        processInput(window);
        sh.bind();
        sh.setUniform1i("texture1", 0);
        tx.bind(0);

        renderer.draw(va, ib, sh);

        sh.unbind();
        tx.unbind();

        //End Draw
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}