#include "RayTraceEngine/RayEngine.h"
#include "Shaders/CheckerBoxShader.h"
#include "Shaders/PathTraceShader.h"
#include "Shaders/PerspectiveGeneratorShader.h"
#include "Utils/Camera.h"
#include "Utils/SFMLUtils.h"
#include "Utils/TriangleMeshLoader.h"

// Include a graphics library for displaying the render result
#include "SFML/Graphics.hpp"

#include <chrono>
#include <cstring>
#include <iostream>

int main()
{
    RayEngine rayEngine = RayEngine();

    // ============================================= Define The Geometry ==============================================

    const auto floorMeshHandles = LoadTriangleMeshFromObj(std::filesystem::path("./Data/Floor/floor.obj"), rayEngine);
    const auto duckMeshHandles  = LoadTriangleMeshFromObj(std::filesystem::path("./Data/Duck/duck.obj"), rayEngine);
    const auto metalMeshHandles =
        LoadTriangleMeshFromObj(std::filesystem::path("./Data/Duck_Gold_Metal/duck.obj"), rayEngine);
    const auto glassMeshHandles =
        LoadTriangleMeshFromObj(std::filesystem::path("./Data/Duck_Gold_Glass/duck.obj"), rayEngine);

    SceneDescription                             sceneDesc{};
    std::vector<std::unique_ptr<InstanceHandle>> instances{};
    for (const auto& handles : floorMeshHandles)
    {
        instances.push_back(rayEngine.CreateInstance(
            {handles.intersectable.get(), {10, 0, 0, 0, 0, 10, 0, -1, 0, 0, 10, 0, 0, 0, 0, 1}, {}}));
        sceneDesc.instances.push_back(instances.back().get());
    }
    for (const auto& handles : duckMeshHandles)
    {
        instances.push_back(rayEngine.CreateInstance(
            {handles.intersectable.get(), {2, 0, 0, 2, 0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1}, {}}));
        sceneDesc.instances.push_back(instances.back().get());
    }
    for (const auto& handles : metalMeshHandles)
    {
        instances.push_back(rayEngine.CreateInstance(
            {handles.intersectable.get(), {2, 0, 0, -2, 0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1}, {}}));
        sceneDesc.instances.push_back(instances.back().get());
    }
    for (const auto& handles : glassMeshHandles)
    {
        instances.push_back(rayEngine.CreateInstance(
            {handles.intersectable.get(), {2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2, 2, 0, 0, 0, 1}, {}}));
        sceneDesc.instances.push_back(instances.back().get());
    }
    const auto scene = rayEngine.CreateScene(sceneDesc);

    // ============================================= Define The Rendering =============================================
    const std::uint32_t resX{1024};
    const std::uint32_t resY{1024};
    const Vector3D      cameraPosition{0.00001, 0.1, 6};
    Vector3D            cameraDirection{0, 0, -1};
    const Vector3D      cameraUp{0, 1, 0};
    const std::uint32_t samplesPerPixel{16};
    const float         speed{2.f};
    const float         sensitivity{0.2f};

    // Let's use a phong hit shader
    const auto hitShader = rayEngine.CreateShader({PathTraceShader});

    // We will use a basic ray generator that shoots a ray per pixel from the camera into the scene
    const auto generatorShader = rayEngine.CreateShader({PerspectiveGeneratorShader});

    // Let's use a skybox in form of a checkerboard box for an interesting background.
    const auto missShader = rayEngine.CreateShader({CheckerBoxShader});

    // We will need the camera and viewport and the sample count information in the shaders so let's create resources
    // for them
    ViewportInfo viewportInfo{};
    viewportInfo.viewPortWidth  = resX;
    viewportInfo.viewPortHeight = resY;

    CameraInfo cameraInfo{};
    cameraInfo.cameraPosition  = cameraPosition;
    cameraInfo.cameraDirection = cameraDirection;
    cameraInfo.cameraUp        = cameraUp;

    SampleCountInfo sampleCountInfo{};
    sampleCountInfo.samplesPerPixel = samplesPerPixel;

    // For path tracing we will track absorption and depth along the path
    PathData pathData{};
    pathData.absorption.resize(resX * resY * samplesPerPixel);
    pathData.depth.resize(resX * resY * samplesPerPixel);

    MaterialMap map{};
    for (std::size_t i = 0; i < instances.size(); ++i)
        map.instanceToMaterial[instances[i]->GetId()] = i;

    const auto viewPortShaderResource    = rayEngine.CreateShaderResource({&viewportInfo});
    const auto cameraShaderResource      = rayEngine.CreateShaderResource({&cameraInfo});
    const auto sampleShaderResource      = rayEngine.CreateShaderResource({&sampleCountInfo});
    const auto pathTracingShaderResource = rayEngine.CreateShaderResource({&pathData});
    const auto materialMapShaderResource = rayEngine.CreateShaderResource({&map});

    // For convenience let's use a camera so we can move within the scene (WASD + shift/space for movement, hold right
    // click for rotating the camera)
    auto cam = Camera(cameraShaderResource.get(), speed, sensitivity);

    // =========================================== Create Engine pipeline =============================================

    /**
     * A pipeline consists of a scene and a set of shaders following this execution model:
     *
     *                                      OcclusionShader  -|
     * RayGeneratorShader -> Ray Tracer ->  HitShader        -|  => Rendertarget
     *                           ^          PierceShader     -|
     *                           |          MissShader       -|
     *                           |                            |
     *                           ------------------------------
     */
    PipelineDescription pipelineDescription{};
    pipelineDescription.scene           = scene.get();
    pipelineDescription.generatorShader = {
        generatorShader.get(), {viewPortShaderResource.get(), cameraShaderResource.get(), sampleShaderResource.get()}};
    pipelineDescription.hitShader  = {hitShader.get(),
                                      {sampleShaderResource.get(),
                                       pathTracingShaderResource.get(),
                                       materialMapShaderResource.get(),
                                       floorMeshHandles[0].material.get(),
                                       duckMeshHandles[0].material.get(),
                                       metalMeshHandles[0].material.get(),
                                       glassMeshHandles[0].material.get()}};
    pipelineDescription.missShader = {missShader.get(), {sampleShaderResource.get(), pathTracingShaderResource.get()}};

    const auto pipeline = rayEngine.CreatePipeline(pipelineDescription);

    // ============================================= Create Render Target =============================================

    // Finally we need a target where our shading results should be written to
    const auto renderTarget = rayEngine.CreateRenderTarget({resX, resY});
    Texture    texture;

    sf::RenderWindow window;
    window.create(sf::VideoMode(resX, resY), "Render");

    // ================================================= Render Loop ==================================================

    while (window.isOpen())
    {
        const auto t1 = std::chrono::high_resolution_clock::now();
        pipeline->Run(*renderTarget);
        const auto t2 = std::chrono::high_resolution_clock::now();
        renderTarget->GetAsTexture(TextureFormat::RGBA, texture);
        DisplayTexture(window, texture);
        const auto t3 = std::chrono::high_resolution_clock::now();

        auto& resource = pathTracingShaderResource->Map<PathData>();
        std::memset(resource.absorption.data(), 0, resource.absorption.size() * sizeof(Vector3D));
        std::memset(resource.depth.data(), 0, resource.depth.size() * sizeof(std::uint32_t));

        cam.Update(window);

        sf::Event event{};
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
                break;
            }
            if (event.type == sf::Event::MouseButtonPressed && sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
            {
                cam.BeginDrag(window);
            }
            if (event.type == sf::Event::MouseButtonReleased)
            {
                cam.EndDrag();
            }
        }

        const auto t4        = std::chrono::high_resolution_clock::now();
        const auto frameTime = (t4 - t1).count() / 1'000'000'000.f;
        std::cout << "Render Time: " << (t2 - t1).count() / 1'000'000'000.f << "s\n";
        std::cout << "Display Time: " << (t3 - t2).count() / 1'000'000'000.f << "s\n";
        std::cout << "Frame Time: " << frameTime << "s\n";
        std::cout << "FPS: " << 1.f / frameTime << std::endl;
        std::cout << "=================================\n";
    }

    return 0;
}
