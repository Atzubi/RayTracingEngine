#include "Intersectables/TriangleMeshObject.h"
#include "RayTraceEngine/RayEngine.h"
#include "Shaders/PerspectiveGeneratorShader.h"
#include "Utils/SFMLUtils.h"

// Include a graphics library for displaying the render result
#include "SFML/Graphics.hpp"

namespace
{
    ShaderOutput WhiteShader(const std::uint64_t                     id,
                             const HitShaderInput&                   shaderInput,
                             const std::span<IShaderResource* const> shaderResource,
                             RayGeneratorOutput&                     newRays)
    {
        return {1, 1, 1};
    }
} // namespace

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

    // Create a triangle mesh object
    TriangleMeshObject triangleMeshObject(std::move(vertices), std::move(indices));

    // Add the object to engine
    const auto object = rayEngine.CreateIntersectableObject({&triangleMeshObject});

    // Create an instance of the object
    const auto instance = rayEngine.CreateInstance({object.get(), Matrix4x4::GetIdentity()});

    // Finally we create a scene by placing an instance of our triangle into it
    SceneDescription sceneDesc{};
    sceneDesc.instances.push_back(instance.get());
    const auto scene = rayEngine.CreateScene(sceneDesc);

    // ============================================= Define The Rendering =============================================

    // We want to set up rendering such that we see the triangle on screen

    // Define the rendering resolution and the camera
    const std::uint32_t resX{1000};
    const std::uint32_t resY{1000};
    const Vector3D      cameraPosition{0.5, 0.5, -2};
    const Vector3D      cameraDirection{0, 0, 1};
    const Vector3D      cameraUp{0, 1, 0};
    const std::uint8_t  samplesPerPixel{1};

    // We will use a basic ray generator that shoots a ray per pixel from the camera into the scene
    const auto generatorShader = rayEngine.CreateShader({PerspectiveGeneratorShader});

    // The generator shader needs to know the resolution, samples per pixel and camera position so we create shader
    // resources for it
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

    // For shading we use a simple shader that colors a pixel white. Since it's a hit shader only pixels where a ray
    // intersected the triangle turn white
    const auto hitShader = rayEngine.CreateShader({WhiteShader});

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
    pipelineDescription.hitShader = {hitShader.get(), {}};

    const auto pipeline = rayEngine.CreatePipeline(pipelineDescription);

    // ============================================= Create Render Target =============================================

    // Finally we need a target where our shading results should be written to
    const auto renderTarget = rayEngine.CreateRenderTarget({resX, resY});

    // ============================================= Execute The Pipeline =============================================

    // All that is left is to run the engine
    pipeline->Run(*renderTarget);

    // ================================================ Display Result ================================================

    // We can extract the render result as a texture which can easily be displayed
    const auto texture = renderTarget->GetAsTexture(TextureFormat::RGBA);

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
