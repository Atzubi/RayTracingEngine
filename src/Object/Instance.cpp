//
// Created by sebastian on 22.07.21.
//

#include <algorithm>
#include "Object/Instance.h"
#include "Engine Node/EngineNode.h"


void createAABB(BoundingBox *aabb, Matrix4x4 *transform) {
    // apply transformation to box
    Vector3D mid = {(aabb->maxCorner.x + aabb->minCorner.x) / 2,
                    (aabb->maxCorner.y + aabb->minCorner.y) / 2,
                    (aabb->maxCorner.z + aabb->minCorner.z) / 2};

    aabb->minCorner.x -= mid.x;
    aabb->minCorner.y -= mid.y;
    aabb->minCorner.z -= mid.z;
    aabb->maxCorner.x -= mid.x;
    aabb->maxCorner.y -= mid.y;
    aabb->maxCorner.z -= mid.z;

    Vector3D frontBottomLeft = aabb->minCorner;
    Vector3D frontBottomRight = {aabb->maxCorner.x, aabb->minCorner.y, aabb->minCorner.z};
    Vector3D frontTopLeft = {aabb->minCorner.x, aabb->maxCorner.y, aabb->minCorner.z};
    Vector3D frontTopRight = {aabb->maxCorner.x, aabb->maxCorner.y, aabb->minCorner.z};
    Vector3D backBottomLeft = {aabb->minCorner.x, aabb->minCorner.y, aabb->maxCorner.z};
    Vector3D backBottomRight = {aabb->maxCorner.x, aabb->minCorner.y, aabb->maxCorner.z};
    Vector3D backTopLeft = {aabb->minCorner.x, aabb->maxCorner.y, aabb->maxCorner.z};
    Vector3D backTopRight = aabb->maxCorner;

    Vector3D buffer{};

    buffer = frontBottomLeft;
    frontBottomLeft.x = transform->elements[0][0] * buffer.x +
                        transform->elements[0][1] * buffer.y +
                        transform->elements[0][2] * buffer.z +
                        transform->elements[0][3];
    frontBottomLeft.y = transform->elements[1][0] * buffer.x +
                        transform->elements[1][1] * buffer.y +
                        transform->elements[1][2] * buffer.z +
                        transform->elements[1][3];
    frontBottomLeft.z = transform->elements[2][0] * buffer.x +
                        transform->elements[2][1] * buffer.y +
                        transform->elements[2][2] * buffer.z +
                        transform->elements[2][3];
    buffer = frontBottomRight;
    frontBottomRight.x = transform->elements[0][0] * buffer.x +
                         transform->elements[0][1] * buffer.y +
                         transform->elements[0][2] * buffer.z +
                         transform->elements[0][3];
    frontBottomRight.y = transform->elements[1][0] * buffer.x +
                         transform->elements[1][1] * buffer.y +
                         transform->elements[1][2] * buffer.z +
                         transform->elements[1][3];
    frontBottomRight.z = transform->elements[2][0] * buffer.x +
                         transform->elements[2][1] * buffer.y +
                         transform->elements[2][2] * buffer.z +
                         transform->elements[2][3];
    buffer = frontTopLeft;
    frontTopLeft.x = transform->elements[0][0] * buffer.x +
                     transform->elements[0][1] * buffer.y +
                     transform->elements[0][2] * buffer.z +
                     transform->elements[0][3];
    frontTopLeft.y = transform->elements[1][0] * buffer.x +
                     transform->elements[1][1] * buffer.y +
                     transform->elements[1][2] * buffer.z +
                     transform->elements[1][3];
    frontTopLeft.z = transform->elements[2][0] * buffer.x +
                     transform->elements[2][1] * buffer.y +
                     transform->elements[2][2] * buffer.z +
                     transform->elements[2][3];
    buffer = frontTopRight;
    frontTopRight.x = transform->elements[0][0] * buffer.x +
                      transform->elements[0][1] * buffer.y +
                      transform->elements[0][2] * buffer.z +
                      transform->elements[0][3];
    frontTopRight.y = transform->elements[1][0] * buffer.x +
                      transform->elements[1][1] * buffer.y +
                      transform->elements[1][2] * buffer.z +
                      transform->elements[1][3];
    frontTopRight.z = transform->elements[2][0] * buffer.x +
                      transform->elements[2][1] * buffer.y +
                      transform->elements[2][2] * buffer.z +
                      transform->elements[2][3];
    buffer = backBottomLeft;
    backBottomLeft.x = transform->elements[0][0] * buffer.x +
                       transform->elements[0][1] * buffer.y +
                       transform->elements[0][2] * buffer.z +
                       transform->elements[0][3];
    backBottomLeft.y = transform->elements[1][0] * buffer.x +
                       transform->elements[1][1] * buffer.y +
                       transform->elements[1][2] * buffer.z +
                       transform->elements[1][3];
    backBottomLeft.z = transform->elements[2][0] * buffer.x +
                       transform->elements[2][1] * buffer.y +
                       transform->elements[2][2] * buffer.z +
                       transform->elements[2][3];
    buffer = backBottomRight;
    backBottomRight.x = transform->elements[0][0] * buffer.x +
                        transform->elements[0][1] * buffer.y +
                        transform->elements[0][2] * buffer.z +
                        transform->elements[0][3];
    backBottomRight.y = transform->elements[1][0] * buffer.x +
                        transform->elements[1][1] * buffer.y +
                        transform->elements[1][2] * buffer.z +
                        transform->elements[1][3];
    backBottomRight.z = transform->elements[2][0] * buffer.x +
                        transform->elements[2][1] * buffer.y +
                        transform->elements[2][2] * buffer.z +
                        transform->elements[2][3];
    buffer = backTopLeft;
    backTopLeft.x = transform->elements[0][0] * buffer.x +
                    transform->elements[0][1] * buffer.y +
                    transform->elements[0][2] * buffer.z +
                    transform->elements[0][3];
    backTopLeft.y = transform->elements[1][0] * buffer.x +
                    transform->elements[1][1] * buffer.y +
                    transform->elements[1][2] * buffer.z +
                    transform->elements[1][3];
    backTopLeft.z = transform->elements[2][0] * buffer.x +
                    transform->elements[2][1] * buffer.y +
                    transform->elements[2][2] * buffer.z +
                    transform->elements[2][3];
    buffer = backTopRight;
    backTopRight.x = transform->elements[0][0] * buffer.x +
                     transform->elements[0][1] * buffer.y +
                     transform->elements[0][2] * buffer.z +
                     transform->elements[0][3];
    backTopRight.y = transform->elements[1][0] * buffer.x +
                     transform->elements[1][1] * buffer.y +
                     transform->elements[1][2] * buffer.z +
                     transform->elements[1][3];
    backTopRight.z = transform->elements[2][0] * buffer.x +
                     transform->elements[2][1] * buffer.y +
                     transform->elements[2][2] * buffer.z +
                     transform->elements[2][3];

    aabb->minCorner.x = std::min(std::min(std::min(std::min(
            std::min(std::min(std::min(backTopRight.x, backTopLeft.x), backBottomRight.x), backBottomLeft.x),
            frontTopRight.x), frontTopLeft.x), frontBottomRight.x), frontBottomLeft.x);
    aabb->minCorner.y = std::min(std::min(std::min(std::min(
            std::min(std::min(std::min(backTopRight.y, backTopLeft.y), backBottomRight.y), backBottomLeft.y),
            frontTopRight.y), frontTopLeft.y), frontBottomRight.y), frontBottomLeft.y);
    aabb->minCorner.z = std::min(std::min(std::min(std::min(
            std::min(std::min(std::min(backTopRight.z, backTopLeft.z), backBottomRight.z), backBottomLeft.z),
            frontTopRight.z), frontTopLeft.z), frontBottomRight.z), frontBottomLeft.z);
    aabb->maxCorner.x = std::max(std::max(std::max(std::max(
            std::max(std::max(std::max(backTopRight.x, backTopLeft.x), backBottomRight.x), backBottomLeft.x),
            frontTopRight.x), frontTopLeft.x), frontBottomRight.x), frontBottomLeft.x);
    aabb->maxCorner.y = std::max(std::max(std::max(std::max(
            std::max(std::max(std::max(backTopRight.y, backTopLeft.y), backBottomRight.y), backBottomLeft.y),
            frontTopRight.y), frontTopLeft.y), frontBottomRight.y), frontBottomLeft.y);
    aabb->maxCorner.z = std::max(std::max(std::max(std::max(
            std::max(std::max(std::max(backTopRight.z, backTopLeft.z), backBottomRight.z), backBottomLeft.z),
            frontTopRight.z), frontTopLeft.z), frontBottomRight.z), frontBottomLeft.z);

    aabb->minCorner.x += mid.x;
    aabb->minCorner.y += mid.y;
    aabb->minCorner.z += mid.z;
    aabb->maxCorner.x += mid.x;
    aabb->maxCorner.y += mid.y;
    aabb->maxCorner.z += mid.z;
}

Instance::Instance(EngineNode *node, ObjectCapsule *objectCapsule) {
    engineNode = node;
    baseObjectId = objectCapsule->id;
    objectCached = false;
    objectCache = nullptr;
    cost = objectCapsule->cost;
    boundingBox = objectCapsule->boundingBox;
    transform = Matrix4x4::getIdentity();
    inverseTransform = Matrix4x4::getIdentity();
}

void Instance::applyTransform(Matrix4x4 *newTransform) {
    createAABB(&boundingBox, newTransform);
    transform.multiplyBy(newTransform);
    inverseTransform = transform.getInverse();
}

void Instance::invalidateCache() {
    objectCached = false;
}

Instance::~Instance() = default;

bool Instance::intersectFirst(IntersectionInfo *intersectionInfo, Ray *ray) {
    Object *baseObject;
    if (objectCached) {
        baseObject = objectCache;
    } else {
        baseObject = engineNode->requestBaseData(baseObjectId);
    }

    Ray newRay = *ray;

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
    bool hit = baseObject->intersectFirst(&intersectionInformationBuffer, &newRay);

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
                (ray->origin.x - intersectionInformationBuffer.position.x) *
                (ray->origin.x - intersectionInformationBuffer.position.x) +
                (ray->origin.y - intersectionInformationBuffer.position.y) *
                (ray->origin.y - intersectionInformationBuffer.position.y) +
                (ray->origin.z - intersectionInformationBuffer.position.z) *
                (ray->origin.z - intersectionInformationBuffer.position.z));
    }

    if (intersectionInformationBuffer.hit) {
        if (intersectionInformationBuffer.distance < intersectionInfo->distance) {
            *intersectionInfo = intersectionInformationBuffer;
            return true;
        }
    }
    return false;
}

bool Instance::intersectAny(IntersectionInfo *intersectionInfo, Ray *ray) {
    Object *baseObject;
    if (objectCached) {
        baseObject = objectCache;
    } else {
        baseObject = engineNode->requestBaseData(baseObjectId);
    }

    Ray newRay = *ray;

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
    bool hit = baseObject->intersectAny(&intersectionInformationBuffer, &newRay);

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
                (ray->origin.x - intersectionInformationBuffer.position.x) *
                (ray->origin.x - intersectionInformationBuffer.position.x) +
                (ray->origin.y - intersectionInformationBuffer.position.y) *
                (ray->origin.y - intersectionInformationBuffer.position.y) +
                (ray->origin.z - intersectionInformationBuffer.position.z) *
                (ray->origin.z - intersectionInformationBuffer.position.z));

        *intersectionInfo = intersectionInformationBuffer;
    }

    return hit;
}

bool Instance::intersectAll(std::vector<IntersectionInfo *> *intersectionInfo, Ray *ray) {
    Object *baseObject;
    if (objectCached) {
        baseObject = objectCache;
    } else {
        baseObject = engineNode->requestBaseData(baseObjectId);
    }

    Ray newRay = *ray;

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

    std::vector<IntersectionInfo *> intersectionInformationBuffers;
    bool hit = baseObject->intersectAll(&intersectionInformationBuffers, &newRay);

    if (hit) {
        for (auto intersectionInformationBuffer: intersectionInformationBuffers) {
            Vector3D pos = intersectionInformationBuffer->position;

            intersectionInformationBuffer->position.x = transform.elements[0][0] * pos.x +
                                                        transform.elements[0][1] * pos.y +
                                                        transform.elements[0][2] * pos.z +
                                                        transform.elements[0][3];
            intersectionInformationBuffer->position.y = transform.elements[1][0] * pos.x +
                                                        transform.elements[1][1] * pos.y +
                                                        transform.elements[1][2] * pos.z +
                                                        transform.elements[1][3];
            intersectionInformationBuffer->position.z = transform.elements[2][0] * pos.x +
                                                        transform.elements[2][1] * pos.y +
                                                        transform.elements[2][2] * pos.z +
                                                        transform.elements[2][3];

            Vector3D normal = {intersectionInformationBuffer->normal.x + pos.x,
                               intersectionInformationBuffer->normal.y + pos.y,
                               intersectionInformationBuffer->normal.z + pos.z};

            intersectionInformationBuffer->normal.x = transform.elements[0][0] * normal.x +
                                                      transform.elements[0][1] * normal.y +
                                                      transform.elements[0][2] * normal.z +
                                                      transform.elements[0][3];
            intersectionInformationBuffer->normal.y = transform.elements[1][0] * normal.x +
                                                      transform.elements[1][1] * normal.y +
                                                      transform.elements[1][2] * normal.z +
                                                      transform.elements[1][3];
            intersectionInformationBuffer->normal.z = transform.elements[2][0] * normal.x +
                                                      transform.elements[2][1] * normal.y +
                                                      transform.elements[2][2] * normal.z +
                                                      transform.elements[2][3];

            intersectionInformationBuffer->normal.x =
                    intersectionInformationBuffer->normal.x - intersectionInformationBuffer->position.x;
            intersectionInformationBuffer->normal.y =
                    intersectionInformationBuffer->normal.y - intersectionInformationBuffer->position.y;
            intersectionInformationBuffer->normal.z =
                    intersectionInformationBuffer->normal.z - intersectionInformationBuffer->position.z;

            length = sqrt(intersectionInformationBuffer->normal.x * intersectionInformationBuffer->normal.x +
                          intersectionInformationBuffer->normal.y * intersectionInformationBuffer->normal.y +
                          intersectionInformationBuffer->normal.z * intersectionInformationBuffer->normal.z);

            intersectionInformationBuffer->normal.x /= length;
            intersectionInformationBuffer->normal.y /= length;
            intersectionInformationBuffer->normal.z /= length;

            intersectionInformationBuffer->
                    distance = sqrt(
                    (ray->origin.x - intersectionInformationBuffer->position.x) *
                    (ray->origin.x - intersectionInformationBuffer->position.x) +
                    (ray->origin.y - intersectionInformationBuffer->position.y) *
                    (ray->origin.y - intersectionInformationBuffer->position.y) +
                    (ray->origin.z - intersectionInformationBuffer->position.z) *
                    (ray->origin.z - intersectionInformationBuffer->position.z));

            intersectionInfo->push_back(intersectionInformationBuffer);
        }
    }

    return hit;
}

BoundingBox Instance::getBoundaries() {
    return boundingBox;
}

Object *Instance::clone() {
    return nullptr;
}

double Instance::getSurfaceArea() {
    return cost + boundingBox.getSA(); // TODO: fix math
}

bool Instance::operator==(Object *object) {
    auto obj = dynamic_cast<Instance *>(object);
    if (obj == nullptr) return false;
    if (obj->baseObjectId == baseObjectId) {
        return obj->transform.elements[0][0] == transform.elements[0][0] &&
               obj->transform.elements[0][1] == transform.elements[0][1] &&
               obj->transform.elements[0][2] == transform.elements[0][2] &&
               obj->transform.elements[0][3] == transform.elements[0][3] &&
               obj->transform.elements[1][0] == transform.elements[1][0] &&
               obj->transform.elements[1][1] == transform.elements[1][1] &&
               obj->transform.elements[1][2] == transform.elements[1][2] &&
               obj->transform.elements[1][3] == transform.elements[1][3] &&
               obj->transform.elements[2][0] == transform.elements[2][0] &&
               obj->transform.elements[2][1] == transform.elements[2][1] &&
               obj->transform.elements[2][2] == transform.elements[2][2] &&
               obj->transform.elements[2][3] == transform.elements[2][3] &&
               obj->transform.elements[3][0] == transform.elements[3][0] &&
               obj->transform.elements[3][1] == transform.elements[3][1] &&
               obj->transform.elements[3][2] == transform.elements[3][2] &&
               obj->transform.elements[3][3] == transform.elements[3][3];
    }
    return false;
}

ObjectCapsule Instance::getCapsule() {
    ObjectCapsule capsule{};
    capsule.cost = getSurfaceArea();
    capsule.boundingBox = getBoundaries();
    capsule.id = -1;
    return capsule;
}


