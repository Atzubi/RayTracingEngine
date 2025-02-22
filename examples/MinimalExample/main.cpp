// include relevant headers of the ray tracing engine
#include "Intersectables/TriangleMeshObject.h"
#include "RayTraceEngine/RayEngine.h"
#include "Shaders/HitShader.h"
#include "Shaders/RayGeneratorShader.h"
#include "Utils/SFMLUtils.h"

// include a graphics library for displaying the render result
#include "SFML/Graphics.hpp"

class SimpleHitShader : public IHitShader
{
    std::unique_ptr<IHitShader> Clone() const override { return std::make_unique<SimpleHitShader>(*this); }

    ShaderOutput Shade(const std::uint64_t                  id,
                       const HitShaderInput&                shaderInput,
                       const std::vector<IShaderResource*>& shaderResource,
                       RayGeneratorOutput&                  newRays) const override
    {
        return {255, 255, 255};
    }
};

int main()
{
    // Create the ray tracing engine
    RayEngine rayEngine{};

    // ============================================= Define The Geometry ==============================================

    // Create a vertex buffer with 3 vertices to define a triangle (we only use the position in this example)
    TriangleMeshObject::Vertex              v1{.position = {0, 0, 0}};
    TriangleMeshObject::Vertex              v2{.position = {1, 0, 0}};
    TriangleMeshObject::Vertex              v3{.position = {0, 1, 0}};
    std::vector<TriangleMeshObject::Vertex> vertices{std::move(v1), std::move(v2), std::move(v3)};

    // Create a buffer for the indices, every index points to a vertex in the vertex vector, every 3 indices define 1
    // triangle
    std::vector<uint32_t> indices{0, 1, 2};

    // We can attach a material to our geometry, we will leave this empty for the minimal example
    Material material{};

    // Create a triangle mesh object
    TriangleMeshObject triangleMeshObject(std::move(vertices), std::move(indices), std::move(material));

    // Add the object to engine
    const auto object = rayEngine.CreateIntersectableObject({triangleMeshObject});

    // Finally we create a scene by placing an instance of our triangle into it
    SceneDescription sceneDesc{};
    sceneDesc.intersectables.emplace_back(*object, Matrix4x4::GetIdentity());
    const auto scene = rayEngine.CreateScene(sceneDesc);

    // ============================================= Define The Rendering =============================================

    // We want to set up rendering such that we see the triangle on screen

    // Define the rendering resolution and the camera
    const std::uint32_t resX = 1000;
    const std::uint32_t resY = 1000;
    const Vector3D      cameraPosition{0.5, 0.5, -2};
    const Vector3D      cameraDirection{0, 0, 1};
    const Vector3D      cameraUp{0, 1, 0};

    // We will use a basic ray generator that shoots a ray per pixel from the camera into the scene
    const BasicRayGeneratorShader basicRayGeneratorShader;
    const auto                    generatorShader = rayEngine.CreateShader({&basicRayGeneratorShader});

    // The generator shader needs to know the resolution and camera position so we create a shader resource for it
    ViewportInfo viewportInfo{};
    viewportInfo.viewPortWidth  = resX;
    viewportInfo.viewPortHeight = resY;

    CameraInfo cameraInfo{};
    cameraInfo.cameraPosition  = cameraPosition;
    cameraInfo.cameraDirection = cameraDirection;
    cameraInfo.cameraUp        = cameraUp;

    const auto viewPortShaderResource = rayEngine.CreateShaderResource({&viewportInfo});
    const auto cameraShaderResource   = rayEngine.CreateShaderResource({&cameraInfo});

    // For shading we use a simple shader that colors a pixel white. Since it's a hit shader only pixels where a ray
    // intersected the triangle turn white
    const SimpleHitShader simpleHitShader;
    const auto            hitShader = rayEngine.CreateShader({&simpleHitShader});

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
    pipelineDescription.hitShader       = {hitShader.get(), {}};

    const auto pipeline = rayEngine.CreatePipeline(pipelineDescription);

    // ============================================= Create Render Target =============================================

    // Finally we need a target where our shading results should be written to
    const auto renderTarget = rayEngine.CreateRenderTarget({resX, resY, 3});

    // ============================================= Execute The Pipeline =============================================

    // All that is left is to run the engine
    pipeline->Run(*renderTarget);

    // ================================================ Display Result ================================================

    // We can extract the render result as a texture which can easily be displayed
    const auto texture = renderTarget->GetAsTexture();

    // Create window
    sf::RenderWindow window;
    window.create(sf::VideoMode(resX, resY), "Render");

    // Display the texture on the window
    DisplayTexture(window, texture);

    // Wait for the window to close
    while (window.isOpen())
    {
        sf::Event event{};
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
                break;
            }
        }
    }

    return 0;
}
