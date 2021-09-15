// include relevant headers of the ray tracing engine
#include <RayTraceCore.h>
#include <Object.h>

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
    std::vector<int> objectIDs;

    // load the obj file
    if (!loader.LoadFile("../Data/Basketball/Basketball.obj")) return 1;

    // iterate over all meshes defined in the object loaded
    for (auto &m: loader.LoadedMeshes) {
        // create a vector for the meshes vertices
        std::vector<TriangleMeshObject::Vertex> vertices;

        // create a vector for the indices defined by the mesh, every index points to a vertex in vertex vector,
        // every 3 indices define 1 triangle
        std::vector<uint64_t> indices;

        // create the material container
        auto *material = new Material();

        int comp;

        // fill the material description
        material->name = m.MeshMaterial.name;
        material->Ka = {m.MeshMaterial.Ka.X, m.MeshMaterial.Ka.Y, m.MeshMaterial.Ka.Z};
        material->Kd = {m.MeshMaterial.Kd.X, m.MeshMaterial.Kd.Y, m.MeshMaterial.Kd.Z};
        material->Ks = {m.MeshMaterial.Ks.X, m.MeshMaterial.Ks.Y, m.MeshMaterial.Ks.Z};
        material->Ns = m.MeshMaterial.Ns;
        material->Ni = m.MeshMaterial.Ni;
        material->d = m.MeshMaterial.d;
        material->illum = m.MeshMaterial.illum;
        material->map_Ka.name = m.MeshMaterial.map_Ka;
        material->map_Kd.name = m.MeshMaterial.map_Kd;
        // load the texture using stbi
        material->map_Kd.image = stbi_load(("../Data/Basketball/" + material->map_Kd.name).c_str(),
                                           &(material->map_Kd.w), &(material->map_Kd.h), &comp,
                                           STBI_rgb);
        material->map_Ks.name = m.MeshMaterial.map_Ks;
        material->map_Ns.name = m.MeshMaterial.map_Ns;
        material->map_d.name = m.MeshMaterial.map_d;
        material->map_bump.name = m.MeshMaterial.map_bump;

        // copy index list
        for (unsigned int index: m.Indices) {
            indices.push_back(index);
        }

        // copy vertex list
        for (auto &item: m.Vertices) {
            TriangleMeshObject::Vertex vertex = {item.Position.X,
                                                 item.Position.Y,
                                                 item.Position.Z,
                                                 item.Normal.X,
                                                 item.Normal.Y,
                                                 item.Normal.Z,
                                                 item.TextureCoordinate.X,
                                                 item.TextureCoordinate.Y};
            vertices.push_back(vertex);
        }

        // create a triangle mesh object
        TriangleMeshObject triangleMeshObject(&vertices, &indices, material);

        // add the object to engine
        auto id = rayEngine.addObject(&triangleMeshObject);

        // add the id to our referencing ids
        objectIDs.push_back(id);
    }

    // ================================================================================================================

    // ========================================= Add Shaders to the Engine ============================================
    // use basic hit shader prefab
    BasicHitShader hitShader;

    // add hit shader to the engine, id can be used to reference to the shader within the engine
    int hitShaderID = rayEngine.addShader(&hitShader);

    // use basic ray generator shader prefab
    BasicRayGeneratorShader rayGeneratorShader;

    // add ray generator shader to the engine
    int rayGeneratorShaderID = rayEngine.addShader(&rayGeneratorShader);

    // ================================================================================================================

    // =========================================== Create Engine Pipeline =============================================

    // objects that are bound to a pipeline get instanced, create instance id vector for referencing instanced objects
    // within a pipeline
    std::vector<int> instanceIDs;

    // instanced objects have their own transformation, create one for each instance
    std::vector<Matrix4x4*> transforms;
    for(int i= 0; i < objectIDs.size(); i++) {
        // identity matrix, no transformation
        auto *transform = new Matrix4x4();
        transform->elements[0][0] = 1;
        transform->elements[0][1] = 0;
        transform->elements[0][2] = 0;
        transform->elements[0][3] = 0;
        transform->elements[1][0] = 0;
        transform->elements[1][1] = 1;
        transform->elements[1][2] = 0;
        transform->elements[1][3] = 0;
        transform->elements[2][0] = 0;
        transform->elements[2][1] = 0;
        transform->elements[2][2] = 1;
        transform->elements[2][3] = 0;
        transform->elements[3][0] = 0;
        transform->elements[3][1] = 0;
        transform->elements[3][2] = 0;
        transform->elements[3][3] = 1;

        transforms.push_back(transform);
    }

    // create a pipeline description
    PipelineDescription pipelineDescription;

    // define resolution, the ray generator shader will be called for each pixel
    pipelineDescription.resolutionX = 1000;
    pipelineDescription.resolutionY = 1000;

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
    pipelineDescription.rayGeneratorShaderIDs.push_back(rayGeneratorShaderID);
    pipelineDescription.hitShaderIDs.push_back(hitShaderID);

    // add pipeline to the engine
    int pipelineID = rayEngine.createPipeline(&pipelineDescription);

    // ================================================================================================================

    // run the pipeline
    rayEngine.runPipeline(pipelineID);

    // get the result of the pipeline
    auto texture = rayEngine.getPipelineResult(pipelineID);

    // create image container for sfml
    sf::Uint8 pixels[1000 * 1000 * 4];

    // translate from texture to sfml
    for (int x = 0; x < 1000; x++) {
        for (int y = 0; y < 1000; y++) {
            pixels[(x + y * 1000) * 4 + 0] = texture.image[(x + y * 1000) * 3 + 0];
            pixels[(x + y * 1000) * 4 + 1] = texture.image[(x + y * 1000) * 3 + 1];
            pixels[(x + y * 1000) * 4 + 2] = texture.image[(x + y * 1000) * 3 + 2];
            pixels[(x + y * 1000) * 4 + 3] = 255;
        }
    }

    // create window
    sf::RenderWindow window;
    window.create(sf::VideoMode(1000, 1000), "Render");

    // create image
    sf::Image image;
    image.create(1000, 1000, pixels);

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
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                break;
            }
        }
    }

    // cleanup
    for(auto transform : transforms){
        delete transform;
    }

    return 0;
}
