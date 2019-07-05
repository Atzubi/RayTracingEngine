//
// Created by sebastian on 08.04.19.
//

#include "src/Data Management Unit/HBVH/SceneData.h"

using namespace objl;

int SceneData::loadFromFile(std::string source) {
    Loader loader;

    // Loads the input data if it exists
    if (!loader.LoadFile(source)) return 1;

    // 3 indices define a triangle -> size = triangle count
    uint64_t size = loader.LoadedIndices.size() / 3;

    Primitive *triangles = new Primitive[size];

    uint64_t c = 0;
    uint64_t c3 = 0;

    // Puts the triangles into a more usable format
    for (auto i : loader.LoadedIndices) {
        if (c3 == 0) {
            triangles[c].v1.x = loader.LoadedVertices[i].Position.X;
            triangles[c].v1.y = loader.LoadedVertices[i].Position.Y;
            triangles[c].v1.z = loader.LoadedVertices[i].Position.Z;
        } else if (c3 == 1) {
            triangles[c].v2.x = loader.LoadedVertices[i].Position.X;
            triangles[c].v2.y = loader.LoadedVertices[i].Position.Y;
            triangles[c].v2.z = loader.LoadedVertices[i].Position.Z;
        } else {
            triangles[c].v3.x = loader.LoadedVertices[i].Position.X;
            triangles[c].v3.y = loader.LoadedVertices[i].Position.Y;
            triangles[c].v3.z = loader.LoadedVertices[i].Position.Z;
        }
        c3 = (c3 + 1) % 3;
        if (c3 == 0)
            c++;
    }
    return 0;
}

void SceneData::setGeometry() {

}

void SceneData::addGeometry() {

}

void SceneData::removeGeometry() {

}

void SceneData::setTextues() {

}

void SceneData::addTexture() {

}

void SceneData::removeTexture() {

}

void SceneData::addMap() {

}

void SceneData::removeMap() {

}
