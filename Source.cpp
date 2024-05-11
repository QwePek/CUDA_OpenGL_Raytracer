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

float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct dataPixels {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

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

dataPixels convertColor(const glm::vec4& color) {
    int r = int(255.999f * color.r);
    int g = int(255.999f * color.g);
    int b = int(255.999f * color.b);
    int a = int(255.999f * color.a);

    dataPixels ret = { r,g,b,a };
    return ret;
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
    float aspectRatio = 16.0f / 9.0f;
    glm::vec2 windowSize = glm::vec2(800, 1);
    windowSize.y = windowSize.x / aspectRatio;
    windowSize.y = (windowSize.y < 1) ? 1 : windowSize.y;

    GLFWwindow* window = glfwCreateWindow(windowSize.x, windowSize.y, "App", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;

        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
        std::cout << "Error initializing GLEW" << std::endl;

    glViewport(0, 0, windowSize.x, windowSize.y);
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
    //Camera
    glm::vec3 cameraCenter(0.0f, 0.0f, 0.0f);
    float focal_length = 1.0f; //Distance between viewport and camera center point
    glm::vec2 viewport(1.0f, 2.0f);
    viewport.x = viewport.y * (double(windowSize.x) / windowSize.y);
    
    glm::vec3 viewport_u = glm::vec3(viewport.x, 0.0f, 0.0f);
    glm::vec3 viewport_v = glm::vec3(0.0f, -viewport.y, 0.0f);

    glm::vec3 pixelDelta_u = viewport_u / windowSize.x;
    glm::vec3 pixelDelta_v = viewport_v / windowSize.y;

    glm::vec3 viewportUpperLeft = cameraCenter - glm::vec3(0.0f, 0.0f, focal_length) -
        viewport_u / 2.0f - viewport_v / 2.0f;
    glm::vec3 pixel00_loc = viewportUpperLeft + 0.5f * (pixelDelta_u + pixelDelta_v);

    //World objects
    HittableList worldObjects;
    worldObjects.add(std::make_shared<Sphere>(glm::vec3(0.0f, 0.0f, -1.0f), 0.5f));
    worldObjects.add(std::make_shared<Sphere>(glm::vec3(0.0f, -100.5f, -1.0f), 100));


    //Texture generation
    uint32_t numOfChannels = 4; //RGBA
    uint32_t dataSize = windowSize.x * windowSize.y * numOfChannels;
    std::vector<dataPixels> data;
    data.reserve(data.size());
    for (int j = 0; j < windowSize.y; j++) {
        std::clog << "\rScanlines remaining: " << (windowSize.y - j) << ' ' << std::flush;
        for (int i = 0; i < windowSize.x; i++) {
            glm::vec3 pixelCenter = pixel00_loc + ((float)i * pixelDelta_u) + ((float)j * pixelDelta_v);
            glm::vec3 rayDir = pixelCenter - cameraCenter;

            Ray r(cameraCenter, rayDir);

            //glm::vec4 clr(double(i) / (windowSize.x - 1), double(j) / (windowSize.y - 1), 0.0f, 1
            data.emplace_back(convertColor(r.color(worldObjects)));
        }
    }

    std::clog << "\ndone\n";

    Texture tx((unsigned char*)data.data(), windowSize.x, windowSize.y);


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