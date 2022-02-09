//
// Created by sebastian on 22.07.21.
//

#include <algorithm>
#include <complex>
#include "Object/Instance.h"
#include "Engine Node/EngineNode.h"

namespace {
    double multiplyLineVector(const Matrix4x4 &transform, const Vector3D &vector, int line) {
        return transform.elements[line][0] * vector.x +
               transform.elements[line][1] * vector.y +
               transform.elements[line][2] * vector.z +
               transform.elements[line][3];
    }

    Vector3D multiplyMatrixVector(const Matrix4x4 &transform, const Vector3D &vector) {
        Vector3D result{};
        for (int i = 0; i < 3; i++) {
            result[i] = multiplyLineVector(transform, vector, i);
        }
        return result;
    }

    Vector3D getCenter(const BoundingBox &aabb) {
        return {(aabb.maxCorner.x + aabb.minCorner.x) / 2,
                (aabb.maxCorner.y + aabb.minCorner.y) / 2,
                (aabb.maxCorner.z + aabb.minCorner.z) / 2};
    }

    void moveBoxToCenter(BoundingBox &aabb, const Vector3D &center) {
        for (int i = 0; i < 3; i++) {
            aabb.minCorner[i] -= center[i];
            aabb.maxCorner[i] -= center[i];
        }
    }

    void moveBoxBackToOriginalPosition(BoundingBox &aabb, const Vector3D &center) {
        for (int i = 0; i < 3; i++) {
            aabb.minCorner[i] += center[i];
            aabb.maxCorner[i] += center[i];
        }
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

        frontBottomLeft = multiplyMatrixVector(transform, frontBottomLeft);
        frontBottomRight = multiplyMatrixVector(transform, frontBottomRight);
        frontTopLeft = multiplyMatrixVector(transform, frontTopLeft);
        frontTopRight = multiplyMatrixVector(transform, frontTopRight);
        backBottomLeft = multiplyMatrixVector(transform, backBottomLeft);
        backBottomRight = multiplyMatrixVector(transform, backBottomRight);
        backTopLeft = multiplyMatrixVector(transform, backTopLeft);
        backTopRight = multiplyMatrixVector(transform, backTopRight);
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
        newRay.origin.x -= originalMid.x;
        newRay.origin.y -= originalMid.y;
        newRay.origin.z -= originalMid.z;

        newRay.direction.x += newRay.origin.x;
        newRay.direction.y += newRay.origin.y;
        newRay.direction.z += newRay.origin.z;

        newRay.origin = multiplyMatrixVector(inverseTransform, newRay.origin);
        newRay.direction = multiplyMatrixVector(inverseTransform, newRay.direction);

        newRay.direction.x -= newRay.origin.x;
        newRay.direction.y -= newRay.origin.y;
        newRay.direction.z -= newRay.origin.z;

        double length = sqrt(newRay.direction.x * newRay.direction.x + newRay.direction.y * newRay.direction.y +
                             newRay.direction.z * newRay.direction.z);

        newRay.direction.x /= length;
        newRay.direction.y /= length;
        newRay.direction.z /= length;

        newRay.dirfrac.x = 1.0 / newRay.direction.x;
        newRay.dirfrac.y = 1.0 / newRay.direction.y;
        newRay.dirfrac.z = 1.0 / newRay.direction.z;

        newRay.origin.x += originalMid.x;
        newRay.origin.y += originalMid.y;
        newRay.origin.z += originalMid.z;
    }

    void reverseTransformHit(const Ray &ray, IntersectionInfo &info, const Matrix4x4 &transform) {
        Vector3D pos = info.position;

        info.position = multiplyMatrixVector(transform, pos);

        Vector3D normal = {info.normal.x + pos.x,
                           info.normal.y + pos.y,
                           info.normal.z + pos.z};

        info.normal = multiplyMatrixVector(transform, normal);

        info.normal.x = info.normal.x - info.position.x;
        info.normal.y = info.normal.y - info.position.y;
        info.normal.z = info.normal.z - info.position.z;

        double length = sqrt(
                info.normal.x * info.normal.x + info.normal.y * info.normal.y + info.normal.z * info.normal.z);

        info.normal.x /= length;
        info.normal.y /= length;
        info.normal.z /= length;

        info.distance = sqrt((ray.origin.x - info.position.x) * (ray.origin.x - info.position.x) +
                             (ray.origin.y - info.position.y) * (ray.origin.y - info.position.y) +
                             (ray.origin.z - info.position.z) * (ray.origin.z - info.position.z));
    }

    bool overwriteClosestHit(IntersectionInfo &intersectionInfo, const Ray &ray, IntersectionInfo &info, bool hit,
                             Matrix4x4 transform) {
        if (!hit || info.distance >= intersectionInfo.distance) return false;
        reverseTransformHit(ray, info, transform);
        intersectionInfo = info;
        return true;
    }

    IntersectionInfo initInfo(){
        IntersectionInfo info{};
        info.hit = false;
        info.distance = std::numeric_limits<double>::max();
        info.position = {0, 0, 0};
        return info;
    }

    Ray createTransformedRay(const Ray &ray, const Object *baseObject, Matrix4x4 inverseTransform)  {
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
    IntersectionInfo info = initInfo();
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
    Object *baseObject;
    if (objectCached) {
        baseObject = objectCache;
    } else {
        baseObject = engineNode->requestBaseData(baseObjectId);
    }

    Ray newRay = ray;

    BoundingBox originalAABB = baseObject->getBoundaries();

    Vector3D originalMid = {(originalAABB.maxCorner.x + originalAABB.minCorner.x) / 2,
                            (originalAABB.maxCorner.y + originalAABB.minCorner.y) / 2,
                            (originalAABB.maxCorner.z + originalAABB.minCorner.z) / 2};

    newRay.origin.x -= originalMid.x;
    newRay.origin.y -= originalMid.y;
    newRay.origin.z -= originalMid.z;

    Vector3D directionBuffer{};
    directionBuffer.x = newRay.origin.x + newRay.direction.x;
    directionBuffer.y = newRay.origin.y + newRay.direction.y;
    directionBuffer.z = newRay.origin.z + newRay.direction.z;

    Vector3D originBuffer = newRay.origin;
    newRay.origin.x = inverseTransform.elements[0][0] * originBuffer.x +
                      inverseTransform.elements[0][1] * originBuffer.y +
                      inverseTransform.elements[0][2] * originBuffer.z +
                      inverseTransform.elements[0][3];
    newRay.origin.y = inverseTransform.elements[1][0] * originBuffer.x +
                      inverseTransform.elements[1][1] * originBuffer.y +
                      inverseTransform.elements[1][2] * originBuffer.z +
                      inverseTransform.elements[1][3];
    newRay.origin.z = inverseTransform.elements[2][0] * originBuffer.x +
                      inverseTransform.elements[2][1] * originBuffer.y +
                      inverseTransform.elements[2][2] * originBuffer.z +
                      inverseTransform.elements[2][3];

    newRay.direction.x = inverseTransform.elements[0][0] * directionBuffer.x +
                         inverseTransform.elements[0][1] * directionBuffer.y +
                         inverseTransform.elements[0][2] * directionBuffer.z +
                         inverseTransform.elements[0][3];
    newRay.direction.y = inverseTransform.elements[1][0] * directionBuffer.x +
                         inverseTransform.elements[1][1] * directionBuffer.y +
                         inverseTransform.elements[1][2] * directionBuffer.z +
                         inverseTransform.elements[1][3];
    newRay.direction.z = inverseTransform.elements[2][0] * directionBuffer.x +
                         inverseTransform.elements[2][1] * directionBuffer.y +
                         inverseTransform.elements[2][2] * directionBuffer.z +
                         inverseTransform.elements[2][3];

    newRay.direction.x = newRay.direction.x - newRay.origin.x;
    newRay.direction.y = newRay.direction.y - newRay.origin.y;
    newRay.direction.z = newRay.direction.z - newRay.origin.z;

    double length = sqrt(newRay.direction.x * newRay.direction.x + newRay.direction.y * newRay.direction.y +
                         newRay.direction.z * newRay.direction.z);

    newRay.direction.x /= length;
    newRay.direction.y /= length;
    newRay.direction.z /= length;

    newRay.dirfrac.x = 1.0 / newRay.direction.x;
    newRay.dirfrac.y = 1.0 / newRay.direction.y;
    newRay.dirfrac.z = 1.0 / newRay.direction.z;

    newRay.origin.x += originalMid.x;
    newRay.origin.y += originalMid.y;
    newRay.origin.z += originalMid.z;

    IntersectionInfo intersectionInformationBuffer{};
    intersectionInformationBuffer.hit = false;
    intersectionInformationBuffer.distance = std::numeric_limits<double>::max();
    intersectionInformationBuffer.position = {0, 0, 0};
    bool hit = baseObject->intersectAny(intersectionInformationBuffer, newRay);

    if (hit) {
        Vector3D pos = intersectionInformationBuffer.position;

        intersectionInformationBuffer.position.x = transform.elements[0][0] * pos.x +
                                                   transform.elements[0][1] * pos.y +
                                                   transform.elements[0][2] * pos.z +
                                                   transform.elements[0][3];
        intersectionInformationBuffer.position.y = transform.elements[1][0] * pos.x +
                                                   transform.elements[1][1] * pos.y +
                                                   transform.elements[1][2] * pos.z +
                                                   transform.elements[1][3];
        intersectionInformationBuffer.position.z = transform.elements[2][0] * pos.x +
                                                   transform.elements[2][1] * pos.y +
                                                   transform.elements[2][2] * pos.z +
                                                   transform.elements[2][3];

        Vector3D normal = {intersectionInformationBuffer.normal.x + pos.x,
                           intersectionInformationBuffer.normal.y + pos.y,
                           intersectionInformationBuffer.normal.z + pos.z};

        intersectionInformationBuffer.normal.x = transform.elements[0][0] * normal.x +
                                                 transform.elements[0][1] * normal.y +
                                                 transform.elements[0][2] * normal.z +
                                                 transform.elements[0][3];
        intersectionInformationBuffer.normal.y = transform.elements[1][0] * normal.x +
                                                 transform.elements[1][1] * normal.y +
                                                 transform.elements[1][2] * normal.z +
                                                 transform.elements[1][3];
        intersectionInformationBuffer.normal.z = transform.elements[2][0] * normal.x +
                                                 transform.elements[2][1] * normal.y +
                                                 transform.elements[2][2] * normal.z +
                                                 transform.elements[2][3];

        intersectionInformationBuffer.normal.x =
                intersectionInformationBuffer.normal.x - intersectionInformationBuffer.position.x;
        intersectionInformationBuffer.normal.y =
                intersectionInformationBuffer.normal.y - intersectionInformationBuffer.position.y;
        intersectionInformationBuffer.normal.z =
                intersectionInformationBuffer.normal.z - intersectionInformationBuffer.position.z;

        length = sqrt(intersectionInformationBuffer.normal.x * intersectionInformationBuffer.normal.x +
                      intersectionInformationBuffer.normal.y * intersectionInformationBuffer.normal.y +
                      intersectionInformationBuffer.normal.z * intersectionInformationBuffer.normal.z);

        intersectionInformationBuffer.normal.x /= length;
        intersectionInformationBuffer.normal.y /= length;
        intersectionInformationBuffer.normal.z /= length;

        intersectionInformationBuffer.
                distance = sqrt(
                (ray.origin.x - intersectionInformationBuffer.position.x) *
                (ray.origin.x - intersectionInformationBuffer.position.x) +
                (ray.origin.y - intersectionInformationBuffer.position.y) *
                (ray.origin.y - intersectionInformationBuffer.position.y) +
                (ray.origin.z - intersectionInformationBuffer.position.z) *
                (ray.origin.z - intersectionInformationBuffer.position.z));

        intersectionInfo = intersectionInformationBuffer;
    }

    return hit;
}

bool Instance::intersectAll(std::vector<IntersectionInfo> &intersectionInfo, const Ray &ray) {
    Object *baseObject;
    if (objectCached) {
        baseObject = objectCache;
    } else {
        baseObject = engineNode->requestBaseData(baseObjectId);
    }

    Ray newRay = ray;

    BoundingBox originalAABB = baseObject->getBoundaries();

    Vector3D originalMid = {(originalAABB.maxCorner.x + originalAABB.minCorner.x) / 2,
                            (originalAABB.maxCorner.y + originalAABB.minCorner.y) / 2,
                            (originalAABB.maxCorner.z + originalAABB.minCorner.z) / 2};

    newRay.origin.x -= originalMid.x;
    newRay.origin.y -= originalMid.y;
    newRay.origin.z -= originalMid.z;

    Vector3D directionBuffer{};
    directionBuffer.x = newRay.origin.x + newRay.direction.x;
    directionBuffer.y = newRay.origin.y + newRay.direction.y;
    directionBuffer.z = newRay.origin.z + newRay.direction.z;

    Vector3D originBuffer = newRay.origin;
    newRay.origin.x = inverseTransform.elements[0][0] * originBuffer.x +
                      inverseTransform.elements[0][1] * originBuffer.y +
                      inverseTransform.elements[0][2] * originBuffer.z +
                      inverseTransform.elements[0][3];
    newRay.origin.y = inverseTransform.elements[1][0] * originBuffer.x +
                      inverseTransform.elements[1][1] * originBuffer.y +
                      inverseTransform.elements[1][2] * originBuffer.z +
                      inverseTransform.elements[1][3];
    newRay.origin.z = inverseTransform.elements[2][0] * originBuffer.x +
                      inverseTransform.elements[2][1] * originBuffer.y +
                      inverseTransform.elements[2][2] * originBuffer.z +
                      inverseTransform.elements[2][3];

    newRay.direction.x = inverseTransform.elements[0][0] * directionBuffer.x +
                         inverseTransform.elements[0][1] * directionBuffer.y +
                         inverseTransform.elements[0][2] * directionBuffer.z +
                         inverseTransform.elements[0][3];
    newRay.direction.y = inverseTransform.elements[1][0] * directionBuffer.x +
                         inverseTransform.elements[1][1] * directionBuffer.y +
                         inverseTransform.elements[1][2] * directionBuffer.z +
                         inverseTransform.elements[1][3];
    newRay.direction.z = inverseTransform.elements[2][0] * directionBuffer.x +
                         inverseTransform.elements[2][1] * directionBuffer.y +
                         inverseTransform.elements[2][2] * directionBuffer.z +
                         inverseTransform.elements[2][3];

    newRay.direction.x = newRay.direction.x - newRay.origin.x;
    newRay.direction.y = newRay.direction.y - newRay.origin.y;
    newRay.direction.z = newRay.direction.z - newRay.origin.z;

    double length = sqrt(newRay.direction.x * newRay.direction.x + newRay.direction.y * newRay.direction.y +
                         newRay.direction.z * newRay.direction.z);

    newRay.direction.x /= length;
    newRay.direction.y /= length;
    newRay.direction.z /= length;

    newRay.dirfrac.x = 1.0 / newRay.direction.x;
    newRay.dirfrac.y = 1.0 / newRay.direction.y;
    newRay.dirfrac.z = 1.0 / newRay.direction.z;

    newRay.origin.x += originalMid.x;
    newRay.origin.y += originalMid.y;
    newRay.origin.z += originalMid.z;

    std::vector<IntersectionInfo> intersectionInformationBuffers;
    bool hit = baseObject->intersectAll(intersectionInformationBuffers, newRay);

    if (hit) {
        for (auto intersectionInformationBuffer: intersectionInformationBuffers) {
            Vector3D pos = intersectionInformationBuffer.position;

            intersectionInformationBuffer.position.x = transform.elements[0][0] * pos.x +
                                                       transform.elements[0][1] * pos.y +
                                                       transform.elements[0][2] * pos.z +
                                                       transform.elements[0][3];
            intersectionInformationBuffer.position.y = transform.elements[1][0] * pos.x +
                                                       transform.elements[1][1] * pos.y +
                                                       transform.elements[1][2] * pos.z +
                                                       transform.elements[1][3];
            intersectionInformationBuffer.position.z = transform.elements[2][0] * pos.x +
                                                       transform.elements[2][1] * pos.y +
                                                       transform.elements[2][2] * pos.z +
                                                       transform.elements[2][3];

            Vector3D normal = {intersectionInformationBuffer.normal.x + pos.x,
                               intersectionInformationBuffer.normal.y + pos.y,
                               intersectionInformationBuffer.normal.z + pos.z};

            intersectionInformationBuffer.normal.x = transform.elements[0][0] * normal.x +
                                                     transform.elements[0][1] * normal.y +
                                                     transform.elements[0][2] * normal.z +
                                                     transform.elements[0][3];
            intersectionInformationBuffer.normal.y = transform.elements[1][0] * normal.x +
                                                     transform.elements[1][1] * normal.y +
                                                     transform.elements[1][2] * normal.z +
                                                     transform.elements[1][3];
            intersectionInformationBuffer.normal.z = transform.elements[2][0] * normal.x +
                                                     transform.elements[2][1] * normal.y +
                                                     transform.elements[2][2] * normal.z +
                                                     transform.elements[2][3];

            intersectionInformationBuffer.normal.x =
                    intersectionInformationBuffer.normal.x - intersectionInformationBuffer.position.x;
            intersectionInformationBuffer.normal.y =
                    intersectionInformationBuffer.normal.y - intersectionInformationBuffer.position.y;
            intersectionInformationBuffer.normal.z =
                    intersectionInformationBuffer.normal.z - intersectionInformationBuffer.position.z;

            length = sqrt(intersectionInformationBuffer.normal.x * intersectionInformationBuffer.normal.x +
                          intersectionInformationBuffer.normal.y * intersectionInformationBuffer.normal.y +
                          intersectionInformationBuffer.normal.z * intersectionInformationBuffer.normal.z);

            intersectionInformationBuffer.normal.x /= length;
            intersectionInformationBuffer.normal.y /= length;
            intersectionInformationBuffer.normal.z /= length;

            intersectionInformationBuffer.
                    distance = sqrt(
                    (ray.origin.x - intersectionInformationBuffer.position.x) *
                    (ray.origin.x - intersectionInformationBuffer.position.x) +
                    (ray.origin.y - intersectionInformationBuffer.position.y) *
                    (ray.origin.y - intersectionInformationBuffer.position.y) +
                    (ray.origin.z - intersectionInformationBuffer.position.z) *
                    (ray.origin.z - intersectionInformationBuffer.position.z));

            intersectionInfo.push_back(intersectionInformationBuffer);
        }
    }

    return hit;
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


