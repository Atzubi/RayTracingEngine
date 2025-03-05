#include "RayTraceEngine/RayEngine.h"
#include "Shaders/PerspectiveGeneratorShader.h"
#include "Shaders/PhongShader.h"
#include "Utils/Camera.h"
#include "Utils/SFMLUtils.h"
#include "Utils/TriangleMeshLoader.h"

// Include a graphics library for displaying the render result
#include "SFML/Graphics.hpp"

#include <chrono>
#include <iostream>
#include <math.h> // for gcc

int main()
{
    RayEngine rayEngine = RayEngine();

    // ============================================= Define The Geometry ==============================================

    const auto handles = LoadTriangleMeshFromObj(std::filesystem::path("./Data/Duck/duck.obj"), rayEngine);

    SceneDescription                             sceneDesc{};
    std::vector<std::unique_ptr<InstanceHandle>> instances{};
    for (const auto& meshHandles : handles)
    {
        instances.push_back(rayEngine.CreateInstance(
            {meshHandles.intersectable.get(), {2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1}, {}}));
        sceneDesc.instances.push_back(instances.back().get());
        instances.push_back(rayEngine.CreateInstance(
            {meshHandles.intersectable.get(), {2, 0, 0, 2, 0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1}, {}}));
        sceneDesc.instances.push_back(instances.back().get());
        instances.push_back(rayEngine.CreateInstance(
            {meshHandles.intersectable.get(), {2, 0, 0, 4, 0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1}, {}}));
        sceneDesc.instances.push_back(instances.back().get());
        instances.push_back(rayEngine.CreateInstance(
            {meshHandles.intersectable.get(), {2, 0, 0, 6, 0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1}, {}}));
        sceneDesc.instances.push_back(instances.back().get());
    }
    const auto scene = rayEngine.CreateScene(sceneDesc);

    // ============================================= Define The Rendering =============================================
    const std::uint32_t resX{1024};
    const std::uint32_t resY{1024};
    const Vector3D      cameraPosition{3, 0.5, 8};
    const Vector3D      cameraDirection{0, 0, -1};
    const Vector3D      cameraUp{0, 1, 0};
    const std::uint8_t  samplesPerPixel{1};
    const float         speed{2.f};
    const float         sensitivity{0.2f};

    // Let's use a phong hit shader
    const auto hitShader = rayEngine.CreateShader({BasicPhongHitShader});

    // We will use a basic ray generator that shoots a ray per pixel from the camera into the scene
    const auto generatorShader = rayEngine.CreateShader({PerspectiveGeneratorShader});

    // We will need the camera and viewport and the sample count information in the shader so let's create resources for
    // them
    ViewportInfo viewportInfo{};
    viewportInfo.viewPortWidth  = resX;
    viewportInfo.viewPortHeight = resY;

    CameraInfo cameraInfo{};
    cameraInfo.cameraPosition  = cameraPosition;
    cameraInfo.cameraDirection = cameraDirection;
    cameraInfo.cameraUp        = cameraUp;

    SampleCountInfo sampleCountInfo{};
    sampleCountInfo.samplesPerPixel = samplesPerPixel;

    const auto viewPortShaderResource = rayEngine.CreateShaderResource({&viewportInfo});
    const auto cameraShaderResource   = rayEngine.CreateShaderResource({&cameraInfo});
    const auto sampleShaderResource   = rayEngine.CreateShaderResource({&sampleCountInfo});

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
    pipelineDescription.hitShader = {hitShader.get(), {cameraShaderResource.get(), handles[0].material.get()}};

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

        // Move objects in sine curve
        for (std::size_t i = 0; i < instances.size(); ++i)
        {
            const auto currentOffsetY = instances[i]->GetTransform().elements[1][3];
            const auto offsetY =
                sinf(i + std::chrono::high_resolution_clock::now().time_since_epoch().count() * 0.000000001f) -
                currentOffsetY;
            instances[i]->Transform(Matrix4x4{1, 0, 0, 0, 0, 1, 0, offsetY, 0, 0, 1, 0, 0, 0, 0, 1});
        }

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
