//
// Created by sebastian on 19.07.21.
//

#include "Acceleration Structures/DBVH.h"


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

DBVH::InnerNode::InnerNode() {
    surfaceArea = 0;
    leftChild = nullptr;
    rightChild = nullptr;
    boundingBox = {std::numeric_limits<double>::max(), std::numeric_limits<double>::max(),
                   std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(),
                   -std::numeric_limits<double>::max(), -std::numeric_limits<double>::max()};
}

uint8_t DBVH::InnerNode::getType() {
    return 0;
}

BoundingBox DBVH::InnerNode::getBoundingBox() {
    return boundingBox;
}

bool DBVH::InnerNode::optimizeSAH() {
    int bestSAH = 0;
    double SAHs[5];

    BoundingBox leftBox{}, rightBox{}, leftLeftBox{}, leftRightBox{}, rightLeftBox{}, rightRightBox{};
    double leftSA = 0, rightSA = 0, leftLeftSA = 0, leftRightSA = 0, rightLeftSA = 0, rightRightSA = 0;

    BoundingBox swapLeftLeftToRight{}, swapLeftRightToRight{}, swapRightLeftToLeft{}, swapRightRightToLeft{};

    SAHs[0] = surfaceArea;

    if (leftChild->getType() == 0) {
        auto *leftNode = (InnerNode *) leftChild;
        leftBox = leftNode->boundingBox;
        leftSA = leftNode->surfaceArea;
        if (leftNode->leftChild->getType() == 0) {
            leftLeftBox = ((InnerNode *) (leftNode->leftChild))->boundingBox;
            leftLeftSA = ((InnerNode *) (leftNode->leftChild))->surfaceArea;
        } else {
            leftLeftBox = ((Child *) (leftNode->leftChild))->object->getBoundaries();
            leftLeftSA = ((Child *) (leftNode->leftChild))->object->getSurfaceArea();
        }
        if (leftNode->rightChild->getType() == 0) {
            leftRightBox = ((InnerNode *) (leftNode->rightChild))->boundingBox;
            leftRightSA = ((InnerNode *) (leftNode->rightChild))->surfaceArea;
        } else {
            leftRightBox = ((Child *) (leftNode->rightChild))->object->getBoundaries();
            leftRightSA = ((Child *) (leftNode->rightChild))->object->getSurfaceArea();
        }

        if (rightChild->getType() == 0) {
            auto *rightNode = (InnerNode *) rightChild;
            rightBox = rightNode->boundingBox;
            rightSA = rightNode->surfaceArea;
            if (rightNode->leftChild->getType() == 0) {
                rightLeftBox = ((InnerNode *) (rightNode->leftChild))->boundingBox;
                rightLeftSA = ((InnerNode *) (rightNode->leftChild))->surfaceArea;
            } else {
                rightLeftBox = ((Child *) (rightNode->leftChild))->object->getBoundaries();
                rightLeftSA = ((Child *) (rightNode->leftChild))->object->getSurfaceArea();
            }
            if (rightNode->rightChild->getType() == 0) {
                rightRightBox = ((InnerNode *) (rightNode->rightChild))->boundingBox;
                rightRightSA = ((InnerNode *) (rightNode->rightChild))->surfaceArea;
            } else {
                rightRightBox = ((Child *) (rightNode->rightChild))->object->getBoundaries();
                rightRightSA = ((Child *) (rightNode->rightChild))->object->getSurfaceArea();
            }

            swapLeftLeftToRight = {std::min(rightBox.minCorner.x, leftRightBox.minCorner.x),
                                   std::min(rightBox.minCorner.y, leftRightBox.minCorner.y),
                                   std::min(rightBox.minCorner.z, leftRightBox.minCorner.z),
                                   std::max(rightBox.maxCorner.x, leftRightBox.maxCorner.x),
                                   std::max(rightBox.maxCorner.y, leftRightBox.maxCorner.y),
                                   std::max(rightBox.maxCorner.z, leftRightBox.maxCorner.z)};
            swapLeftRightToRight = {std::min(rightBox.minCorner.x, leftLeftBox.minCorner.x),
                                    std::min(rightBox.minCorner.y, leftLeftBox.minCorner.y),
                                    std::min(rightBox.minCorner.z, leftLeftBox.minCorner.z),
                                    std::max(rightBox.maxCorner.x, leftLeftBox.maxCorner.x),
                                    std::max(rightBox.maxCorner.y, leftLeftBox.maxCorner.y),
                                    std::max(rightBox.maxCorner.z, leftLeftBox.maxCorner.z)};
            swapRightLeftToLeft = {std::min(leftBox.minCorner.x, rightRightBox.minCorner.x),
                                   std::min(leftBox.minCorner.y, rightRightBox.minCorner.y),
                                   std::min(leftBox.minCorner.z, rightRightBox.minCorner.z),
                                   std::max(leftBox.maxCorner.x, rightRightBox.maxCorner.x),
                                   std::max(leftBox.maxCorner.y, rightRightBox.maxCorner.y),
                                   std::max(leftBox.maxCorner.z, rightRightBox.maxCorner.z)};
            swapRightRightToLeft = {std::min(leftBox.minCorner.x, rightLeftBox.minCorner.x),
                                    std::min(leftBox.minCorner.y, rightLeftBox.minCorner.y),
                                    std::min(leftBox.minCorner.z, rightLeftBox.minCorner.z),
                                    std::max(leftBox.maxCorner.x, rightLeftBox.maxCorner.x),
                                    std::max(leftBox.maxCorner.y, rightLeftBox.maxCorner.y),
                                    std::max(leftBox.maxCorner.z, rightLeftBox.maxCorner.z)};

            SAHs[1] =
                    boundingBox.getSA() + leftLeftSA + rightSA + leftRightSA + swapLeftLeftToRight.getSA();
            SAHs[2] =
                    boundingBox.getSA() + leftRightSA + rightSA + leftLeftSA + swapLeftRightToRight.getSA();
            SAHs[3] =
                    boundingBox.getSA() + rightLeftSA + leftSA + rightRightSA + swapRightLeftToLeft.getSA();
            SAHs[4] = boundingBox.getSA() + rightRightSA + leftSA + rightLeftSA +
                      swapRightRightToLeft.getSA();
        } else {
            auto *rightNode = (Child *) rightChild;
            rightBox = rightNode->object->getBoundaries();
            rightSA = rightNode->object->getSurfaceArea();

            swapLeftLeftToRight = {std::min(rightBox.minCorner.x, leftRightBox.minCorner.x),
                                   std::min(rightBox.minCorner.y, leftRightBox.minCorner.y),
                                   std::min(rightBox.minCorner.z, leftRightBox.minCorner.z),
                                   std::max(rightBox.maxCorner.x, leftRightBox.maxCorner.x),
                                   std::max(rightBox.maxCorner.y, leftRightBox.maxCorner.y),
                                   std::max(rightBox.maxCorner.z, leftRightBox.maxCorner.z)};
            swapLeftRightToRight = {std::min(rightBox.minCorner.x, leftLeftBox.minCorner.x),
                                    std::min(rightBox.minCorner.y, leftLeftBox.minCorner.y),
                                    std::min(rightBox.minCorner.z, leftLeftBox.minCorner.z),
                                    std::max(rightBox.maxCorner.x, leftLeftBox.maxCorner.x),
                                    std::max(rightBox.maxCorner.y, leftLeftBox.maxCorner.y),
                                    std::max(rightBox.maxCorner.z, leftLeftBox.maxCorner.z)};

            SAHs[1] =
                    boundingBox.getSA() + leftLeftSA + rightSA + leftRightSA + swapLeftLeftToRight.getSA();
            SAHs[2] =
                    boundingBox.getSA() + leftRightSA + rightSA + leftLeftSA + swapLeftRightToRight.getSA();
            SAHs[3] = std::numeric_limits<double>::max();
            SAHs[4] = std::numeric_limits<double>::max();
        }
    } else {
        auto *leftNode = (Child *) leftChild;
        leftBox = leftNode->object->getBoundaries();
        leftSA = leftNode->object->getSurfaceArea();

        if (rightChild->getType() == 0) {
            auto *rightNode = (InnerNode *) rightChild;
            rightBox = rightNode->boundingBox;
            rightSA = rightNode->surfaceArea;
            if (rightNode->leftChild->getType() == 0) {
                rightLeftBox = ((InnerNode *) (rightNode->leftChild))->boundingBox;
                rightLeftSA = ((InnerNode *) (rightNode->leftChild))->surfaceArea;
            } else {
                rightLeftBox = ((Child *) (rightNode->leftChild))->object->getBoundaries();
                rightLeftSA = ((Child *) (rightNode->leftChild))->object->getSurfaceArea();
            }
            if (rightNode->rightChild->getType() == 0) {
                rightRightBox = ((InnerNode *) (rightNode->rightChild))->boundingBox;
                rightRightSA = ((InnerNode *) (rightNode->rightChild))->surfaceArea;
            } else {
                rightRightBox = ((Child *) (rightNode->rightChild))->object->getBoundaries();
                rightRightSA = ((Child *) (rightNode->rightChild))->object->getSurfaceArea();
            }

            swapRightLeftToLeft = {std::min(leftBox.minCorner.x, rightRightBox.minCorner.x),
                                   std::min(leftBox.minCorner.y, rightRightBox.minCorner.y),
                                   std::min(leftBox.minCorner.z, rightRightBox.minCorner.z),
                                   std::max(leftBox.maxCorner.x, rightRightBox.maxCorner.x),
                                   std::max(leftBox.maxCorner.y, rightRightBox.maxCorner.y),
                                   std::max(leftBox.maxCorner.z, rightRightBox.maxCorner.z)};
            swapRightRightToLeft = {std::min(leftBox.minCorner.x, rightLeftBox.minCorner.x),
                                    std::min(leftBox.minCorner.y, rightLeftBox.minCorner.y),
                                    std::min(leftBox.minCorner.z, rightLeftBox.minCorner.z),
                                    std::max(leftBox.maxCorner.x, rightLeftBox.maxCorner.x),
                                    std::max(leftBox.maxCorner.y, rightLeftBox.maxCorner.y),
                                    std::max(leftBox.maxCorner.z, rightLeftBox.maxCorner.z)};

            SAHs[1] = std::numeric_limits<double>::max();
            SAHs[2] = std::numeric_limits<double>::max();
            SAHs[3] =
                    boundingBox.getSA() + rightLeftSA + leftSA + rightRightSA + swapRightLeftToLeft.getSA();
            SAHs[4] = boundingBox.getSA() + rightRightSA + leftSA + rightLeftSA +
                      swapRightRightToLeft.getSA();
        } else {
            return false;
        }
    }


    if (SAHs[1] < SAHs[0]) {
        bestSAH = 1;
    }
    if (SAHs[2] < SAHs[bestSAH]) {
        bestSAH = 2;
    }
    if (SAHs[3] < SAHs[bestSAH]) {
        bestSAH = 3;
    }
    if (SAHs[4] < SAHs[bestSAH]) {
        bestSAH = 4;
    }

    switch (bestSAH) {
        case 1: {
            auto buffer = rightChild;
            rightChild = ((InnerNode *) leftChild)->leftChild;
            ((InnerNode *) leftChild)->boundingBox = swapLeftLeftToRight;
            ((InnerNode *) leftChild)->leftChild = buffer;

            surfaceArea = SAHs[1];
            ((InnerNode *) leftChild)->surfaceArea = rightSA + leftRightSA + swapLeftLeftToRight.getSA();
            return true;
        }
        case 2: {
            auto buffer = rightChild;
            rightChild = ((InnerNode *) leftChild)->rightChild;
            ((InnerNode *) leftChild)->boundingBox = swapLeftRightToRight;
            ((InnerNode *) leftChild)->rightChild = buffer;

            surfaceArea = SAHs[2];
            ((InnerNode *) leftChild)->surfaceArea = rightSA + leftLeftSA + swapLeftRightToRight.getSA();
            return true;
        }
        case 3: {
            auto buffer = leftChild;
            leftChild = ((InnerNode *) rightChild)->leftChild;
            ((InnerNode *) rightChild)->boundingBox = swapRightLeftToLeft;
            ((InnerNode *) rightChild)->leftChild = buffer;

            surfaceArea = SAHs[3];
            ((InnerNode *) rightChild)->surfaceArea = leftSA + rightRightSA + swapRightLeftToLeft.getSA();
            return true;
        }
        case 4: {
            auto buffer = leftChild;
            leftChild = ((InnerNode *) rightChild)->rightChild;
            ((InnerNode *) rightChild)->boundingBox = swapRightRightToLeft;
            ((InnerNode *) rightChild)->rightChild = buffer;

            surfaceArea = SAHs[4];
            ((InnerNode *) rightChild)->surfaceArea = leftSA + rightLeftSA + swapRightRightToLeft.getSA();
            return true;
        }
        default:
            surfaceArea = SAHs[0];
            return false;
    }
}

DBVH::InnerNode::~InnerNode() {
    delete leftChild;
    delete rightChild;
}

void DBVH::InnerNode::refit() {
    boundingBox = {std::numeric_limits<double>::max(),
                   std::numeric_limits<double>::max(),
                   std::numeric_limits<double>::max(),
                   -std::numeric_limits<double>::max(),
                   -std::numeric_limits<double>::max(),
                   -std::numeric_limits<double>::max()};
    if (rightChild->getType() == 0) {
        ::refit(&boundingBox, ((InnerNode *) rightChild)->boundingBox);
        surfaceArea = ((InnerNode *) rightChild)->surfaceArea;
    } else {
        ::refit(&boundingBox, ((Child *) rightChild)->object->getBoundaries());
        surfaceArea = ((Child *) rightChild)->object->getSurfaceArea();
    }
    if (leftChild->getType() == 0) {
        ::refit(&boundingBox, ((InnerNode *) leftChild)->boundingBox);
        surfaceArea += ((InnerNode *) leftChild)->surfaceArea;
        surfaceArea += boundingBox.getSA();
    } else {
        ::refit(&boundingBox, ((Child *) leftChild)->object->getBoundaries());
        surfaceArea += ((Child *) leftChild)->object->getSurfaceArea();
        surfaceArea += boundingBox.getSA();
    }
}

uint8_t DBVH::Child::getType() {
    return 1;
}

BoundingBox DBVH::Child::getBoundingBox() {
    return object->getBoundaries();
}

void DBVH::add(DBVH::Node *currentNode, std::vector<Object *> *objects, uint8_t depth) {
    if (depth > maxDepth) maxDepth = depth;

    // refit current node to objects
    auto *node = (InnerNode *) currentNode;
    refit(node->boundingBox, objects, 0);

    // create split buckets
    Vector3D splittingPlanes[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                   0};
    uint8_t newParent[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

    double x = (node->boundingBox.maxCorner.x - node->boundingBox.minCorner.x) / 4.0;
    splittingPlanes[0].x = node->boundingBox.minCorner.x + x;
    splittingPlanes[1].x = node->boundingBox.minCorner.x + x * 2.0;
    splittingPlanes[2].x = node->boundingBox.minCorner.x + x * 3.0;

    double y = (node->boundingBox.maxCorner.y - node->boundingBox.minCorner.y) / 4.0;
    splittingPlanes[3].y = node->boundingBox.minCorner.y + y;
    splittingPlanes[4].y = node->boundingBox.minCorner.y + y * 2.0;
    splittingPlanes[5].y = node->boundingBox.minCorner.y + y * 3.0;

    double z = (node->boundingBox.maxCorner.z - node->boundingBox.minCorner.z) / 4.0;
    splittingPlanes[6].z = node->boundingBox.minCorner.z + z;
    splittingPlanes[7].z = node->boundingBox.minCorner.z + z * 2.0;
    splittingPlanes[8].z = node->boundingBox.minCorner.z + z * 3.0;

    // evaluate split buckets
    double SAH[9];
    BoundingBox leftBox = {std::numeric_limits<double>::max(), std::numeric_limits<double>::max(),
                           std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(),
                           -std::numeric_limits<double>::max(), -std::numeric_limits<double>::max()};
    BoundingBox rightBox = {std::numeric_limits<double>::max(), std::numeric_limits<double>::max(),
                            std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(),
                            -std::numeric_limits<double>::max(), -std::numeric_limits<double>::max()};
    double leftSA = 0, rightSA = 0;

    if (node->leftChild != nullptr) {
        if (node->leftChild->getType() == 0) {
            leftBox = ((InnerNode *) (node->leftChild))->boundingBox;
            leftSA = ((InnerNode *) (node->leftChild))->surfaceArea / leftBox.getSA();
        } else {
            leftBox = ((Child *) (node->leftChild))->object->getBoundaries();
            leftSA = ((Child *) (node->leftChild))->object->getSurfaceArea() / leftBox.getSA();
        }
        if (node->rightChild != nullptr) {
            if (node->rightChild->getType() == 0) {
                rightBox = ((InnerNode *) (node->rightChild))->boundingBox;
                rightSA = ((InnerNode *) (node->rightChild))->surfaceArea / rightBox.getSA();
            } else {
                rightBox = ((Child *) (node->rightChild))->object->getBoundaries();
                rightSA = ((Child *) (node->rightChild))->object->getSurfaceArea() / rightBox.getSA();
            }
            for (int i = 0; i < 9; i++) {
                SAH[i] = evaluateBucket(&leftBox, &rightBox, leftSA, rightSA, objects, splittingPlanes[i],
                                        &(newParent[i]));
            }
        } else {
            for (int i = 0; i < 9; i++) {
                SAH[i] = evaluateBucket(nullptr, nullptr, 0, 0, objects, splittingPlanes[i], &(newParent[i]));
            }
        }
    } else {
        for (int i = 0; i < 9; i++) {
            SAH[i] = evaluateBucket(nullptr, nullptr, 0, 0, objects, splittingPlanes[i], &(newParent[i]));
        }
    }

    // choose best split bucket and split node accordingly
    auto *leftObjects = new std::vector<Object *>();
    auto *rightObjects = new std::vector<Object *>();

    double bestSAH = std::numeric_limits<double>::max();
    int bestSplittingPlane = -1;

    for (int i = 0; i < 9; i++) {
        if (SAH[i] < bestSAH) {
            bestSAH = SAH[i];
            bestSplittingPlane = i;
        }
    }

    if (bestSplittingPlane == -1) {
        for (uint64_t i = 0; i < objects->size() / 2; i++) {
            leftObjects->push_back(objects->at(i));
        }
        for (uint64_t i = objects->size() / 2; i < objects->size(); i++) {
            rightObjects->push_back(objects->at(i));
        }
    } else {
        switch (newParent[bestSplittingPlane]) {
            case 0:
                // default
            case 1:
                // existing boxes with correct order
                split(leftObjects, rightObjects, objects, splittingPlanes[bestSplittingPlane]);
                break;
            case 2: {
                // existing boxes with wrong order
                split(leftObjects, rightObjects, objects, splittingPlanes[bestSplittingPlane]);
                auto buffer = leftObjects;
                leftObjects = rightObjects;
                rightObjects = buffer;
                break;
            }
            case 3: {
                // existing box all new left
                *leftObjects = *objects;
                break;
            }
            case 4: {
                // existing box all new right
                *rightObjects = *objects;
                break;
            }
            case 5: {
                // new box split
                auto *newNode = new InnerNode();
                newNode->rightChild = node->rightChild;
                newNode->leftChild = node->leftChild;
                refit(&newNode->boundingBox, node->rightChild->getBoundingBox());
                refit(&newNode->boundingBox, node->leftChild->getBoundingBox());
                newNode->surfaceArea = node->surfaceArea;
                node->leftChild = newNode;
                node->rightChild = nullptr;
                *rightObjects = *objects;
                break;
            }
            case 6: {
                // new box correct order
                split(leftObjects, rightObjects, objects, splittingPlanes[bestSplittingPlane]);
                auto *newNode = new InnerNode();
                newNode->rightChild = node->rightChild;
                newNode->leftChild = node->leftChild;
                refit(&newNode->boundingBox, node->rightChild->getBoundingBox());
                refit(&newNode->boundingBox, node->leftChild->getBoundingBox());
                newNode->surfaceArea = node->surfaceArea;
                node->leftChild = newNode;
                node->rightChild = nullptr;
                break;
            }
            case 7: {
                // new box wrong order
                split(leftObjects, rightObjects, objects, splittingPlanes[bestSplittingPlane]);
                auto buffer = leftObjects;
                leftObjects = rightObjects;
                rightObjects = buffer;
                auto *newNode = new InnerNode();
                newNode->rightChild = node->rightChild;
                newNode->leftChild = node->leftChild;
                refit(&newNode->boundingBox, node->rightChild->getBoundingBox());
                refit(&newNode->boundingBox, node->leftChild->getBoundingBox());
                newNode->surfaceArea = node->surfaceArea;
                node->leftChild = newNode;
                node->rightChild = nullptr;
                break;
            }
            default:
                std::cout << "wtf\n";
                throw (std::exception());
        }
    }

    // pass objects to children
    if (leftObjects->size() == 1) {
        if (node->leftChild == nullptr) {
            // create new child
            auto child = new Child();
            child->object = leftObjects->at(0);
            node->leftChild = child;
        } else if (node->leftChild->getType() == 1) {
            // create new parent for both children
            auto buffer = node->leftChild;
            auto child = new Child();
            auto parent = new InnerNode();
            child->object = leftObjects->at(0);
            parent->boundingBox = buffer->getBoundingBox();
            refit(parent->boundingBox, leftObjects, 0);
            /*double SA = parent->boundingBox.getSA();
            SA += child->object->getSurfaceArea();
            SA += ((Child *) buffer)->object->getSurfaceArea();
            parent->surfaceArea = SA;*/
            parent->leftChild = buffer;
            parent->rightChild = child;
            node->leftChild = parent;
        } else {
            add(node->leftChild, leftObjects, depth + 1);
        }
    } else if (!leftObjects->empty()) {
        if (node->leftChild == nullptr) {
            // create new child
            auto child = new InnerNode();
            node->leftChild = child;
            add(node->leftChild, leftObjects, depth + 1);
        } else if (node->leftChild->getType() == 1) {
            // create new parent for both children
            auto buffer = node->leftChild;
            auto parent = new InnerNode();
            parent->leftChild = buffer;
            parent->boundingBox = buffer->getBoundingBox();
            parent->surfaceArea = parent->boundingBox.getSA() * 2;
            node->leftChild = parent;
            add(node->leftChild, leftObjects, depth + 1);
        } else {
            add(node->leftChild, leftObjects, depth + 1);
        }
    }

    if (rightObjects->size() == 1) {
        if (node->rightChild == nullptr) {
            // create new child
            auto child = new Child();
            child->object = rightObjects->at(0);
            node->rightChild = child;
        } else if (node->rightChild->getType() == 1) {
            // create new parent for both children
            auto buffer = node->rightChild;
            auto child = new Child();
            auto parent = new InnerNode();
            child->object = rightObjects->at(0);
            parent->boundingBox = buffer->getBoundingBox();
            refit(parent->boundingBox, rightObjects, 0);
            /*double SA = parent->boundingBox.getSA();
            SA += child->object->getSurfaceArea();
            SA += ((Child *) buffer)->object->getSurfaceArea();
            parent->surfaceArea = SA;*/
            parent->leftChild = buffer;
            parent->rightChild = child;
            node->rightChild = parent;
        } else {
            add(node->rightChild, rightObjects, depth + 1);
        }
    } else if (!rightObjects->empty()) {
        if (node->rightChild == nullptr) {
            // create new child
            auto child = new InnerNode();
            node->rightChild = child;
            add(node->rightChild, rightObjects, depth + 1);
        } else if (node->rightChild->getType() == 1) {
            // create new parent for both children
            auto buffer = node->rightChild;
            auto parent = new InnerNode();
            parent->rightChild = buffer;
            parent->boundingBox = buffer->getBoundingBox();
            parent->surfaceArea = parent->boundingBox.getSA() * 2;
            node->rightChild = parent;
            add(node->rightChild, rightObjects, depth + 1);
        } else {
            add(node->rightChild, rightObjects, depth + 1);
        }
    }

    delete leftObjects;
    delete rightObjects;

    node->surfaceArea = node->boundingBox.getSA();
    if (node->leftChild->getType() == 0) {
        node->surfaceArea += ((InnerNode *) node->leftChild)->surfaceArea;
    } else {
        node->surfaceArea += ((Child *) node->leftChild)->object->getSurfaceArea();
    }
    if (node->rightChild->getType() == 0) {
        node->surfaceArea += ((InnerNode *) node->rightChild)->surfaceArea;
    } else {
        node->surfaceArea += ((Child *) node->rightChild)->object->getSurfaceArea();
    }

    // use tree rotations going the tree back up to optimize SAH
    node->optimizeSAH();
}

void DBVH::remove(DBVH::Node *currentNode, Object *object) {
    if (currentNode->getType() == 0) {
        auto node = (InnerNode *) currentNode;
        if (contains(node->boundingBox, object->getBoundaries())) {
            if (node->leftChild->getType() == 0) {
                auto child = (InnerNode *) (node->leftChild);
                if (contains(child->boundingBox, object->getBoundaries())) {
                    if (child->leftChild->getType() == 1) {
                        auto grandChild = (Child *) (child->leftChild);
                        if (grandChild->object->operator==(object)) {
                            node->leftChild = child->rightChild;
                            delete grandChild;
                            node->refit();
                            return;
                        }
                    }
                    if (child->rightChild->getType() == 1) {
                        auto grandChild = (Child *) (child->rightChild);
                        if (grandChild->object->operator==(object)) {
                            node->leftChild = child->leftChild;
                            delete grandChild;
                            node->refit();
                            return;
                        }
                    }
                }
            }
            if (node->rightChild->getType() == 0) {
                auto child = (InnerNode *) (node->rightChild);
                if (contains(child->boundingBox, object->getBoundaries())) {
                    if (child->leftChild->getType() == 1) {
                        auto grandChild = (Child *) (child->leftChild);
                        if (grandChild->object->operator==(object)) {
                            node->rightChild = child->rightChild;
                            delete grandChild;
                            node->refit();
                            return;
                        }
                    }
                    if (child->rightChild->getType() == 1) {
                        auto grandChild = (Child *) (child->rightChild);
                        if (grandChild->object->operator==(object)) {
                            node->rightChild = child->leftChild;
                            delete grandChild;
                            node->refit();
                            return;
                        }
                    }
                }
            }
            remove(node->leftChild, object);
            remove(node->rightChild, object);

            node->refit();

            node->optimizeSAH();
        }
    }
}

void DBVH::remove(DBVH::Node **currentNode, std::vector<Object *> *objects) {
    if (*currentNode == nullptr) return;
    // find object in tree by insertion
    // remove object and refit nodes going the tree back up
    for (auto &object: *objects) {
        if ((*currentNode)->getType() == 0) {
            auto node = (InnerNode *) *currentNode;
            if (node->leftChild->getType() == 1) {
                auto child = (Child *) (node->leftChild);
                if (child->object->operator==(object)) {
                    delete child;
                    *currentNode = node->rightChild;
                    continue;
                }
            }
            if (node->rightChild->getType() == 1) {
                auto child = (Child *) (node->rightChild);
                if (child->object->operator==(object)) {
                    delete child;
                    *currentNode = node->leftChild;
                    continue;
                }
            }
        } else {
            if (((Child *) *currentNode)->object->operator==(object)) {
                delete root;
                root = nullptr;
                return;
            }
        }
        remove(*currentNode, object);
    }
}

bool DBVH::traverseALl(std::vector<IntersectionInfo *> *intersectionInfo, Ray *ray) {
    Node **stack = new Node *[maxDepth];
    uint64_t stackPointer = 1;
    stack[0] = root;

    while (stackPointer != 0) {
        auto *node = (InnerNode *) (stack[stackPointer - 1]);
        stackPointer--;

        double distanceRight = 0;
        double distanceLeft = 0;

        if (node->rightChild->getType() == 0) {
            auto *rightChild = (InnerNode *) node->rightChild;
            if (rayBoxIntersection(&(rightChild->boundingBox.minCorner),
                                   &(rightChild->boundingBox.maxCorner), ray, &distanceRight)) {
                stack[stackPointer++] = node->rightChild;
            }
        } else {
            auto *intersectionInformationBuffer = new IntersectionInfo();
            intersectionInformationBuffer->hit = false;
            intersectionInformationBuffer->distance = std::numeric_limits<double>::max();
            intersectionInformationBuffer->position = {0, 0, 0};
            auto *rightChild = (Child *) (node->rightChild);
            rightChild->object->intersectFirst(intersectionInformationBuffer, ray);
            if (intersectionInformationBuffer->hit) {
                intersectionInfo->push_back(intersectionInformationBuffer);
            } else {
                delete intersectionInformationBuffer;
            }
        }
        if (node->leftChild->getType() == 0) {
            auto *leftChild = (InnerNode *) node->leftChild;
            if (rayBoxIntersection(&(leftChild->boundingBox.minCorner),
                                   &(leftChild->boundingBox.maxCorner), ray, &distanceLeft)) {
                stack[stackPointer++] = node->leftChild;
            }
        } else {
            auto *intersectionInformationBuffer = new IntersectionInfo();
            intersectionInformationBuffer->hit = false;
            intersectionInformationBuffer->distance = std::numeric_limits<double>::max();
            intersectionInformationBuffer->position = {0, 0, 0};
            auto *leftChild = (Child *) (node->leftChild);
            leftChild->object->intersectFirst(intersectionInformationBuffer, ray);
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

bool DBVH::traverseFirst(IntersectionInfo *intersectionInfo, Ray *ray) {
    bool hit = false;

    auto *stack = new TraversalContainer[maxDepth];
    uint64_t stackPointer = 1;
    stack[0] = {root, 0};

    while (stackPointer != 0) {
        if (stack[stackPointer - 1].distance >= intersectionInfo->distance) {
            stackPointer--;
            continue;
        }
        auto *node = (InnerNode *) (stack[stackPointer - 1].node);
        stackPointer--;

        double distanceRight = 0;
        double distanceLeft = 0;
        bool right = false;
        bool left = false;

        if (node->rightChild->getType() == 0) {
            auto *rightChild = (InnerNode *) node->rightChild;
            right = rayBoxIntersection(&(rightChild->boundingBox.minCorner),
                                       &(rightChild->boundingBox.maxCorner), ray, &distanceRight);
        } else {
            IntersectionInfo intersectionInformationBuffer{};
            intersectionInformationBuffer.hit = false;
            intersectionInformationBuffer.distance = std::numeric_limits<double>::max();
            intersectionInformationBuffer.position = {0, 0, 0};
            auto *rightChild = (Child *) (node->rightChild);
            rightChild->object->intersectFirst(&intersectionInformationBuffer, ray);
            if (intersectionInformationBuffer.hit) {
                if (intersectionInformationBuffer.distance < intersectionInfo->distance) {
                    *intersectionInfo = intersectionInformationBuffer;
                    hit = true;
                }
            }
        }
        if (node->leftChild->getType() == 0) {
            auto *leftChild = (InnerNode *) node->leftChild;
            left = rayBoxIntersection(&(leftChild->boundingBox.minCorner),
                                      &(leftChild->boundingBox.maxCorner), ray, &distanceLeft);
        } else {
            IntersectionInfo intersectionInformationBuffer{};
            intersectionInformationBuffer.hit = false;
            intersectionInformationBuffer.distance = std::numeric_limits<double>::max();
            intersectionInformationBuffer.position = {0, 0, 0};
            auto *leftChild = (Child *) (node->leftChild);
            leftChild->object->intersectFirst(&intersectionInformationBuffer, ray);
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

bool DBVH::traverseAny(IntersectionInfo *intersectionInfo, Ray *ray) {
    Node **stack = new Node *[maxDepth];
    uint64_t stackPointer = 1;
    stack[0] = root;

    while (stackPointer != 0) {
        auto *node = (InnerNode *) (stack[stackPointer - 1]);
        stackPointer--;

        double distanceRight = 0;
        double distanceLeft = 0;

        if (node->rightChild->getType() == 0) {
            auto *rightChild = (InnerNode *) node->rightChild;
            if (rayBoxIntersection(&(rightChild->boundingBox.minCorner),
                                   &(rightChild->boundingBox.maxCorner), ray, &distanceRight)) {
                stack[stackPointer++] = node->rightChild;
            }
        } else {
            IntersectionInfo intersectionInformationBuffer{};
            intersectionInformationBuffer.hit = false;
            intersectionInformationBuffer.distance = std::numeric_limits<double>::max();
            intersectionInformationBuffer.position = {0, 0, 0};
            auto *rightChild = (Child *) (node->rightChild);
            rightChild->object->intersectFirst(&intersectionInformationBuffer, ray);
            if (intersectionInformationBuffer.hit) {
                *intersectionInfo = intersectionInformationBuffer;
                delete[] stack;
                return true;
            }
        }
        if (node->leftChild->getType() == 0) {
            auto *leftChild = (InnerNode *) node->leftChild;
            if (rayBoxIntersection(&(leftChild->boundingBox.minCorner),
                                   &(leftChild->boundingBox.maxCorner), ray, &distanceLeft)) {
                stack[stackPointer++] = node->leftChild;
            }
        } else {
            IntersectionInfo intersectionInformationBuffer{};
            intersectionInformationBuffer.hit = false;
            intersectionInformationBuffer.distance = std::numeric_limits<double>::max();
            intersectionInformationBuffer.position = {0, 0, 0};
            auto *leftChild = (Child *) (node->leftChild);
            leftChild->object->intersectFirst(&intersectionInformationBuffer, ray);
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

DBVH::DBVH() {
    root = nullptr;
    maxDepth = 0;
    instance = false;
}

DBVH::DBVH(const DBVH &clone) {
    root = clone.root;
    maxDepth = clone.maxDepth;
    instance = true;
}

DBVH::~DBVH() {
    if (!instance) {
        delete root;
    }
}

void DBVH::addObjects(std::vector<Object *> *objects) {
    if (objects->empty()) return;
    if (root == nullptr) {
        if (objects->size() == 1) {
            root = new Child();
            ((Child *) root)->object = objects->at(0);
            return;
        } else {
            root = new InnerNode();
        }
    } else if (root->getType() == 1) {
        objects->push_back(((Child *) (root))->object);
        ((Child *) root)->object = nullptr;
        delete root;
        root = new InnerNode();
    }
    add(root, objects, 1);
}

void DBVH::removeObjects(std::vector<Object *> *objects) {
    remove(&root, objects);
}

BoundingBox DBVH::getBoundaries() {
    return root->getBoundingBox();
}

bool DBVH::intersectFirst(IntersectionInfo *intersectionInfo, Ray *ray) {
    bool hit;

    if (root == nullptr) return false;

    if (root->getType() == 1) {
        auto *node = (Child *) (root);
        hit = node->object->intersectFirst(intersectionInfo, ray);
    } else {
        hit = traverseFirst(intersectionInfo, ray);
    }

    return hit;
}

bool DBVH::intersectAny(IntersectionInfo *intersectionInfo, Ray *ray) {
    bool hit;

    if (root == nullptr) return false;

    if (root->getType() == 1) {
        auto *node = (Child *) (root);
        hit = node->object->intersectAny(intersectionInfo, ray);
    } else {
        hit = traverseAny(intersectionInfo, ray);
    }

    return hit;
}

bool DBVH::intersectAll(std::vector<IntersectionInfo *> *intersectionInfo, Ray *ray) {
    bool hit;

    if (root == nullptr) return false;

    if (root->getType() == 1) {
        auto *node = (Child *) (root);
        hit = node->object->intersectAll(intersectionInfo, ray);
    } else {
        hit = traverseALl(intersectionInfo, ray);
    }

    return hit;
}

double DBVH::getSurfaceArea() {
    if (root->getType() == 0) {
        return ((InnerNode *) root)->surfaceArea;
    } else {
        return ((Child *) root)->object->getSurfaceArea();
    }
}

bool DBVH::operator==(Object *object) {
    auto obj = dynamic_cast<DBVH *>(object);
    if (obj == nullptr) {
        return false;
    } else {
        return obj->root == root;
    }
}

Object *DBVH::clone() {
    return new DBVH(*this);
}
