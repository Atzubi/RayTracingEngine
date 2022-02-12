//
// Created by sebastian on 22.07.21.
//

#include <algorithm>
#include <complex>
#include "Object/Instance.h"
#include "Engine Node/EngineNode.h"

namespace {
    Vector3D getCenter(const BoundingBox &aabb) {
        return (aabb.maxCorner + aabb.minCorner) / 2.0;
    }

    void moveBoxToCenter(BoundingBox &aabb, const Vector3D &center) {
        aabb.minCorner -= center;
        aabb.maxCorner -= center;
    }

    void moveBoxBackToOriginalPosition(BoundingBox &aabb, const Vector3D &center) {
        aabb.minCorner += center;
        aabb.maxCorner += center;
    }

    void setNewBox(BoundingBox &aabb, const Vector3D &frontBottomLeft, const Vector3D &frontBottomRight,
                   const Vector3D &frontTopLeft, const Vector3D &frontTopRight, const Vector3D &backBottomLeft,
                   const Vector3D &backBottomRight, const Vector3D &backTopLeft, const Vector3D &backTopRight) {
        for (int i = 0; i < 3; i++) {
            aabb.minCorner[i] = std::min(std::min(std::min(std::min(
                    std::min(std::min(std::min(backTopRight[i], backTopLeft[i]), backBottomRight[i]),
                             backBottomLeft[i]),
                    frontTopRight[i]), frontTopLeft[i]), frontBottomRight[i]), frontBottomLeft[i]);
            aabb.maxCorner[i] = std::max(std::max(std::max(std::max(
                    std::max(std::min(std::max(backTopRight[i], backTopLeft[i]), backBottomRight[i]),
                             backBottomLeft[i]),
                    frontTopRight[i]), frontTopLeft[i]), frontBottomRight[i]), frontBottomLeft[i]);
        }
    }

    void transformOldBox(const BoundingBox &aabb, const Matrix4x4 &transform, Vector3D &frontBottomLeft,
                         Vector3D &frontBottomRight, Vector3D &frontTopLeft, Vector3D &frontTopRight,
                         Vector3D &backBottomLeft, Vector3D &backBottomRight, Vector3D &backTopLeft,
                         Vector3D &backTopRight) {
        frontBottomLeft = aabb.minCorner;
        frontBottomRight = {aabb.maxCorner.x, aabb.minCorner.y, aabb.minCorner.z};
        frontTopLeft = {aabb.minCorner.x, aabb.maxCorner.y, aabb.minCorner.z};
        frontTopRight = {aabb.maxCorner.x, aabb.maxCorner.y, aabb.minCorner.z};
        backBottomLeft = {aabb.minCorner.x, aabb.minCorner.y, aabb.maxCorner.z};
        backBottomRight = {aabb.maxCorner.x, aabb.minCorner.y, aabb.maxCorner.z};
        backTopLeft = {aabb.minCorner.x, aabb.maxCorner.y, aabb.maxCorner.z};
        backTopRight = aabb.maxCorner;

        frontBottomLeft = transform * frontBottomLeft;
        frontBottomRight = transform * frontBottomRight;
        frontTopLeft = transform * frontTopLeft;
        frontTopRight = transform * frontTopRight;
        backBottomLeft = transform * backBottomLeft;
        backBottomRight = transform * backBottomRight;
        backTopLeft = transform * backTopLeft;
        backTopRight = transform * backTopRight;
    }

    void applyTransformToBox(BoundingBox &aabb, const Matrix4x4 &transform) {
        Vector3D frontBottomLeft{};
        Vector3D frontBottomRight{};
        Vector3D frontTopLeft{};
        Vector3D frontTopRight{};
        Vector3D backBottomLeft{};
        Vector3D backBottomRight{};
        Vector3D backTopLeft{};
        Vector3D backTopRight{};

        transformOldBox(aabb, transform, frontBottomLeft, frontBottomRight, frontTopLeft, frontTopRight, backBottomLeft,
                        backBottomRight,
                        backTopLeft, backTopRight);

        setNewBox(aabb, frontBottomLeft, frontBottomRight, frontTopLeft, frontTopRight, backBottomLeft, backBottomRight,
                  backTopLeft, backTopRight);
    }

    void createTransformedAABB(BoundingBox &aabb, const Matrix4x4 &transform) {
        Vector3D center = getCenter(aabb);

        moveBoxToCenter(aabb, center);

        applyTransformToBox(aabb, transform);

        moveBoxBackToOriginalPosition(aabb, center);
    }

    bool isTransformEqual(const Matrix4x4 &transform1, const Matrix4x4 &transform2) {
        for (int x = 0; x < 4; x++) {
            for (int y = 0; y < 4; y++) {
                if (transform1.elements[x][y] != transform2.elements[x][y])
                    return false;
            }
        }
        return true;
    }

    void transformRay(const Vector3D &originalMid, Ray &newRay, const Matrix4x4 &inverseTransform) {
        newRay.origin -= originalMid;
        newRay.direction += newRay.origin;

        newRay.origin = inverseTransform * newRay.origin;
        newRay.direction = inverseTransform * newRay.direction;

        newRay.direction -= newRay.origin;
        newRay.direction.normalize();

        newRay.dirfrac = newRay.direction.getInverse();

        newRay.origin += originalMid;
    }

    void reverseTransformHit(const Ray &ray, IntersectionInfo &info, const Matrix4x4 &transform) {
        Vector3D pos = info.position;
        info.position = transform * pos;
        info.normal = transform * (info.normal + pos) - info.position;
        info.normal.normalize();
        info.distance = (ray.origin - info.position).getLength();
    }

    bool overwriteClosestHit(IntersectionInfo &intersectionInfo, const Ray &ray, IntersectionInfo &info, bool hit,
                             const Matrix4x4 &transform) {
        if (!hit || info.distance >= intersectionInfo.distance) return false;
        reverseTransformHit(ray, info, transform);
        intersectionInfo = info;
        return true;
    }

    bool overwriteAnyHit(IntersectionInfo &intersectionInfo, const Ray &ray, IntersectionInfo &info, bool hit,
                         const Matrix4x4 &transform) {
        if (!hit) return false;
        reverseTransformHit(ray, info, transform);
        intersectionInfo = info;
        return true;
    }

    bool overwriteAllHit(std::vector<IntersectionInfo> &intersectionInfo, const Ray &ray,
                         std::vector<IntersectionInfo> &infos, bool hit, const Matrix4x4 &transform) {
        if (!hit) return false;
        for (auto info: infos) {
            reverseTransformHit(ray, info, transform);
            intersectionInfo.push_back(info);
        }
        return true;
    }

    Ray createTransformedRay(const Ray &ray, const Object *baseObject, Matrix4x4 inverseTransform) {
        BoundingBox originalAABB = baseObject->getBoundaries();
        Vector3D originalMid = getCenter(originalAABB);

        Ray newRay = ray;
        transformRay(originalMid, newRay, inverseTransform);
        return newRay;
    }
}

Instance::Instance(EngineNode &node, ObjectCapsule &objectCapsule) : baseObjectId(objectCapsule.id) {
    engineNode = &node;
    objectCached = false;
    objectCache = nullptr;
    cost = objectCapsule.cost;
    boundingBox = objectCapsule.boundingBox;
    transform = Matrix4x4::getIdentity();
    inverseTransform = Matrix4x4::getIdentity();
}

void Instance::applyTransform(const Matrix4x4 &newTransform) {
    boundingBox = engineNode->requestBaseData(baseObjectId)->getBoundaries();
    transform.multiplyBy(newTransform);
    inverseTransform = transform.getInverse();
    createTransformedAABB(boundingBox, transform);
}

void Instance::invalidateCache() {
    objectCached = false;
}

Instance::~Instance() = default;

bool Instance::intersectFirst(IntersectionInfo &intersectionInfo, const Ray &ray) {
    Object *baseObject = getBaseObject();
    Ray newRay = createTransformedRay(ray, baseObject, inverseTransform);
    IntersectionInfo info{.hit = false, .distance = std::numeric_limits<double>::max()};
    bool hit = baseObject->intersectFirst(info, newRay);
    return overwriteClosestHit(intersectionInfo, ray, info, hit, transform);
}

Object *Instance::getBaseObject() {
    if (!objectCached) {
        objectCache = engineNode->requestBaseData(baseObjectId);
        objectCached = true;
    }
    return objectCache;
}

bool Instance::intersectAny(IntersectionInfo &intersectionInfo, const Ray &ray) {
    Object *baseObject = getBaseObject();
    Ray newRay = createTransformedRay(ray, baseObject, inverseTransform);
    IntersectionInfo info{.hit = false, .distance = std::numeric_limits<double>::max()};
    bool hit = baseObject->intersectAny(info, newRay);
    return overwriteAnyHit(intersectionInfo, ray, info, hit, transform);

}

bool Instance::intersectAll(std::vector<IntersectionInfo> &intersectionInfo, const Ray &ray) {
    Object *baseObject = getBaseObject();
    Ray newRay = createTransformedRay(ray, baseObject, inverseTransform);
    std::vector<IntersectionInfo> infos;
    bool hit = baseObject->intersectAll(infos, newRay);
    return overwriteAllHit(intersectionInfo, ray, infos, hit, transform);
}

BoundingBox Instance::getBoundaries() const {
    return boundingBox;
}

std::unique_ptr<Object> Instance::clone() const {
    return nullptr;
}

double Instance::getSurfaceArea() const {
    return cost + boundingBox.getSA(); // TODO: fix math
}

bool Instance::operator==(const Object &object) const {
    const auto obj = dynamic_cast<const Instance *>(&object);
    if (obj == nullptr || obj->baseObjectId != baseObjectId) return false;
    return isTransformEqual(obj->transform, transform);
}

bool Instance::operator!=(const Object &object) const {
    return !operator==(object);
}

ObjectCapsule Instance::getCapsule() const {
    ObjectCapsule capsule{{0}, getBoundaries(), getSurfaceArea()};
    return capsule;
}


