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
        cudaError_t err = cudaGetLastError();
        if (err != cudaSuccess) {
            printf("CUDA Error: %s\n", cudaGetErrorString(err));
        }
        // Make sure we call CUDA Device Reset before exiting
        cudaDeviceReset();
        exit(99);
    }
}

__global__ void render_init(glm::u32vec2 imgSize, curandState* rand_state) {
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    int j = threadIdx.y + blockIdx.y * blockDim.y;

    if ((i >= imgSize.x) || (j >= imgSize.y))
        return;
    int pixel_index = j * imgSize.x + i;
    curand_init(1984 + pixel_index, 0, 0, &rand_state[pixel_index]);
}

__global__ void render(dataPixels* data, glm::u32vec2 imgSize, Camera** cam, Hittable** world, curandState* rand_state)
{
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    int j = threadIdx.y + blockIdx.y * blockDim.y;

    if ((i >= imgSize.x) || (j >= imgSize.y))
        return;
    
    int pixelIndex = i + j * imgSize.x;
    curandState localRandState = rand_state[pixelIndex];
    glm::vec3 pixelColor(0.0f, 0.0f, 0.0f);
    for (int sampleIdx = 0; sampleIdx < (*cam)->getPerPixelSamples(); sampleIdx++){
        Ray r = (*cam)->getRay(i, j, &localRandState);
        pixelColor += (*cam)->rayColor(r, (*cam)->getMaxRecursionDepth(), world, &localRandState);
    }
    rand_state[pixelIndex] = localRandState;
    data[pixelIndex] = (*cam)->convertColor((*cam)->getPixelSampleScale() * pixelColor);
}

__global__ void rand_init(curandState* rand_state) {
    if (threadIdx.x == 0 && blockIdx.x == 0) {
        curand_init(1984, 0, 0, rand_state);
    }
}

#define RND (curand_uniform(&local_rand_state))

__global__ void initCamera(Camera** camera, glm::u32vec2 imgSize)
{
    *camera = new Camera(glm::vec3(13, 2, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), 20, 10.0, 0.6, 16.0f / 9.0f, imgSize.x, 100, 50);
}

__global__ void initWorld(Hittable** worldObjects, Hittable** listObjects, curandState* rand_state)
{
    if (threadIdx.x != 0 || blockIdx.x != 0)
        return;

    int id = 0;
    curandState local_rand_state = *rand_state;
    listObjects[id++] = new Sphere(glm::vec3(0, -1000.0f, -1), 1000.0f, new Materials::Lambertian(glm::vec3(0.5f, 0.5f, 0.5f)));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            double choose_mat = Utils::generateRandomNumber(&local_rand_state);
            glm::vec3 center(a + 0.9 * Utils::generateRandomNumber(&local_rand_state), 0.2, b + 0.9 * Utils::generateRandomNumber(&local_rand_state));

            if ((center - glm::vec3(4, 0.2, 0)).length() > 0.9) {
                Material* sphere_material;

                if (choose_mat < 0.8) {
                    glm::vec3 albedo = Utils::Vector::randomVector(0.0, 1.0, &local_rand_state) * Utils::Vector::randomVector(0.0, 1.0, &local_rand_state);
                    sphere_material = new Materials::Lambertian(albedo);
                }
                else if (choose_mat < 0.95) {
                    glm::vec3 albedo = Utils::Vector::randomVector(0.5, 1.0, &local_rand_state);
                    double fuzz = Utils::generateRandomNumber(0, 0.5, &local_rand_state);
                    sphere_material = new Materials::Metal(albedo, fuzz);
                }
                else
                    sphere_material = new Materials::Dielectric(1.5);

                listObjects[id++] = new Sphere(center, 0.2, sphere_material);
            }
        }
    }
    listObjects[id++] = new Sphere(glm::vec3(0, 1, 0), 1.0, new Materials::Dielectric(1.5f));
    listObjects[id++] = new Sphere(glm::vec3(-4, 1, 0), 1.0, new Materials::Lambertian(glm::vec3(0.4f, 0.2f, 0.1f)));
    listObjects[id++] = new Sphere(glm::vec3(4, 1, 0), 1.0, new Materials::Metal(glm::vec3(0.7f, 0.6f, 0.5f), 0.0f));
    *rand_state = local_rand_state;
    *worldObjects = new HittableList(listObjects, 22 * 22 + 1 + 3);
}

__global__ void freeWorld(Hittable** worldObjects, Camera** camera) {
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

    glm::u32vec2 imgTmp(1920, 1920);
    Camera** cam;
    checkCudaErrors(cudaMalloc((void**)&cam, sizeof(Camera*)));
    initCamera<<<1, 1 >>>(cam, imgTmp);
    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());

    glm::u32vec2 imgSize = glm::u32vec2(imgTmp.x, imgTmp.x / (16.0f / 9.0f)); //imGuiCam.getImageSize();

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
    //To tutaj bo nie chce mi sie liczyc ile to bd :)
    int hittableCount = 1;
    for (int a = -11; a < 11; a++)
        for (int b = -11; b < 11; b++)
            hittableCount++;

    Hittable** hittableList;
    checkCudaErrors(cudaMalloc((void**)&hittableList, hittableCount * sizeof(Hittable*)));
    Hittable** world;
    checkCudaErrors(cudaMalloc((void**)&world, sizeof(Hittable*)));

    //Allocate randState
    uint32_t numOfChannels = 4; //RGBA
    uint32_t pixelsSize = imgSize.x * imgSize.y * numOfChannels;

    curandState* curRandState; //For pixels
    checkCudaErrors(cudaMalloc((void**)&curRandState, pixelsSize * sizeof(curandState)));
    initWorld<<<1, 1>>>(world, hittableList, curRandState1);
    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());

    dataPixels* pixels;
    checkCudaErrors(cudaMallocManaged((void**)&pixels, pixelsSize * sizeof(dataPixels)));

    clock_t start, stop;
    start = clock();

    // Render our buffer
    int threadsX = 8, threadsY = 8;
    dim3 blocks(imgSize.x / threadsX + 1, imgSize.y / threadsY + 1);
    dim3 threads(threadsX, threadsY);
    render_init<<<blocks, threads>>>(imgSize, curRandState);
    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());
    render<<<blocks, threads>>>(pixels, imgSize, cam, world, curRandState);
    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());

    stop = clock();
    double timer_seconds = ((double)(stop - start)) / CLOCKS_PER_SEC;
    std::cerr << "took " << timer_seconds << " seconds.\n";

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