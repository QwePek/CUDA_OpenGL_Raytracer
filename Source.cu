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

//CUDA
#include <curand_kernel.h>
#include <device_launch_parameters.h>

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// limited version of checkCudaErrors from helper_cuda.h in CUDA examples
#define checkCudaErrors(val) check_cuda( (val), #val, __FILE__, __LINE__ )

void check_cuda(cudaError_t result, char const* const func, const char* const file, int const line) {
    if (result) {
        std::cerr << "CUDA error = " << static_cast<unsigned int>(result) << " at " <<
            file << ":" << line << " '" << func << "' \n";
        // Make sure we call CUDA Device Reset before exiting
        cudaDeviceReset();
        exit(99);
    }
}

__global__ void render(dataPixels** data, glm::u32vec2 imgSize, Camera** cam, HittableList** world, curandState* rand_state)
{
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    int j = threadIdx.y + blockIdx.y * blockDim.y;

    if ((i >= imgSize.x) || (j >= imgSize.y))
        return;
    int pixelIndex = i + j * imgSize.x;
    curandState localRandState = rand_state[pixelIndex];
    glm::dvec3 pixelColor(0.0f, 0.0f, 0.0f);
    for (int sampleIdx = 0; sampleIdx < (*cam)->getPerPixelSamples(); sampleIdx++) {
        Ray r = (*cam)->getRay(i, j, rand_state);
        pixelColor += (*cam)->rayColor(r, (*cam)->getMaxRecursionDepth(), **world, rand_state);
    }
    rand_state[pixelIndex] = localRandState;
    *data[pixelIndex] = (*cam)->convertColor((*cam)->getPixelSampleScale() * pixelColor);
}

__global__ void rand_init(curandState* rand_state) {
    if (threadIdx.x == 0 && blockIdx.x == 0) {
        curand_init(1984, 0, 0, rand_state);
    }
}

#define RND (curand_uniform(&local_rand_state))

__global__ void initCamera(Camera** camera)
{
    *camera = new Camera(glm::dvec3(13, 2, 3), glm::dvec3(0, 0, 0), glm::dvec3(0, 1, 0), 20, 10.0, 0.6, 16.0f / 9.0f, 400, 20, 10);
}

__global__ void initWorld(HittableList** worldObjects, curandState* rand_state)
{
    if (threadIdx.x != 0 || blockIdx.x != 0)
        return;

    int hittableCount = 1;
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            hittableCount++;
        }
    }
    int id = 0;
    (*worldObjects)->objectsSize = hittableCount;
    (*worldObjects)->objects = new Hittable*[hittableCount];
    (*worldObjects)->objects[id++] = new Sphere(glm::dvec3(0, -1000, 0), 1000, new Materials::Lambertian(glm::dvec3(0.5, 0.5, 0.5)));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            double choose_mat = Utils::generateRandomNumber(rand_state);
            glm::dvec3 center(a + 0.9 * Utils::generateRandomNumber(rand_state), 0.2, b + 0.9 * Utils::generateRandomNumber(rand_state));

            if ((center - glm::dvec3(4, 0.2, 0)).length() > 0.9) {
                Material* sphere_material;

                if (choose_mat < 0.8) {
                    glm::dvec3 albedo = Utils::Vector::randomVector(0.0, 1.0, rand_state) * Utils::Vector::randomVector(0.0, 1.0, rand_state);
                    sphere_material = new Materials::Lambertian(albedo);
                }
                else if (choose_mat < 0.95) {
                    glm::dvec3 albedo = Utils::Vector::randomVector(0.5, 1.0, rand_state);
                    double fuzz = Utils::generateRandomNumber(0, 0.5, rand_state);
                    sphere_material = new Materials::Metal(albedo, fuzz);
                }
                else
                    sphere_material = new Materials::Dielectric(1.5);

                (*worldObjects)->objects[id++] = new Sphere(center, 0.2, sphere_material);
            }
        }
    }
    (*worldObjects)->objects[id++] = new Sphere(glm::dvec3(0, 1, 0), 1.0, new Materials::Dielectric(1.5));
    (*worldObjects)->objects[id++] = new Sphere(glm::dvec3(-4, 1, 0), 1.0, new Materials::Lambertian(glm::dvec3(0.4, 0.2, 0.1)));
    (*worldObjects)->objects[id++] = new Sphere(glm::dvec3(4, 1, 0), 1.0, new Materials::Metal(glm::dvec3(0.7, 0.6, 0.5), 0.0));
}

__global__ void freeWorld(HittableList** worldObjects, Camera** camera) {
    delete *worldObjects;
    delete *camera;
}

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
    curandState* curRandState1; //For world creation
    checkCudaErrors(cudaMalloc((void**)&curRandState1, 1 * sizeof(curandState)));
    rand_init<<<1, 1>>>(curRandState1);
    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());

    Camera** cam;
    checkCudaErrors(cudaMalloc((void**)&cam, sizeof(Camera*)));
    initCamera<<<1, 1 >>>(cam);
    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());

    //Data variables
    glm::u32vec2 imgSize = glm::dvec2(700, 800);//(*cam)->getImageSize();

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
    HittableList** world;
    //To tutaj bo nie chce mi sie liczyc ile to bd :)
    int hittableCount = 1;
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            hittableCount++;
        }
    }
    checkCudaErrors(cudaMalloc((void**)&world, sizeof(HittableList*)));

    //Allocate randState
    uint32_t numOfChannels = 4; //RGBA
    uint32_t pixelsSize = imgSize.x * imgSize.y * numOfChannels;

    curandState* curRandState; //For pixels
    checkCudaErrors(cudaMalloc((void**)&curRandState, pixelsSize * sizeof(curandState)));
    initWorld<<<1, 1>>>(world, curRandState1);
    checkCudaErrors(cudaGetLastError());
    //checkCudaErrors(cudaDeviceSynchronize());

    dataPixels** pixels;
    checkCudaErrors(cudaMallocManaged((void**)&pixels, pixelsSize * sizeof(dataPixels*)));
    render<<<1, 1>>>(pixels, imgSize, cam, world, curRandState);

    //std::vector<dataPixels> pixels = cam.getPixelData();


    //Texture generation
    Texture tx((unsigned char*)pixels, imgSize.x, imgSize.y);

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