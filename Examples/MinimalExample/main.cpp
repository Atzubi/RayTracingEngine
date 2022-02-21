// include relevant headers of the ray tracing engine
#include "RayTraceEngine/RayTraceCore.h"
#include "RayTraceEngine/Intersectable.h"

// include an obj loader for parsing obj files
#include "OBJ_Loader.h"

// include a graphics library for displaying the render result
#include "SFML/Graphics.hpp"

#define STB_IMAGE_IMPLEMENTATION

// include stb for texture format support
#include "Stb/stb_image.h"

int main() {
    // create the obj parser
    objl::Loader loader;

    // create the ray tracing engine
    RayEngine rayEngine = RayEngine();

    // ======================================== Create Geometry Objects ===============================================

    // create a vector for object ids, they can be used to reference objects within the engine
    std::vector<ObjectId> objectIDs;

    // load the obj file
    if (!loader.LoadFile("./Data/Basketball/Basketball.obj")) return 1;

    // iterate over all meshes defined in the object loaded
    for (auto &m: loader.LoadedMeshes) {
        // create a vector for the meshes vertices
        std::vector<TriangleMeshObject::Vertex> vertices;

        // create a vector for the indices defined by the mesh, every index points to a vertex in vertex vector,
        // every 3 indices define 1 triangle
        std::vector<uint64_t> indices;

        // create the material container
        Material material;

        int comp;

        // fill the material description
        material.name = m.MeshMaterial.name;
        material.Ka = {m.MeshMaterial.Ka.X, m.MeshMaterial.Ka.Y, m.MeshMaterial.Ka.Z};
        material.Kd = {m.MeshMaterial.Kd.X, m.MeshMaterial.Kd.Y, m.MeshMaterial.Kd.Z};
        material.Ks = {m.MeshMaterial.Ks.X, m.MeshMaterial.Ks.Y, m.MeshMaterial.Ks.Z};
        material.Ns = m.MeshMaterial.Ns;
        material.Ni = m.MeshMaterial.Ni;
        material.d = m.MeshMaterial.d;
        material.illum = m.MeshMaterial.illum;
        material.map_Ka.name = m.MeshMaterial.map_Ka;
        material.map_Kd.name = m.MeshMaterial.map_Kd;
        // load the texture using stbi
        auto texture = stbi_load(("../Data/Basketball/" + material.map_Kd.name).c_str(),
                                 &(material.map_Kd.w), &(material.map_Kd.h), &comp,
                                 STBI_rgb);
        if (texture) {
            material.map_Kd.image.reserve(comp * material.map_Kd.w * material.map_Kd.h);
            for (int i = 0; i < comp * material.map_Kd.w * material.map_Kd.h; i++) {
                material.map_Kd.image.push_back(texture[i]);
            }
        }
        material.map_Ks.name = m.MeshMaterial.map_Ks;
        material.map_Ns.name = m.MeshMaterial.map_Ns;
        material.map_d.name = m.MeshMaterial.map_d;
        material.map_bump.name = m.MeshMaterial.map_bump;

        // copy index list
        for (unsigned int index: m.Indices) {
            indices.push_back(index);
        }

        // copy vertex list
        for (auto &item: m.Vertices) {
            TriangleMeshObject::Vertex vertex = {{item.Position.X,
                                                         item.Position.Y,
                                                         item.Position.Z},
                                                 {item.Normal.X,
                                                         item.Normal.Y,
                                                         item.Normal.Z},
                                                 {item.TextureCoordinate.X,
                                                         item.TextureCoordinate.Y}};
            vertices.push_back(vertex);
        }

        // create a triangle mesh object
        TriangleMeshObject triangleMeshObject(&vertices, &indices, &material);

        // add the object to engine
        auto id = rayEngine.addObject(triangleMeshObject);

        // add the id to our referencing ids
        objectIDs.push_back(id);
    }

    // ================================================================================================================

    // ========================================= Add Shaders to the Engine ============================================
    // use basic hit shader prefab
    BasicHitShader hitShader;

    // add hit shader to the engine, id can be used to reference to the shader within the engine
    auto hitShaderID = rayEngine.addShader(hitShader);

    // use basic ray generator shader prefab
    BasicRayGeneratorShader rayGeneratorShader;

    // add ray generator shader to the engine
    auto rayGeneratorShaderID = rayEngine.addShader(rayGeneratorShader);

    // ================================================================================================================

    // =========================================== Create Engine pipeline =============================================

    // objects that are bound to a pipeline get instanced, create instance id vector for referencing instanced objects
    // within a pipeline
    std::vector<InstanceId> instanceIDs;

    // instanced objects have their own transformation, create one for each instance
    std::vector<Matrix4x4> transforms;
    for (unsigned long i = 0; i < objectIDs.size(); i++) {
        // identity matrix, no transformation
        Matrix4x4 transform = Matrix4x4::getIdentity();

        transforms.push_back(transform);
    }

    // create a pipeline description
    PipelineDescription pipelineDescription;

    // define resolution, the ray generator shader will be called for each pixel
    int resolutionX = 1000;
    int resolutionY = 1000;
    pipelineDescription.resolutionX = resolutionX;
    pipelineDescription.resolutionY = resolutionY;

    // define the virtual camera position and orientation, the ray generator shader can take this into account
    pipelineDescription.cameraPosition = {0, 3, -10};
    pipelineDescription.cameraDirection = {0, 0, 1};
    pipelineDescription.cameraUp = {0, 1, 0};

    // add objects to the pipeline
    pipelineDescription.objectIDs = objectIDs;

    // instance ids will be returned when the pipeline is created
    pipelineDescription.objectInstanceIDs = &instanceIDs;

    // add object instance information
    pipelineDescription.objectTransformations = transforms;

    // add shaders to the pipeline
    pipelineDescription.rayGeneratorShaders.push_back({rayGeneratorShaderID, {}});
    pipelineDescription.hitShaders.push_back({hitShaderID, {}});

    // add pipeline to the engine
    auto pipelineID = rayEngine.createPipeline(pipelineDescription);

    // ================================================================================================================

    // run the pipeline
    rayEngine.runPipeline(pipelineID);

    // get the result of the pipeline
    auto texture = rayEngine.getPipelineResult(pipelineID);

    // create image container for sfml
    int channelCount = 4;
    std::vector<sf::Uint8> pixels(resolutionX * resolutionY * channelCount);

    unsigned char fullOpacity = 255;

    // translate from texture to sfml
    for (int x = 0; x < resolutionY; x++) {
        for (int y = 0; y < resolutionY; y++) {
            pixels[(x + y * resolutionY) * channelCount + 0] = texture->image[(x + y * resolutionY) * 3 + 0];
            pixels[(x + y * resolutionY) * channelCount + 1] = texture->image[(x + y * resolutionY) * 3 + 1];
            pixels[(x + y * resolutionY) * channelCount + 2] = texture->image[(x + y * resolutionY) * 3 + 2];
            pixels[(x + y * resolutionY) * channelCount + 3] = fullOpacity;
        }
    }

    // create window
    sf::RenderWindow window;
    window.create(sf::VideoMode(resolutionX, resolutionY), "Render");

    // create image
    sf::Image image;
    image.create(resolutionX, resolutionY, pixels.data());

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
    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                break;
            }
        }
    }

    return 0;
}
