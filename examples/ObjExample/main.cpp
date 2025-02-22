// include relevant headers of the ray tracing engine
#include "Intersectables/TriangleMeshObject.h"
#include "RayTraceEngine/RayEngine.h"
#include "Shaders/HitShader.h"
#include "Shaders/RayGeneratorShader.h"
#include "Utils/Camera.h"
#include "Utils/SFMLUtils.h"
#include "Utils/TriangleMeshLoader.h"

#include "SFML/Graphics.hpp"

#include <chrono>
#include <cmath>
#include <iostream>

int main()
{
    RayEngine rayEngine = RayEngine();

    // ============================================= Define The Geometry ==============================================

    const auto meshHandles = LoadTriangleMeshFromObj(std::filesystem::path("./Data/Duck/duck.obj"), rayEngine);

    SceneDescription sceneDesc{};
    for (const auto& handle : meshHandles.intersectables)
    {
        sceneDesc.intersectables.push_back({*handle.get(), {2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1}, {}});
        sceneDesc.intersectables.push_back({*handle.get(), {2, 0, 0, 2, 0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1}, {}});
        sceneDesc.intersectables.push_back({*handle.get(), {2, 0, 0, 4, 0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1}, {}});
        sceneDesc.intersectables.push_back({*handle.get(), {2, 0, 0, 6, 0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1}, {}});
    }
    const auto scene     = rayEngine.CreateScene(sceneDesc);
    auto       instances = scene->GetInstanceHandles();

    // ============================================= Define The Rendering =============================================
    const std::uint32_t resX = 1000;
    const std::uint32_t resY = 1000;
    const Vector3D      cameraPosition{3, 0.5, 8};
    const Vector3D      cameraDirection{0, 0, -1};
    const Vector3D      cameraUp{0, 1, 0};
    const float         speed       = 2.f;
    const float         sensitivity = 0.2f;

    // Let's use a phong hit shader
    const BasicPhongHitShader phongHitShader;
    const auto                hitShader = rayEngine.CreateShader({&phongHitShader});

    // We will use a basic ray generator that shoots a ray per pixel from the camera into the scene
    const BasicRayGeneratorShader basicRayGeneratorShader;
    const auto                    generatorShader = rayEngine.CreateShader({&basicRayGeneratorShader});

    // We will need the camera and viewport information in the shader so let's create resources for them
    CameraInfo cameraInfo{};
    cameraInfo.cameraPosition  = cameraPosition;
    cameraInfo.cameraDirection = cameraDirection;
    cameraInfo.cameraUp        = cameraUp;

    const auto cameraShaderResource = rayEngine.CreateShaderResource({&cameraInfo});

    ViewportInfo viewportInfo{};
    viewportInfo.viewPortWidth  = resX;
    viewportInfo.viewPortHeight = resY;

    const auto viewPortShaderResource = rayEngine.CreateShaderResource({&viewportInfo});

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
    pipelineDescription.generatorShader = {generatorShader.get(),
                                           {viewPortShaderResource.get(), cameraShaderResource.get()}};
    pipelineDescription.hitShader       = {hitShader.get(), {cameraShaderResource.get()}};

    const auto pipeline = rayEngine.CreatePipeline(pipelineDescription);

    // ============================================= Create Render Target =============================================

    // Finally we need a target where our shading results should be written to
    const auto renderTarget = rayEngine.CreateRenderTarget({resX, resY, 3});

    sf::RenderWindow window;
    window.create(sf::VideoMode(resX, resY), "Render");

    // ================================================= Render Loop ==================================================

    while (window.isOpen())
    {
        const auto t1 = std::chrono::high_resolution_clock::now();
        pipeline->Run(*renderTarget);
        const auto t2 = std::chrono::high_resolution_clock::now();
        DisplayTexture(window, renderTarget->GetAsTexture());
        const auto t3 = std::chrono::high_resolution_clock::now();

        // Move objects in sine curve
        for (std::size_t i = 0; i < instances.size(); ++i)
        {
            volatile auto test           = instances[i]->GetTransform();
            const auto    currentOffsetY = instances[i]->GetTransform().elements[1][3];
            const auto    offsetY =
                std::sinf(i + std::chrono::high_resolution_clock::now().time_since_epoch().count() * 0.000000001f) -
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
        std::cout << "Render Time: " << (t2 - t1).count() / 1'000'000'000.f << std::endl;
        std::cout << "Display Time: " << (t3 - t2).count() / 1'000'000'000.f << std::endl;
        std::cout << "Frame Time: " << frameTime << std::endl;
        std::cout << "FPS: " << 1.f / frameTime << std::endl;
        std::cout << "=================================\n";
    }

    return 0;
}
