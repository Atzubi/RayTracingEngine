//
// Created by Sebastian on 13.12.2021.
//

#include <algorithm>
#include "DBVHv2.h"

void refit(BoundingBox *target, BoundingBox resizeBy) {
    target->minCorner.x = std::min(target->minCorner.x, resizeBy.minCorner.x);
    target->minCorner.y = std::min(target->minCorner.y, resizeBy.minCorner.y);
    target->minCorner.z = std::min(target->minCorner.z, resizeBy.minCorner.z);
    target->maxCorner.x = std::max(target->maxCorner.x, resizeBy.maxCorner.x);
    target->maxCorner.y = std::max(target->maxCorner.y, resizeBy.maxCorner.y);
    target->maxCorner.z = std::max(target->maxCorner.z, resizeBy.maxCorner.z);
}

void refit(BoundingBox &aabb, Object *object) {
    refit(&aabb, object->getBoundaries());
}

void refit(BoundingBox &aabb, std::vector<Object *> *objects, double looseness) {
    // refit box to fit all objects
    for (auto &object: *objects) {
        refit(aabb, object);
    }

    // increase box size by looseness factor
    if (looseness > 0) {
        double scaleX = aabb.maxCorner.x - aabb.minCorner.x;
        double scaleY = aabb.maxCorner.y - aabb.minCorner.y;
        double scaleZ = aabb.maxCorner.z - aabb.minCorner.z;
        aabb.minCorner.x -= scaleX * looseness;
        aabb.minCorner.y -= scaleY * looseness;
        aabb.minCorner.z -= scaleZ * looseness;
        aabb.maxCorner.x += scaleX * looseness;
        aabb.maxCorner.y += scaleY * looseness;
        aabb.maxCorner.z += scaleZ * looseness;
    }
}

double evaluateBucket(BoundingBox *leftChildBox, BoundingBox *rightChildBox, double leftSAH, double rightSAH,
                      std::vector<Object *> *objects, Vector3D splittingPlane, uint8_t *newParent) {
    // initialize both bucket boxes
    BoundingBox aabbLeft = {std::numeric_limits<double>::max(), std::numeric_limits<double>::max(),
                            std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(),
                            -std::numeric_limits<double>::max(), -std::numeric_limits<double>::max()};
    BoundingBox aabbRight = {std::numeric_limits<double>::max(), std::numeric_limits<double>::max(),
                             std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(),
                             -std::numeric_limits<double>::max(), -std::numeric_limits<double>::max()};
    /*BoundingBox aabbLeft = *leftChildBox;
    BoundingBox aabbRight = *rightChildBox;*/

    double leftCount = 0;
    double rightCount = 0;

    // sort all objects into their bucket, then update the buckets bounding box
    if (splittingPlane.x != 0) {
        for (auto &object: *objects) {
            if ((object->getBoundaries().maxCorner.x + object->getBoundaries().minCorner.x) / 2 <
                splittingPlane.x) {
                refit(aabbLeft, object);
                leftCount += 1;
            } else {
                refit(aabbRight, object);
                rightCount += 1;
            }
        }
    } else if (splittingPlane.y != 0) {
        for (auto &object: *objects) {
            if ((object->getBoundaries().maxCorner.y + object->getBoundaries().minCorner.y) / 2 <
                splittingPlane.y) {
                refit(aabbLeft, object);
                leftCount += 1;
            } else {
                refit(aabbRight, object);
                rightCount += 1;
            }
        }
    } else {
        for (auto &object: *objects) {
            if ((object->getBoundaries().maxCorner.z + object->getBoundaries().minCorner.z) / 2 <
                splittingPlane.z) {
                refit(aabbLeft, object);
                leftCount += 1;
            } else {
                refit(aabbRight, object);
                rightCount += 1;
            }
        }
    }

    if (leftChildBox != nullptr && rightChildBox != nullptr) {
        BoundingBox oldLeft = *leftChildBox;
        BoundingBox oldLeftNewLeft = *leftChildBox;
        BoundingBox oldLeftNewRight = *leftChildBox;
        BoundingBox oldLeftNewLeftNewRight = *leftChildBox;
        BoundingBox oldLeftOldRight = *leftChildBox;
        BoundingBox oldLeftOldRightNewLeft = *leftChildBox;
        BoundingBox oldLeftOldRightNewRight = *leftChildBox;
        BoundingBox oldRightNewLeftNewRight = *rightChildBox;
        BoundingBox oldRightNewRight = *rightChildBox;
        BoundingBox oldRightNewLeft = *rightChildBox;
        BoundingBox oldRight = *rightChildBox;
        BoundingBox newLeftNewRight = aabbLeft;
        BoundingBox newRight = aabbRight;
        BoundingBox newLeft = aabbLeft;

        double SAHs[7];
        double SAH = std::numeric_limits<double>::max();
        int bestSAH = 0;

        refit(&newLeftNewRight, newRight);
        refit(&oldLeftNewLeft, newLeft);
        refit(&oldLeftNewRight, newRight);
        refit(&oldLeftNewLeftNewRight, newLeftNewRight);
        refit(&oldLeftOldRight, oldRight);
        refit(&oldRightNewLeft, newLeft);
        refit(&oldRightNewRight, newRight);
        refit(&oldLeftOldRightNewLeft, oldRightNewLeft);
        refit(&oldLeftOldRightNewRight, oldRightNewRight);
        refit(&oldRightNewLeftNewRight, newLeftNewRight);

        SAHs[0] = oldLeftNewLeft.getSA() * (leftCount + leftSAH) +
                  oldRightNewRight.getSA() * (rightCount + rightSAH);
        SAHs[1] = oldLeftNewRight.getSA() * (rightCount + leftSAH) +
                  oldRightNewLeft.getSA() * (leftCount + rightSAH);
        SAHs[2] = oldLeft.getSA() * leftSAH +
                  oldRightNewLeftNewRight.getSA() * (leftCount + rightCount + rightSAH);
        SAHs[3] = oldLeftNewLeftNewRight.getSA() * (leftCount + rightCount + leftSAH) +
                  oldRight.getSA() * rightSAH;
        SAHs[4] = oldLeftOldRight.getSA() * (leftSAH + rightSAH) +
                  newLeftNewRight.getSA() * (leftCount + rightCount);
        SAHs[5] = oldLeftOldRightNewLeft.getSA() * (leftCount + leftSAH + rightSAH) +
                  newRight.getSA() * rightCount;
        SAHs[6] = oldLeftOldRightNewRight.getSA() * (rightCount + leftSAH + rightSAH) +
                  newLeft.getSA() * leftCount;

        for (int i = 0; i < 7; i++) {
            if (SAHs[i] < SAH) {
                SAH = SAHs[i];
                bestSAH = i;
            }
        }

        *newParent = bestSAH + 1;
        return SAHs[bestSAH];
    } else {
        // return the combined surface area of both boxes
        return aabbLeft.getSA() * leftCount + aabbRight.getSA() * rightCount;
    }
}

void split(std::vector<Object *> *leftChild, std::vector<Object *> *rightChild, std::vector<Object *> *objects,
           Vector3D splittingPlane) {
    if (splittingPlane.x != 0) {
        for (auto &object: *objects) {
            if ((object->getBoundaries().maxCorner.x + object->getBoundaries().minCorner.x) / 2 <
                splittingPlane.x) {
                leftChild->push_back(object);
            } else {
                rightChild->push_back(object);
            }
        }
    } else if (splittingPlane.y != 0) {
        for (auto &object: *objects) {
            if ((object->getBoundaries().maxCorner.y + object->getBoundaries().minCorner.y) / 2 <
                splittingPlane.y) {
                leftChild->push_back(object);
            } else {
                rightChild->push_back(object);
            }
        }
    } else {
        for (auto &object: *objects) {
            if ((object->getBoundaries().maxCorner.z + object->getBoundaries().minCorner.z) / 2 <
                splittingPlane.z) {
                leftChild->push_back(object);
            } else {
                rightChild->push_back(object);
            }
        }
    }
}

bool contains(BoundingBox aabb1, BoundingBox aabb2) {
    return aabb1.minCorner.x <= aabb2.minCorner.x &&
           aabb1.maxCorner.x >= aabb2.maxCorner.x &&
           aabb1.minCorner.y <= aabb2.minCorner.y &&
           aabb1.maxCorner.y >= aabb2.maxCorner.y &&
           aabb1.minCorner.z <= aabb2.minCorner.z &&
           aabb1.maxCorner.z >= aabb2.maxCorner.z;
}

bool rayBoxIntersection(Vector3D *min, Vector3D *max, Ray *ray, double *distance) {
    double t1 = (min->x - ray->origin.x) * ray->dirfrac.x;
    double t2 = (max->x - ray->origin.x) * ray->dirfrac.x;
    double t3 = (min->y - ray->origin.y) * ray->dirfrac.y;
    double t4 = (max->y - ray->origin.y) * ray->dirfrac.y;
    double t5 = (min->z - ray->origin.z) * ray->dirfrac.z;
    double t6 = (max->z - ray->origin.z) * ray->dirfrac.z;

    double tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
    double tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

    *distance = tmin;
    return tmax >= 0 && tmin <= tmax;
}

static void add(DBVHNode *currentNode, std::vector<Object *> *objects, uint8_t depth) {

}

static void remove(DBVHNode *currentNode, Object *object) {

}

static void remove(DBVHNode **currentNode, std::vector<Object *> *objects) {

}

static bool traverseALl(DBVHNode *root, std::vector<IntersectionInfo *> *intersectionInfo, Ray *ray) {
    auto **stack = new DBVHNode *[root->maxDepthRight > root->maxDepthLeft ? root->maxDepthRight : root->maxDepthLeft];
    uint64_t stackPointer = 1;
    stack[0] = root;

    while (stackPointer != 0) {
        auto *node = stack[stackPointer - 1];
        stackPointer--;

        double distanceRight = 0;
        double distanceLeft = 0;

        if (node->maxDepthRight > 1) {
            // TODO request child if missing
            auto *rightChild = node->rightChild;
            if (rayBoxIntersection(&(rightChild->boundingBox.minCorner),
                                   &(rightChild->boundingBox.maxCorner), ray, &distanceRight)) {
                stack[stackPointer++] = node->rightChild;
            }
        } else {
            // TODO request leaf if missing
            auto *intersectionInformationBuffer = new IntersectionInfo();
            intersectionInformationBuffer->hit = false;
            intersectionInformationBuffer->distance = std::numeric_limits<double>::max();
            intersectionInformationBuffer->position = {0, 0, 0};
            auto *rightLeaf = node->rightLeaf;
            rightLeaf->intersectFirst(intersectionInformationBuffer, ray);
            if (intersectionInformationBuffer->hit) {
                intersectionInfo->push_back(intersectionInformationBuffer);
            } else {
                delete intersectionInformationBuffer;
            }
        }
        if (node->maxDepthLeft > 1) {
            // TODO request child if missing
            auto *leftChild = node->leftChild;
            if (rayBoxIntersection(&(leftChild->boundingBox.minCorner),
                                   &(leftChild->boundingBox.maxCorner), ray, &distanceLeft)) {
                stack[stackPointer++] = node->leftChild;
            }
        } else {
            // TODO request leaf if missing
            auto *intersectionInformationBuffer = new IntersectionInfo();
            intersectionInformationBuffer->hit = false;
            intersectionInformationBuffer->distance = std::numeric_limits<double>::max();
            intersectionInformationBuffer->position = {0, 0, 0};
            auto *leftLeaf = node->leftLeaf;
            leftLeaf->intersectFirst(intersectionInformationBuffer, ray);
            if (intersectionInformationBuffer->hit) {
                intersectionInfo->push_back(intersectionInformationBuffer);
            } else {
                delete intersectionInformationBuffer;
            }
        }
    }

    delete[] stack;
    return false;
}

struct TraversalContainer {
    DBVHNode *node;
    double distance;
};

static bool traverseFirst(DBVHNode *root, IntersectionInfo *intersectionInfo, Ray *ray) {
    bool hit = false;

    auto *stack = new TraversalContainer[root->maxDepthRight > root->maxDepthLeft ? root->maxDepthRight
                                                                                  : root->maxDepthLeft];
    uint64_t stackPointer = 1;
    stack[0] = {root, 0};

    while (stackPointer != 0) {
        if (stack[stackPointer - 1].distance >= intersectionInfo->distance) {
            stackPointer--;
            continue;
        }
        auto *node = stack[stackPointer - 1].node;
        stackPointer--;

        double distanceRight = 0;
        double distanceLeft = 0;
        bool right = false;
        bool left = false;

        if (node->maxDepthRight > 1) {
            // TODO request child if missing
            auto *rightChild = node->rightChild;
            right = rayBoxIntersection(&(rightChild->boundingBox.minCorner),
                                       &(rightChild->boundingBox.maxCorner), ray, &distanceRight);
        } else {
            // TODO request leaf if missing
            IntersectionInfo intersectionInformationBuffer{};
            intersectionInformationBuffer.hit = false;
            intersectionInformationBuffer.distance = std::numeric_limits<double>::max();
            intersectionInformationBuffer.position = {0, 0, 0};
            auto *rightLeaf = node->rightLeaf;
            rightLeaf->intersectFirst(&intersectionInformationBuffer, ray);
            if (intersectionInformationBuffer.hit) {
                if (intersectionInformationBuffer.distance < intersectionInfo->distance) {
                    *intersectionInfo = intersectionInformationBuffer;
                    hit = true;
                }
            }
        }
        if (node->maxDepthLeft > 1) {
            // TODO request child if missing
            auto *leftChild = node->leftChild;
            left = rayBoxIntersection(&(leftChild->boundingBox.minCorner),
                                      &(leftChild->boundingBox.maxCorner), ray, &distanceLeft);
        } else {
            // TODO request leaf if missing
            IntersectionInfo intersectionInformationBuffer{};
            intersectionInformationBuffer.hit = false;
            intersectionInformationBuffer.distance = std::numeric_limits<double>::max();
            intersectionInformationBuffer.position = {0, 0, 0};
            auto *leftLeaf = node->leftLeaf;
            leftLeaf->intersectFirst(&intersectionInformationBuffer, ray);
            if (intersectionInformationBuffer.hit) {
                if (intersectionInformationBuffer.distance < intersectionInfo->distance) {
                    *intersectionInfo = intersectionInformationBuffer;
                    hit = true;
                }
            }
        }

        if (right && left) {
            if (distanceRight < distanceLeft) {
                stack[stackPointer++] = {node->leftChild, distanceLeft};
                stack[stackPointer++] = {node->rightChild, distanceRight};
            } else {
                stack[stackPointer++] = {node->rightChild, distanceRight};
                stack[stackPointer++] = {node->leftChild, distanceLeft};
            }
        } else if (right) {
            stack[stackPointer++] = {node->rightChild, distanceRight};
        } else if (left) {
            stack[stackPointer++] = {node->leftChild, distanceLeft};
        }
    }

    delete[] stack;
    return hit;
}

static bool traverseAny(DBVHNode *root, IntersectionInfo *intersectionInfo, Ray *ray) {
    auto **stack = new DBVHNode *[root->maxDepthRight > root->maxDepthLeft ? root->maxDepthRight : root->maxDepthLeft];
    uint64_t stackPointer = 1;
    stack[0] = root;

    while (stackPointer != 0) {
        auto *node = stack[stackPointer - 1];
        stackPointer--;

        double distanceRight = 0;
        double distanceLeft = 0;

        if (node->maxDepthRight > 1) {
            // TODO request child if missing
            auto *rightChild = node->rightChild;
            if (rayBoxIntersection(&(rightChild->boundingBox.minCorner),
                                   &(rightChild->boundingBox.maxCorner), ray, &distanceRight)) {
                stack[stackPointer++] = node->rightChild;
            }
        } else {
            // TODO request leaf if missing
            IntersectionInfo intersectionInformationBuffer{};
            intersectionInformationBuffer.hit = false;
            intersectionInformationBuffer.distance = std::numeric_limits<double>::max();
            intersectionInformationBuffer.position = {0, 0, 0};
            auto *rightLeaf = node->rightLeaf;
            rightLeaf->intersectAny(&intersectionInformationBuffer, ray);
            if (intersectionInformationBuffer.hit) {
                *intersectionInfo = intersectionInformationBuffer;
                delete[] stack;
                return true;
            }
        }
        if (node->maxDepthLeft > 1) {
            // TODO request child if missing
            auto *leftChild = node->leftChild;
            if (rayBoxIntersection(&(leftChild->boundingBox.minCorner),
                                   &(leftChild->boundingBox.maxCorner), ray, &distanceLeft)) {
                stack[stackPointer++] = node->leftChild;
            }
        } else {
            // TODO request leaf if missing
            IntersectionInfo intersectionInformationBuffer{};
            intersectionInformationBuffer.hit = false;
            intersectionInformationBuffer.distance = std::numeric_limits<double>::max();
            intersectionInformationBuffer.position = {0, 0, 0};
            auto *leftLeaf = node->leftLeaf;
            leftLeaf->intersectAny(&intersectionInformationBuffer, ray);
            if (intersectionInformationBuffer.hit) {
                *intersectionInfo = intersectionInformationBuffer;
                delete[] stack;
                return true;
            }
        }
    }

    delete[] stack;
    return false;
}

void DBVHv2::addObjects(DBVHNode *root, std::vector<Object *> *objects) {
    if (objects->empty()) return;
    if (root == nullptr) {
        // TODO error handling (should never happen)
    } else if (root->maxDepthLeft == 0) {
        root->leftLeaf = objects->back();
        objects->pop_back();
        root->maxDepthLeft = 1;
        if (!objects->empty()) {
            root->rightLeaf = objects->back();
            objects->pop_back();
            root->maxDepthRight = 1;
        }
    } else if (root->maxDepthRight == 0) {
        root->rightLeaf = objects->back();
        objects->pop_back();
        root->maxDepthRight = 1;
    }
    if (objects->empty()) return;
    add(root, objects, 1);
}

void DBVHv2::removeObjects(DBVHNode *root, std::vector<Object *> *objects) {

}

bool DBVHv2::intersectFirst(DBVHNode *root, IntersectionInfo *intersectionInfo, Ray *ray) {
    bool hit;

    if (root == nullptr) return false;

    if (root->maxDepthLeft == 0) {
        return false;
    } else {
        if (root->maxDepthRight == 0) {
            hit = root->leftLeaf->intersectFirst(intersectionInfo, ray);
        } else {
            hit = traverseFirst(root, intersectionInfo, ray);
        }
    }

    return hit;
}

bool DBVHv2::intersectAny(DBVHNode *root, IntersectionInfo *intersectionInfo, Ray *ray) {
    bool hit;

    if (root == nullptr) return false;

    if (root->maxDepthLeft == 0) {
        return false;
    } else {
        if (root->maxDepthRight == 0) {
            hit = root->leftLeaf->intersectAny(intersectionInfo, ray);
        } else {
            hit = traverseAny(root, intersectionInfo, ray);
        }
    }

    return hit;
}

bool DBVHv2::intersectAll(DBVHNode *root, std::vector<IntersectionInfo *> *intersectionInfo, Ray *ray) {
    bool hit;

    if (root == nullptr) return false;

    if (root->maxDepthLeft == 0) {
        return false;
    } else {
        if (root->maxDepthRight == 0) {
            hit = root->leftLeaf->intersectAll(intersectionInfo, ray);
        } else {
            hit = traverseALl(root, intersectionInfo, ray);
        }
    }

    return hit;
}
