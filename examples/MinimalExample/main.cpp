// include relevant headers of the ray tracing engine
#include "../Shaders/HitShader.h"
#include "../Shaders/RayGeneratorShader.h"
#include "Intersectables/TriangleMeshObject.h"
#include "RayTraceEngine/RayEngine.h"

// include an obj loader for parsing obj files
#include "OBJ_Loader.h"

// include a graphics library for displaying the render result
#include "SFML/Graphics.hpp"

#define STB_IMAGE_IMPLEMENTATION

// include stb for texture format support
#include "Stb/stb_image.h"

int main()
{
    // create the obj parser
    objl::Loader loader;

    // create the ray tracing engine
    RayEngine rayEngine = RayEngine();

    // ======================================== Create Geometry Objects ===============================================

    std::vector<std::unique_ptr<IntersectableObjectHandle>> intersectables;
    std::vector<std::unique_ptr<RenderTargetHandle>>        renderTargets;

    // load the obj file
    if (!loader.LoadFile("./Data/Basketball/Basketball.obj"))
        return 1;

    // iterate over all meshes defined in the object loaded
    for (auto& m : loader.LoadedMeshes)
    {
        // create a vector for the meshes vertices
        std::vector<TriangleMeshObject::Vertex> vertices;

        // create a vector for the indices defined by the mesh, every index points to a vertex in vertex vector,
        // every 3 indices define 1 triangle
        std::vector<uint64_t> indices;

        // create the material container
        Material material;

        int comp;

        // fill the material description
        material.name        = m.MeshMaterial.name;
        material.Ka          = {m.MeshMaterial.Ka.X, m.MeshMaterial.Ka.Y, m.MeshMaterial.Ka.Z};
        material.Kd          = {m.MeshMaterial.Kd.X, m.MeshMaterial.Kd.Y, m.MeshMaterial.Kd.Z};
        material.Ks          = {m.MeshMaterial.Ks.X, m.MeshMaterial.Ks.Y, m.MeshMaterial.Ks.Z};
        material.Ns          = m.MeshMaterial.Ns;
        material.Ni          = m.MeshMaterial.Ni;
        material.d           = m.MeshMaterial.d;
        material.illum       = m.MeshMaterial.illum;
        material.map_Ka.name = m.MeshMaterial.map_Ka;
        material.map_Kd.name = m.MeshMaterial.map_Kd;
        // load the texture using stbi
        auto texture = stbi_load(("../Data/Basketball/" + material.map_Kd.name).c_str(),
                                 reinterpret_cast<int*>(&(material.map_Kd.w)),
                                 reinterpret_cast<int*>(&(material.map_Kd.h)),
                                 &comp,
                                 STBI_rgb);
        if (texture)
        {
            auto tex        = rayEngine.CreateRenderTarget({material.map_Kd.w, material.map_Kd.h});
            material.map_Kd = tex->GetAsTexture();
            for (int i = 0; i < comp * material.map_Kd.w * material.map_Kd.h; i++)
            {
                (*material.map_Kd.image)[i] = texture[i];
            }
            renderTargets.push_back(std::move(tex));
        }
        else
        {
            auto tex        = rayEngine.CreateRenderTarget({0, 0});
            material.map_Kd = tex->GetAsTexture();
            renderTargets.push_back(std::move(tex));
        }
        material.map_Ks.name   = m.MeshMaterial.map_Ks;
        material.map_Ns.name   = m.MeshMaterial.map_Ns;
        material.map_d.name    = m.MeshMaterial.map_d;
        material.map_bump.name = m.MeshMaterial.map_bump;

        // copy index list
        for (unsigned int index : m.Indices)
        {
            indices.push_back(index);
        }

        // copy vertex list
        for (auto& item : m.Vertices)
        {
            TriangleMeshObject::Vertex vertex = {{item.Position.X, item.Position.Y, item.Position.Z},
                                                 {item.Normal.X, item.Normal.Y, item.Normal.Z},
                                                 {item.TextureCoordinate.X, item.TextureCoordinate.Y}};
            vertices.push_back(vertex);
        }

        // create a triangle mesh object
        TriangleMeshObject triangleMeshObject(std::move(vertices), std::move(indices), std::move(material));

        // add the object to engine
        auto handle = rayEngine.CreateIntersectableObject({triangleMeshObject});

        // add the id to our referencing ids
        intersectables.push_back(std::move(handle));
    }

    // ================================================================================================================

    // ========================================= Add Shaders to the Engine ============================================
    std::uint32_t resX = 1000;
    std::uint32_t resY = 1000;
    Vector3D      cameraPosition{0, 3, -10};
    Vector3D      cameraDirection{0, 0, 1};
    Vector3D      cameraUp{0, 1, 0};

    // use basic hit shader prefab
    BasicHitShader basicHitShader;

    CameraInfo cameraInfo{};
    cameraInfo.cameraPosition = cameraPosition;

    auto hitShaderResource = rayEngine.CreateShaderResource({&cameraInfo});

    // add hit shader to the engine, id can be used to reference to the shader within the engine
    auto hitShader = rayEngine.CreateShader({&basicHitShader});

    // use basic ray generator shader prefab
    BasicRayGeneratorShader basicRayGeneratorShader;

    ViewportInfo viewportInfo{};
    viewportInfo.viewPortWidth   = resX;
    viewportInfo.viewPortHeight  = resY;
    viewportInfo.cameraPosition  = cameraPosition;
    viewportInfo.cameraDirection = cameraDirection;
    viewportInfo.cameraUp        = cameraUp;

    auto generatorShaderResource = rayEngine.CreateShaderResource({&viewportInfo});

    // add ray generator shader to the engine
    auto generatorShader = rayEngine.CreateShader({&basicRayGeneratorShader});

    // ================================================================================================================

    // =========================================== Create Engine pipeline =============================================

    // instanced objects have their own transformation, create one for each instance
    SceneDescription sceneDesc{};
    for (unsigned long i = 0; i < intersectables.size(); i++)
    {
        sceneDesc.intersectables.push_back({*intersectables[i].get(), {Matrix4x4::GetIdentity()}, {}});
    }
    auto scene = rayEngine.CreateScene(sceneDesc);

    // create a pipeline description
    PipelineDescription pipelineDescription{};
    pipelineDescription.scene           = scene.get();
    pipelineDescription.generatorShader = {generatorShader.get(), {generatorShaderResource.get()}};
    pipelineDescription.hitShader       = {hitShader.get(), {hitShaderResource.get()}};

    // add pipeline to the engine
    auto pipeline = rayEngine.CreatePipeline(pipelineDescription);

    // ================================================================================================================

    auto renderTarget = rayEngine.CreateRenderTarget({resX, resY});

    // run the pipeline
    pipeline->Run(*renderTarget.get());

    // get the result of the pipeline
    auto texture = renderTarget->GetAsTexture();

    // create image container for sfml
    int                    channelCount = 4;
    std::vector<sf::Uint8> pixels(resX * resY * channelCount);

    unsigned char fullOpacity = 255;

    // translate from texture to sfml
    for (int x = 0; x < resX; x++)
    {
        for (int y = 0; y < resY; y++)
        {
            pixels[(x + y * resX) * channelCount + 0] = (*texture.image)[(x + y * resX) * 3 + 0];
            pixels[(x + y * resX) * channelCount + 1] = (*texture.image)[(x + y * resX) * 3 + 1];
            pixels[(x + y * resX) * channelCount + 2] = (*texture.image)[(x + y * resX) * 3 + 2];
            pixels[(x + y * resX) * channelCount + 3] = fullOpacity;
        }
    }

    // create window
    sf::RenderWindow window;
    window.create(sf::VideoMode(resX, resY), "Render");

    // create image
    sf::Image image;
    image.create(resX, resY, pixels.data());

    // create texture from image
    sf::Texture tex;
    tex.loadFromImage(image);

    // create sprite from texture
    sf::Sprite sprite;
    sprite.setTexture(tex);

    // draw sprite on screen
    window.draw(sprite);
    window.display();

    // wait for window to close
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
