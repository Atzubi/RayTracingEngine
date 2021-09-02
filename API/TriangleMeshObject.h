//
// Created by sebastian on 13.11.19.
//

#ifndef RAYTRACECORE_TRIANGLEMESHOBJECT_H
#define RAYTRACECORE_TRIANGLEMESHOBJECT_H

#include "Object.h"

/**
 * Contains all the information required to construct a 3d model based on a 3d triangle mesh.
 * vertices:        a list of coordinates that are used as vertices
 * normals:         defines a normal per vertex
 * map:             defines mapping coordinates per vertex
 * ids:             a list of vertex ids that form a triangle
 */
class TriangleMeshObject : public Object {
public:
    struct Vertex {
        Vector3D position;
        Vector3D normal;
        Vector2D texture;
    };

private:
    friend class Triangle;
    std::vector<Vertex> vertices;
    std::vector<uint64_t> indices;
    Material material;

    std::vector<Object *> triangles;
    Object *structure;

public:
    TriangleMeshObject(const std::vector<Vertex>* vertices, const std::vector<uint64_t>* indices, const Material* material);

    ~TriangleMeshObject() override;

    BoundingBox getBoundaries() override;

    bool intersectFirst(IntersectionInfo *intersectionInfo, Ray *ray) override;
    bool intersectAny(IntersectionInfo *intersectionInfo, Ray *ray) override;
    bool intersectAll(std::vector<IntersectionInfo *> *intersectionInfo, Ray *ray) override;

    Object *clone() override;

    double getSurfaceArea() override;

    bool operator==(Object *object) override;
};

#endif //RAYTRACECORE_TRIANGLEMESHOBJECT_H
