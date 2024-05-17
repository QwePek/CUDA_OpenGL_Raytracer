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
#include "src/Raytracing/Materials/Dielectric.h"

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
    Camera cam(glm::dvec3(13, 2, 3), glm::dvec3(0, 0, 0), glm::dvec3(0, 1, 0), 20, 10.0, 0.6
        , 16.0f / 9.0f, 1920, 500, 50);

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

    std::shared_ptr<Material> ground_material = std::make_shared<Materials::Lambertian>(glm::dvec3(0.5, 0.5, 0.5));
    worldObjects.add(std::make_shared<Sphere>(glm::dvec3(0, -1000, 0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            double choose_mat = Utils::generateRandomNumber();
            glm::dvec3 center(a + 0.9 * Utils::generateRandomNumber(), 0.2, b + 0.9 * Utils::generateRandomNumber());

            if ((center - glm::dvec3(4, 0.2, 0)).length() > 0.9) {
                std::shared_ptr<Material> sphere_material;
                
                if (choose_mat < 0.8) {
                    glm::dvec3 albedo = Utils::Vector::randomVector(0.0, 1.0) * Utils::Vector::randomVector(0.0, 1.0);
                    sphere_material = std::make_shared<Materials::Lambertian>(albedo);
                }
                else if (choose_mat < 0.95) {
                    glm::dvec3 albedo = Utils::Vector::randomVector(0.5, 1.0);
                    double fuzz = Utils::generateRandomNumber(0, 0.5);
                    sphere_material = std::make_shared<Materials::Metal>(albedo, fuzz);
                }
                else
                    sphere_material = std::make_shared<Materials::Dielectric>(1.5);
                
                worldObjects.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
            }
        }
    }

    std::shared_ptr<Material> material1 = std::make_shared<Materials::Dielectric>(1.5);
    std::shared_ptr<Material> material2 = std::make_shared<Materials::Lambertian>(glm::dvec3(0.4, 0.2, 0.1));
    std::shared_ptr<Material> material3 = std::make_shared<Materials::Metal>(glm::dvec3(0.7, 0.6, 0.5), 0.0);
    worldObjects.add(std::make_shared<Sphere>(glm::dvec3(0, 1, 0), 1.0, material1));
    worldObjects.add(std::make_shared<Sphere>(glm::dvec3(-4, 1, 0), 1.0, material2));
    worldObjects.add(std::make_shared<Sphere>(glm::dvec3(4, 1, 0), 1.0, material3));


    //std::shared_ptr<Material> groundMat  = std::make_shared<Materials::Lambertian>(glm::dvec3(0.3, 0.8, 0.0));
    //std::shared_ptr<Material> centerMat  = std::make_shared<Materials::Lambertian>(glm::dvec3(0.5, 0.2, 0.1));
    //std::shared_ptr<Material> leftMat    = std::make_shared<Materials::Diaelectric>(1.50);
    //std::shared_ptr<Material> leftBubble = std::make_shared<Materials::Diaelectric>(1.00 / 1.50);
    //std::shared_ptr<Material> rightMat   = std::make_shared<Materials::Metal>(glm::dvec3(0.8, 0.6, 0.2), 1.0);

    //worldObjects.add(std::make_shared<Sphere>(glm::dvec3(0.0, -100.5, -1.0), 100.0, groundMat));
    //worldObjects.add(std::make_shared<Sphere>(glm::dvec3(0.0,    0.0, -1.2), 0.5  , centerMat));
    //worldObjects.add(std::make_shared<Sphere>(glm::dvec3(-1.0,   0.0, -1.0), 0.5  , leftMat));
    //worldObjects.add(std::make_shared<Sphere>(glm::dvec3(-1.0,   0.0, -1.0), 0.4  , leftBubble));
    //worldObjects.add(std::make_shared<Sphere>(glm::dvec3(1.0,    0.0, -1.0), 0.5  , rightMat));

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