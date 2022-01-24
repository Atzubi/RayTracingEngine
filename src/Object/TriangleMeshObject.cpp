//
// Created by sebastian on 13.11.19.
//

#include <cmath>
#include "RayTraceEngine/TriangleMeshObject.h"
#include "Acceleration Structures/DBVHv2.h"


class Triangle : public Object {
public:
    TriangleMeshObject *mesh{};
    uint64_t pos{};

    Triangle() = default;

    Object *clone() override {
        return nullptr;
    }

    [[nodiscard]] BoundingBox getBoundaries() const override {
        Vector3D vertex1 = mesh->vertices[mesh->indices[pos]].position;
        Vector3D vertex2 = mesh->vertices[mesh->indices[pos + 1]].position;
        Vector3D vertex3 = mesh->vertices[mesh->indices[pos + 2]].position;

        Vector3D front = vertex1, back = vertex1;
        if (front.x > vertex2.x) {
            front.x = vertex2.x;
        }
        if (front.x > vertex3.x) {
            front.x = vertex3.x;
        }
        if (front.y > vertex2.y) {
            front.y = vertex2.y;
        }
        if (front.y > vertex3.y) {
            front.y = vertex3.y;
        }
        if (front.z > vertex2.z) {
            front.z = vertex2.z;
        }
        if (front.z > vertex3.z) {
            front.z = vertex3.z;
        }
        if (back.x < vertex2.x) {
            back.x = vertex2.x;
        }
        if (back.x < vertex3.x) {
            back.x = vertex3.x;
        }
        if (back.y < vertex2.y) {
            back.y = vertex2.y;
        }
        if (back.y < vertex3.y) {
            back.y = vertex3.y;
        }
        if (back.z < vertex2.z) {
            back.z = vertex2.z;
        }
        if (back.z < vertex3.z) {
            back.z = vertex3.z;
        }
        return {front, back};
    }

    bool intersectFirst(IntersectionInfo *intersectionInfo, Ray *ray) const override {
        Vector3D vertex1 = mesh->vertices[mesh->indices[pos]].position;
        Vector3D vertex2 = mesh->vertices[mesh->indices[pos + 1]].position;
        Vector3D vertex3 = mesh->vertices[mesh->indices[pos + 2]].position;

        Vector3D e1{}, e2{}, pvec{}, qvec{}, tvec{};
        double_t epsilon = 0.000001f;

        e1.x = vertex2.x - vertex1.x;
        e1.y = vertex2.y - vertex1.y;
        e1.z = vertex2.z - vertex1.z;

        e2.x = vertex3.x - vertex1.x;
        e2.y = vertex3.y - vertex1.y;
        e2.z = vertex3.z - vertex1.z;

        pvec.x = ray->direction.y * e2.z - ray->direction.z * e2.y;
        pvec.y = ray->direction.z * e2.x - ray->direction.x * e2.z;
        pvec.z = ray->direction.x * e2.y - ray->direction.y * e2.x;

        //NORMALIZE(pvec);
        double_t det = pvec.x * e1.x + pvec.y * e1.y + pvec.z * e1.z;

        if (det < epsilon && det > -epsilon) {
            intersectionInfo->hit = false;
            return false;
        }

        double_t invDet = 1.0f / det;

        tvec.x = ray->origin.x - vertex1.x;
        tvec.y = ray->origin.y - vertex1.y;
        tvec.z = ray->origin.z - vertex1.z;

        // NORMALIZE(tvec);
        double_t u = invDet * (tvec.x * pvec.x + tvec.y * pvec.y + tvec.z * pvec.z);

        if (u < 0.0f || u > 1.0f) {
            intersectionInfo->hit = false;
            return false;
        }

        qvec.x = tvec.y * e1.z - tvec.z * e1.y;
        qvec.y = tvec.z * e1.x - tvec.x * e1.z;
        qvec.z = tvec.x * e1.y - tvec.y * e1.x;

        // NORMALIZE(qvec);
        double_t v =
                invDet * (qvec.x * ray->direction.x + qvec.y * ray->direction.y + qvec.z * ray->direction.z);

        if (v < 0.0f || u + v > 1.0f) {
            intersectionInfo->hit = false;
            return false;
        }

        double t = invDet * (e2.x * qvec.x + e2.y * qvec.y + e2.z * qvec.z);

        if (t <= epsilon) {
            intersectionInfo->hit = false;
            return false;
        }

        double_t w = 1 - u - v;

        intersectionInfo->position.x = ray->origin.x + ray->direction.x * t;
        intersectionInfo->position.y = ray->origin.y + ray->direction.y * t;
        intersectionInfo->position.z = ray->origin.z + ray->direction.z * t;

        intersectionInfo->distance = sqrt(
                (ray->origin.x - intersectionInfo->position.x) * (ray->origin.x - intersectionInfo->position.x) +
                (ray->origin.y - intersectionInfo->position.y) * (ray->origin.y - intersectionInfo->position.y) +
                (ray->origin.z - intersectionInfo->position.z) * (ray->origin.z - intersectionInfo->position.z));

        Vector3D normal1 = mesh->vertices[mesh->indices[pos]].normal;
        Vector3D normal2 = mesh->vertices[mesh->indices[pos + 1]].normal;
        Vector3D normal3 = mesh->vertices[mesh->indices[pos + 2]].normal;

        intersectionInfo->normal.x = w * normal1.x + u * normal2.x + v * normal3.x;
        intersectionInfo->normal.y = w * normal1.y + u * normal2.y + v * normal3.y;
        intersectionInfo->normal.z = w * normal1.z + u * normal2.z + v * normal3.z;

        double_t length = sqrt(intersectionInfo->normal.x * intersectionInfo->normal.x +
                               intersectionInfo->normal.y * intersectionInfo->normal.y +
                               intersectionInfo->normal.z * intersectionInfo->normal.z);

        intersectionInfo->normal.x /= length;
        intersectionInfo->normal.y /= length;
        intersectionInfo->normal.z /= length;

        Vector2D texture1 = mesh->vertices[mesh->indices[pos]].texture;
        Vector2D texture2 = mesh->vertices[mesh->indices[pos + 1]].texture;
        Vector2D texture3 = mesh->vertices[mesh->indices[pos + 2]].texture;

        intersectionInfo->texture.x = w * texture1.x + u * texture2.x + v * texture3.x;
        intersectionInfo->texture.y = w * texture1.y + u * texture2.y + v * texture3.y;

        intersectionInfo->material = &mesh->material;

        intersectionInfo->hit = true;
        return true;
    }

    bool intersectAny(IntersectionInfo *intersectionInfo, Ray *ray) const override {
        return intersectFirst(intersectionInfo, ray);
    }

    bool intersectAll(std::vector<IntersectionInfo *> *intersectionInfo, Ray *ray) const override {
        auto *info = new IntersectionInfo();
        *info = {false, std::numeric_limits<double>::max(), ray->origin, ray->direction, 0, 0,
                 0, 0, 0};
        bool hit = intersectFirst(info, ray);
        intersectionInfo->push_back(info);
        return hit;
    }

    [[nodiscard]] double getSurfaceArea() const override {
        return getBoundaries().getSA();
    }

    [[nodiscard]] ObjectCapsule getCapsule() const override {
        ObjectCapsule capsule{-1, getBoundaries(), getSurfaceArea()};
        return capsule;
    }

    bool operator==(const Object &object) const override {
        const auto *triangle = dynamic_cast<const Triangle *>(&object);
        if (triangle == nullptr) {
            return false;
        } else {
            Vector3D vertex1 = mesh->vertices[mesh->indices[pos]].position;
            Vector3D vertex2 = mesh->vertices[mesh->indices[pos + 1]].position;
            Vector3D vertex3 = mesh->vertices[mesh->indices[pos + 2]].position;

            Vector3D otherVertex1 = triangle->mesh->vertices[triangle->mesh->indices[pos]].position;
            Vector3D otherVertex2 = triangle->mesh->vertices[triangle->mesh->indices[pos + 1]].position;
            Vector3D otherVertex3 = triangle->mesh->vertices[triangle->mesh->indices[pos + 2]].position;

            return otherVertex1.x == vertex1.x && otherVertex1.y == vertex1.y &&
                   otherVertex1.z == vertex1.z && otherVertex2.x == vertex2.x &&
                   otherVertex2.y == vertex2.y && otherVertex2.z == vertex2.z &&
                   otherVertex3.x == vertex3.x && otherVertex3.y == vertex3.y &&
                   otherVertex3.z == vertex3.z;
        }
    }

    ~Triangle() override = default;
};

TriangleMeshObject::TriangleMeshObject(const std::vector<Vertex> *vertices, const std::vector<uint64_t> *indices,
                                       const Material *material) {
    if (indices->size() % 3 != 0) {
        throw std::invalid_argument("Invalid Index Count");
    }

    this->vertices = *vertices;
    this->indices = *indices;
    this->material = *material;

    for (int i = 0; i < indices->size() / 3; i++) {
        auto *triangle = new Triangle();
        triangle->mesh = this;
        triangle->pos = i * 3;
        triangles.push_back(triangle);
    }

    auto *tree = new DBVHNode();
    DBVHv2::addObjects(tree, &triangles);
    structure = tree;
}

TriangleMeshObject::~TriangleMeshObject() {
    for (auto t: triangles) {
        delete t;
    }
    DBVHv2::deleteTree(structure);
};

BoundingBox TriangleMeshObject::getBoundaries() const {
    return structure->boundingBox;
}

bool TriangleMeshObject::intersectFirst(IntersectionInfo *intersectionInfo, Ray *ray) const {
    return DBVHv2::intersectFirst(structure, intersectionInfo, ray);

}

bool TriangleMeshObject::intersectAny(IntersectionInfo *intersectionInfo, Ray *ray) const {
    return DBVHv2::intersectAny(structure, intersectionInfo, ray);
}

bool TriangleMeshObject::intersectAll(std::vector<IntersectionInfo *> *intersectionInfo, Ray *ray) const {
    return DBVHv2::intersectAll(structure, intersectionInfo, ray);
}

Object *TriangleMeshObject::clone() {
    // TODO
    return new TriangleMeshObject(&vertices, &indices, &material);
}

double TriangleMeshObject::getSurfaceArea() const {
    return structure->surfaceArea;
}

bool TriangleMeshObject::operator==(const Object &object) const {
    // TODO
    return false;
}

ObjectCapsule TriangleMeshObject::getCapsule() const{
    ObjectCapsule capsule{-1, getBoundaries(), getSurfaceArea()};
    return capsule;
}