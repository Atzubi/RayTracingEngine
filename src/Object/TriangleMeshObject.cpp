//
// Created by sebastian on 13.11.19.
//

#include "../../API/TriangleMeshObject.h"

TriangleMeshObject::TriangleMeshObject(std::vector<double> *vertices, std::vector<double> *normals,
                                       std::vector<double> *map, std::vector<uint64_t> *ids) {
    if (ids->size() % 3 != 0) {
        std::__throw_invalid_argument("Invalid ID Count");
    }
    if (vertices->size() != normals->size()) {
        if (!normals->empty()) {
            std::__throw_invalid_argument("Invalid Normal Count");
        }
    } else if (vertices->size() != map->size()) {
        std::__throw_invalid_argument("Invalid Texture Coordinate Count");
    }
}

TriangleMeshObject::~TriangleMeshObject() = default;

BoundingBox TriangleMeshObject::getBoundaries() {
    return BoundingBox();
}

IntersectionInfo TriangleMeshObject::intersect(Ray ray) {
    return IntersectionInfo();
}

bool TriangleMeshObject::moveObject(Vector3D newPosition) {
    return false;
}

bool TriangleMeshObject::turnObject(Vector3D newOrientation) {
    return false;
}

bool TriangleMeshObject::scaleObject(double newScaleFactor) {
    return false;
}

bool TriangleMeshObject::manipulateObject(Vector3D newPosition, Vector3D newOrientation, double newScaleFactor) {
    return false;
}
