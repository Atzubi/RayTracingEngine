//
// Created by Sebastian on 13.12.2021.
//

#include <algorithm>
#include <limits>
#include "DBVHv2.h"

namespace {
    constexpr int numberOfSplittingPlanes = 9;

    bool isEmpty(const DBVHNode &root) {
        return root.maxDepthLeft == 0;
    }

    bool isLastElement(const DBVHNode &root) {
        return root.maxDepthRight == 0 && !isEmpty(root);
    }

    bool isLeafLeft(const DBVHNode &root) {
        return root.maxDepthLeft == 1;
    }

    bool isLeafRight(const DBVHNode &root) {
        return root.maxDepthRight == 1;
    }

    bool isNodeLeft(const DBVHNode &root) {
        return root.maxDepthLeft > 1;
    }

    bool isNodeRight(const DBVHNode &root) {
        return root.maxDepthRight > 1;
    }

    bool isEmptyRight(const DBVHNode &root) {
        return root.maxDepthRight == 0;
    }

    bool isEmptyLeft(const DBVHNode &root) {
        return isEmpty(root);
    }

    enum SplitOperation {
        Default, DefaultWrongOrder, AllNewLeft, AllNewRight, SplitOldNew, DefaultOldLeft, DefaultWrongOrderOldLeft
    };

    void refit(BoundingBox &target, BoundingBox resizeBy) {
        target.minCorner.x = std::min(target.minCorner.x, resizeBy.minCorner.x);
        target.minCorner.y = std::min(target.minCorner.y, resizeBy.minCorner.y);
        target.minCorner.z = std::min(target.minCorner.z, resizeBy.minCorner.z);
        target.maxCorner.x = std::max(target.maxCorner.x, resizeBy.maxCorner.x);
        target.maxCorner.y = std::max(target.maxCorner.y, resizeBy.maxCorner.y);
        target.maxCorner.z = std::max(target.maxCorner.z, resizeBy.maxCorner.z);
    }

    void refit(BoundingBox &aabb, const Object &object) {
        refit(aabb, object.getBoundaries());
    }

    void refit(BoundingBox &aabb, const std::vector<Object *> &objects, double looseness) {
        // refit box to fit all objects
        for (const auto &object: objects) {
            refit(aabb, *object);
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

    int getCurrentSplittingPlane(const Vector3D &splittingPlane) {
        for (int i = 0; i < 3; i++) {
            if (splittingPlane[i] != 0) {
                return i;
            }
        }
        throw std::invalid_argument("The given vector does not define a splitting plane");
    }

    bool isObjectInLeftSplit(const Vector3D &splittingPlane, int currentSplittingPlane, Object *const &object) {
        return (object->getBoundaries().maxCorner[currentSplittingPlane] +
                object->getBoundaries().minCorner[currentSplittingPlane]) / 2 <
               splittingPlane[currentSplittingPlane];
    }

    void
    sortObjectsIntoBuckets(const std::vector<Object *> &objects, const Vector3D &splittingPlane, BoundingBox &aabbLeft,
                           BoundingBox &aabbRight, double &objectCostLeft, double &objectCostRight) {
        int currentSplittingPlane = getCurrentSplittingPlane(splittingPlane);

        for (const auto &object: objects) {
            if (isObjectInLeftSplit(splittingPlane, currentSplittingPlane, object)) {
                refit(aabbLeft, *object);
                objectCostLeft += object->getSurfaceArea();
            } else {
                refit(aabbRight, *object);
                objectCostRight += object->getSurfaceArea();
            }
        }
    }

    double computeSAH(BoundingBox &aabbLeft, BoundingBox &aabbRight, double objectCostLeft, double objectCostRight) {
        BoundingBox combined;
        refit(combined, aabbLeft);
        refit(combined, aabbRight);

        double pLeft = aabbLeft.getSA() / combined.getSA();
        double pRight = aabbRight.getSA() / combined.getSA();

        return pLeft * objectCostLeft + pRight * objectCostRight;
    }

    void setBoxesAndHitProbability(const DBVHNode &node, BoundingBox &leftChildBox, BoundingBox &rightChildBox,
                                   double &pLeft,
                                   double &pRight) {
        pLeft = 0;
        pRight = 0;
        if (isNodeLeft(node)) {
            leftChildBox = (node.leftChild)->boundingBox;
            pLeft = (node.leftChild)->surfaceArea / leftChildBox.getSA();
        } else {
            leftChildBox = (node.leftLeaf)->getBoundaries();
            pLeft = (node.leftLeaf)->getSurfaceArea() / leftChildBox.getSA();
        }
        if (isNodeRight(node)) {
            rightChildBox = (node.rightChild)->boundingBox;
            pRight = (node.rightChild)->surfaceArea / rightChildBox.getSA();
        } else {
            rightChildBox = (node.rightLeaf)->getBoundaries();
            pRight = (node.rightLeaf)->getSurfaceArea() / rightChildBox.getSA();
        }
    }

    SplitOperation getBestSplitOperation(const double *SAHs) {
        double SAH = std::numeric_limits<double>::max();
        SplitOperation bestSAH = Default;
        for (int i = 0; i < 7; i++) {
            if (SAHs[i] < SAH) {
                SAH = SAHs[i];
                bestSAH = static_cast<SplitOperation>(i);
            }
        }
        return bestSAH;
    }

    double computeSAHWithNewParent(const DBVHNode &node, const BoundingBox &aabbLeft, const BoundingBox &aabbRight,
                                   double objectCostLeft, double objectCostRight, SplitOperation &newParent) {
        BoundingBox leftChildBox;
        BoundingBox rightChildBox;
        double pLeft;
        double pRight;
        setBoxesAndHitProbability(node, leftChildBox, rightChildBox, pLeft, pRight);

        BoundingBox oldLeft = leftChildBox;
        BoundingBox oldLeftNewLeft = leftChildBox;
        BoundingBox oldLeftNewRight = leftChildBox;
        BoundingBox oldLeftNewLeftNewRight = leftChildBox;
        BoundingBox oldLeftOldRight = leftChildBox;
        BoundingBox oldLeftOldRightNewLeft = leftChildBox;
        BoundingBox oldLeftOldRightNewRight = leftChildBox;
        BoundingBox oldRightNewLeftNewRight = rightChildBox;
        BoundingBox oldRightNewRight = rightChildBox;
        BoundingBox oldRightNewLeft = rightChildBox;
        BoundingBox oldRight = rightChildBox;
        BoundingBox newLeftNewRight = aabbLeft;
        BoundingBox newRight = aabbRight;
        BoundingBox newLeft = aabbLeft;

        refit(newLeftNewRight, newRight);
        refit(oldLeftNewLeft, newLeft);
        refit(oldLeftNewRight, newRight);
        refit(oldLeftNewLeftNewRight, newLeftNewRight);
        refit(oldLeftOldRight, oldRight);
        refit(oldRightNewLeft, newLeft);
        refit(oldRightNewRight, newRight);
        refit(oldLeftOldRightNewLeft, oldRightNewLeft);
        refit(oldLeftOldRightNewRight, oldRightNewRight);
        refit(oldRightNewLeftNewRight, newLeftNewRight);

        double SAHs[7];

        SAHs[Default] = oldLeftNewLeft.getSA() * (objectCostLeft + pLeft) +
                        oldRightNewRight.getSA() * (objectCostRight + pRight);
        SAHs[DefaultWrongOrder] = oldLeftNewRight.getSA() * (objectCostRight + pLeft) +
                                  oldRightNewLeft.getSA() * (objectCostLeft + pRight);
        SAHs[AllNewLeft] = oldLeft.getSA() * pLeft +
                           oldRightNewLeftNewRight.getSA() * (objectCostLeft + objectCostRight + pRight);
        SAHs[AllNewRight] = oldLeftNewLeftNewRight.getSA() * (objectCostLeft + objectCostRight + pLeft) +
                            oldRight.getSA() * pRight;
        SAHs[SplitOldNew] = oldLeftOldRight.getSA() * (pLeft + pRight) +
                            newLeftNewRight.getSA() * (objectCostLeft + objectCostRight);
        SAHs[DefaultOldLeft] = oldLeftOldRightNewLeft.getSA() * (objectCostLeft + pLeft + pRight) +
                               newRight.getSA() * objectCostRight;
        SAHs[DefaultWrongOrderOldLeft] = oldLeftOldRightNewRight.getSA() * (objectCostRight + pLeft + pRight) +
                                         newLeft.getSA() * objectCostLeft;


        SplitOperation bestSAH = getBestSplitOperation(SAHs);

        newParent = bestSAH;
        return SAHs[bestSAH];
    }

    double evaluateBucket(const DBVHNode &node, const std::vector<Object *> &objects, const Vector3D &splittingPlane,
                          SplitOperation &newParent) {
        BoundingBox aabbLeft;
        BoundingBox aabbRight;

        double objectCostLeft = 0;
        double objectCostRight = 0;

        sortObjectsIntoBuckets(objects, splittingPlane, aabbLeft, aabbRight, objectCostLeft, objectCostRight);

        if (!isEmptyLeft(node) && !isEmptyRight(node)) {
            return computeSAHWithNewParent(node, aabbLeft, aabbRight, objectCostLeft, objectCostRight, newParent);
        } else {
            return computeSAH(aabbLeft, aabbRight, objectCostLeft, objectCostRight);

        }
    }

    void split(std::vector<Object *> &leftChild, std::vector<Object *> &rightChild,
               const std::vector<Object *> &objects, const Vector3D &splittingPlane) {
        int currentSplittingPlane = getCurrentSplittingPlane(splittingPlane);

        for (const auto &object: objects) {
            if (isObjectInLeftSplit(splittingPlane, currentSplittingPlane, object)) {
                leftChild.push_back(object);
            } else {
                rightChild.push_back(object);
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

    bool rayBoxIntersection(const Vector3D &min, const Vector3D &max, const Ray &ray, double &distance) {
        double t1 = (min.x - ray.origin.x) * ray.dirfrac.x;
        double t2 = (max.x - ray.origin.x) * ray.dirfrac.x;
        double t3 = (min.y - ray.origin.y) * ray.dirfrac.y;
        double t4 = (max.y - ray.origin.y) * ray.dirfrac.y;
        double t5 = (min.z - ray.origin.z) * ray.dirfrac.z;
        double t6 = (max.z - ray.origin.z) * ray.dirfrac.z;

        double tMin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
        double tMax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

        distance = tMin;
        return tMax >= 0 && tMin <= tMax;
    }

    void refitChild(DBVHNode &node, DBVHNode &child){
        refit(node.boundingBox, child.boundingBox);
        node.surfaceArea = child.surfaceArea;
        node.maxDepthRight = std::max(child.maxDepthRight, child.maxDepthLeft) + 1;
    }

    void refitLeaf(DBVHNode &node, Object &leaf){
        refit(node.boundingBox, leaf.getBoundaries());
        node.surfaceArea = leaf.getSurfaceArea();
        node.maxDepthRight = 1;
    }

    void refit(DBVHNode &node) {
        node.boundingBox = BoundingBox();
        if (isNodeRight(node)) {
            refitChild(node, *node.rightChild);
        } else {
            refitLeaf(node, *node.rightLeaf);
        }
        if (isNodeLeft(node)) {
            refitChild(node, *node.leftChild);
        } else {
            refitLeaf(node, *node.leftLeaf);
        }
        node.surfaceArea += node.boundingBox.getSA();
    }

    bool optimizeSAH(DBVHNode &node) {
        int bestSAH = 0;
        double SAHs[5];

        BoundingBox leftBox{}, rightBox{}, leftLeftBox{}, leftRightBox{}, rightLeftBox{}, rightRightBox{};
        double leftSA = 0, rightSA = 0, leftLeftSA = 0, leftRightSA = 0, rightLeftSA = 0, rightRightSA = 0;

        BoundingBox swapLeftLeftToRight{}, swapLeftRightToRight{}, swapRightLeftToLeft{}, swapRightRightToLeft{};

        SAHs[0] = node.surfaceArea;

        if (isNodeLeft(node)) {
            auto leftNode = node.leftChild.get();
            leftBox = leftNode->boundingBox;
            leftSA = leftNode->surfaceArea;
            if (isNodeLeft(*leftNode)) {
                leftLeftBox = (leftNode->leftChild)->boundingBox;
                leftLeftSA = (leftNode->leftChild)->surfaceArea;
            } else {
                leftLeftBox = (leftNode->leftLeaf)->getBoundaries();
                leftLeftSA = (leftNode->leftLeaf)->getSurfaceArea();
            }
            if (isNodeRight(*leftNode)) {
                leftRightBox = (leftNode->rightChild)->boundingBox;
                leftRightSA = (leftNode->rightChild)->surfaceArea;
            } else {
                leftRightBox = (leftNode->rightLeaf)->getBoundaries();
                leftRightSA = (leftNode->rightLeaf)->getSurfaceArea();
            }

            if (isNodeRight(node)) {
                auto rightNode = node.rightChild.get();
                rightBox = rightNode->boundingBox;
                rightSA = rightNode->surfaceArea;
                if (isNodeLeft(*rightNode)) {
                    rightLeftBox = (rightNode->leftChild)->boundingBox;
                    rightLeftSA = (rightNode->leftChild)->surfaceArea;
                } else {
                    rightLeftBox = (rightNode->leftLeaf)->getBoundaries();
                    rightLeftSA = (rightNode->leftLeaf)->getSurfaceArea();
                }
                if (isNodeRight(*rightNode)) {
                    rightRightBox = (rightNode->rightChild)->boundingBox;
                    rightRightSA = (rightNode->rightChild)->surfaceArea;
                } else {
                    rightRightBox = (rightNode->rightLeaf)->getBoundaries();
                    rightRightSA = (rightNode->rightLeaf)->getSurfaceArea();
                }

                swapLeftLeftToRight = {{std::min(rightBox.minCorner.x, leftRightBox.minCorner.x),
                                               std::min(rightBox.minCorner.y, leftRightBox.minCorner.y),
                                               std::min(rightBox.minCorner.z, leftRightBox.minCorner.z)},
                                       {std::max(rightBox.maxCorner.x, leftRightBox.maxCorner.x),
                                               std::max(rightBox.maxCorner.y, leftRightBox.maxCorner.y),
                                               std::max(rightBox.maxCorner.z, leftRightBox.maxCorner.z)}};
                swapLeftRightToRight = {{std::min(rightBox.minCorner.x, leftLeftBox.minCorner.x),
                                                std::min(rightBox.minCorner.y, leftLeftBox.minCorner.y),
                                                std::min(rightBox.minCorner.z, leftLeftBox.minCorner.z)},
                                        {std::max(rightBox.maxCorner.x, leftLeftBox.maxCorner.x),
                                                std::max(rightBox.maxCorner.y, leftLeftBox.maxCorner.y),
                                                std::max(rightBox.maxCorner.z, leftLeftBox.maxCorner.z)}};
                swapRightLeftToLeft = {{std::min(leftBox.minCorner.x, rightRightBox.minCorner.x),
                                               std::min(leftBox.minCorner.y, rightRightBox.minCorner.y),
                                               std::min(leftBox.minCorner.z, rightRightBox.minCorner.z)},
                                       {std::max(leftBox.maxCorner.x, rightRightBox.maxCorner.x),
                                               std::max(leftBox.maxCorner.y, rightRightBox.maxCorner.y),
                                               std::max(leftBox.maxCorner.z, rightRightBox.maxCorner.z)}};
                swapRightRightToLeft = {{std::min(leftBox.minCorner.x, rightLeftBox.minCorner.x),
                                                std::min(leftBox.minCorner.y, rightLeftBox.minCorner.y),
                                                std::min(leftBox.minCorner.z, rightLeftBox.minCorner.z)},
                                        {std::max(leftBox.maxCorner.x, rightLeftBox.maxCorner.x),
                                                std::max(leftBox.maxCorner.y, rightLeftBox.maxCorner.y),
                                                std::max(leftBox.maxCorner.z, rightLeftBox.maxCorner.z)}};

                SAHs[1] =
                        node.boundingBox.getSA() + leftLeftSA + rightSA + leftRightSA + swapLeftLeftToRight.getSA();
                SAHs[2] =
                        node.boundingBox.getSA() + leftRightSA + rightSA + leftLeftSA + swapLeftRightToRight.getSA();
                SAHs[3] =
                        node.boundingBox.getSA() + rightLeftSA + leftSA + rightRightSA + swapRightLeftToLeft.getSA();
                SAHs[4] = node.boundingBox.getSA() + rightRightSA + leftSA + rightLeftSA +
                          swapRightRightToLeft.getSA();
            } else {
                auto rightNode = node.rightLeaf;
                rightBox = rightNode->getBoundaries();
                rightSA = rightNode->getSurfaceArea();

                swapLeftLeftToRight = {{std::min(rightBox.minCorner.x, leftRightBox.minCorner.x),
                                               std::min(rightBox.minCorner.y, leftRightBox.minCorner.y),
                                               std::min(rightBox.minCorner.z, leftRightBox.minCorner.z)},
                                       {std::max(rightBox.maxCorner.x, leftRightBox.maxCorner.x),
                                               std::max(rightBox.maxCorner.y, leftRightBox.maxCorner.y),
                                               std::max(rightBox.maxCorner.z, leftRightBox.maxCorner.z)}};
                swapLeftRightToRight = {{std::min(rightBox.minCorner.x, leftLeftBox.minCorner.x),
                                                std::min(rightBox.minCorner.y, leftLeftBox.minCorner.y),
                                                std::min(rightBox.minCorner.z, leftLeftBox.minCorner.z)},
                                        {std::max(rightBox.maxCorner.x, leftLeftBox.maxCorner.x),
                                                std::max(rightBox.maxCorner.y, leftLeftBox.maxCorner.y),
                                                std::max(rightBox.maxCorner.z, leftLeftBox.maxCorner.z)}};

                SAHs[1] =
                        node.boundingBox.getSA() + leftLeftSA + rightSA + leftRightSA + swapLeftLeftToRight.getSA();
                SAHs[2] =
                        node.boundingBox.getSA() + leftRightSA + rightSA + leftLeftSA + swapLeftRightToRight.getSA();
                SAHs[3] = std::numeric_limits<double>::max();
                SAHs[4] = std::numeric_limits<double>::max();
            }
        } else {
            auto leftNode = node.leftLeaf;
            leftBox = leftNode->getBoundaries();
            leftSA = leftNode->getSurfaceArea();

            if (isNodeRight(node)) {
                auto &rightNode = node.rightChild;
                rightBox = rightNode->boundingBox;
                rightSA = rightNode->surfaceArea;
                if (isNodeLeft(*rightNode)) {
                    rightLeftBox = (rightNode->leftChild)->boundingBox;
                    rightLeftSA = (rightNode->leftChild)->surfaceArea;
                } else {
                    rightLeftBox = (rightNode->leftLeaf)->getBoundaries();
                    rightLeftSA = (rightNode->leftLeaf)->getSurfaceArea();
                }
                if (isNodeRight(*rightNode)) {
                    rightRightBox = (rightNode->rightChild)->boundingBox;
                    rightRightSA = (rightNode->rightChild)->surfaceArea;
                } else {
                    rightRightBox = (rightNode->rightLeaf)->getBoundaries();
                    rightRightSA = (rightNode->rightLeaf)->getSurfaceArea();
                }

                swapRightLeftToLeft = {{std::min(leftBox.minCorner.x, rightRightBox.minCorner.x),
                                               std::min(leftBox.minCorner.y, rightRightBox.minCorner.y),
                                               std::min(leftBox.minCorner.z, rightRightBox.minCorner.z)},
                                       {std::max(leftBox.maxCorner.x, rightRightBox.maxCorner.x),
                                               std::max(leftBox.maxCorner.y, rightRightBox.maxCorner.y),
                                               std::max(leftBox.maxCorner.z, rightRightBox.maxCorner.z)}};
                swapRightRightToLeft = {{std::min(leftBox.minCorner.x, rightLeftBox.minCorner.x),
                                                std::min(leftBox.minCorner.y, rightLeftBox.minCorner.y),
                                                std::min(leftBox.minCorner.z, rightLeftBox.minCorner.z)},
                                        {std::max(leftBox.maxCorner.x, rightLeftBox.maxCorner.x),
                                                std::max(leftBox.maxCorner.y, rightLeftBox.maxCorner.y),
                                                std::max(leftBox.maxCorner.z, rightLeftBox.maxCorner.z)}};

                SAHs[1] = std::numeric_limits<double>::max();
                SAHs[2] = std::numeric_limits<double>::max();
                SAHs[3] =
                        node.boundingBox.getSA() + rightLeftSA + leftSA + rightRightSA + swapRightLeftToLeft.getSA();
                SAHs[4] = node.boundingBox.getSA() + rightRightSA + leftSA + rightLeftSA +
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
                if (isNodeRight(node)) {
                    auto buffer = std::move(node.rightChild);
                    if (isNodeLeft(*node.leftChild)) {
                        node.rightChild = std::move((node.leftChild)->leftChild);
                        node.maxDepthRight =
                                std::max(node.rightChild->maxDepthLeft, node.rightChild->maxDepthRight) + 1;
                    } else {
                        using namespace std;
                        node.rightLeaf = (node.leftChild)->leftLeaf;
                        node.maxDepthRight = 1;
                    }
                    (node.leftChild)->boundingBox = swapLeftLeftToRight;
                    (node.leftChild)->leftChild.reset();

                    (node.leftChild)->maxDepthLeft = std::max(buffer->maxDepthLeft, buffer->maxDepthRight) + 1;
                    (node.leftChild)->leftChild = std::move(buffer);


                    node.surfaceArea = SAHs[1];
                    (node.leftChild)->surfaceArea = rightSA + leftRightSA + swapLeftLeftToRight.getSA();
                    return true;
                } else {
                    auto buffer = node.rightLeaf;
                    if (isNodeLeft(*node.leftChild)) {
                        node.rightChild = std::move((node.leftChild)->leftChild);
                        node.maxDepthRight =
                                std::max(node.rightChild->maxDepthLeft, node.rightChild->maxDepthRight) + 1;
                    } else {
                        node.rightLeaf = (node.leftChild)->leftLeaf;
                        node.maxDepthRight = 1;
                    }
                    (node.leftChild)->boundingBox = swapLeftLeftToRight;
                    (node.leftChild)->leftLeaf = buffer;
                    (node.leftChild)->maxDepthLeft = 1;

                    node.surfaceArea = SAHs[1];
                    (node.leftChild)->surfaceArea = rightSA + leftRightSA + swapLeftLeftToRight.getSA();
                    return true;
                }
            }
            case 2: {
                if (isNodeRight(node)) {
                    auto buffer = std::move(node.rightChild);
                    if (isNodeRight(*node.leftChild)) {
                        node.rightChild = std::move((node.leftChild)->rightChild);
                        node.maxDepthRight =
                                std::max(node.rightChild->maxDepthLeft, node.rightChild->maxDepthRight) + 1;
                    } else {
                        node.rightLeaf = (node.leftChild)->rightLeaf;
                        node.maxDepthRight = 1;
                    }
                    (node.leftChild)->boundingBox = swapLeftRightToRight;
                    (node.leftChild)->maxDepthRight = std::max(buffer->maxDepthLeft, buffer->maxDepthRight) + 1;
                    (node.leftChild)->rightChild = std::move(buffer);

                    node.surfaceArea = SAHs[2];
                    (node.leftChild)->surfaceArea = rightSA + leftLeftSA + swapLeftRightToRight.getSA();
                    return true;
                } else {
                    auto buffer = node.rightLeaf;
                    if (isNodeRight(*node.leftChild)) {
                        node.rightChild = std::move((node.leftChild)->rightChild);
                        node.maxDepthRight =
                                std::max(node.rightChild->maxDepthLeft, node.rightChild->maxDepthRight) + 1;
                    } else {
                        node.rightLeaf = (node.leftChild)->rightLeaf;
                        node.maxDepthRight = 1;
                    }
                    (node.leftChild)->boundingBox = swapLeftRightToRight;
                    (node.leftChild)->rightLeaf = buffer;
                    (node.leftChild)->maxDepthRight = 1;

                    node.surfaceArea = SAHs[2];
                    (node.leftChild)->surfaceArea = rightSA + leftLeftSA + swapLeftRightToRight.getSA();
                    return true;
                }
            }
            case 3: {
                if (isNodeLeft(node)) {
                    auto buffer = std::move(node.leftChild);
                    if (isNodeLeft(*node.rightChild)) {
                        node.leftChild = std::move((node.rightChild)->leftChild);
                        node.maxDepthLeft = std::max(node.leftChild->maxDepthLeft, node.leftChild->maxDepthRight) + 1;
                    } else {
                        node.leftLeaf = (node.rightChild)->leftLeaf;
                        node.maxDepthLeft = 1;
                    }
                    (node.rightChild)->boundingBox = swapRightLeftToLeft;
                    (node.rightChild)->maxDepthLeft = std::max(buffer->maxDepthLeft, buffer->maxDepthRight) + 1;
                    (node.rightChild)->leftChild = std::move(buffer);

                    node.surfaceArea = SAHs[3];
                    (node.rightChild)->surfaceArea = leftSA + rightRightSA + swapRightLeftToLeft.getSA();
                    return true;
                } else {
                    auto buffer = node.leftLeaf;
                    if (isNodeLeft(*node.rightChild)) {
                        node.leftChild = std::move((node.rightChild)->leftChild);
                        node.maxDepthLeft = std::max(node.leftChild->maxDepthLeft, node.leftChild->maxDepthRight) + 1;
                    } else {
                        node.leftLeaf = (node.rightChild)->leftLeaf;
                        node.maxDepthLeft = 1;
                    }
                    (node.rightChild)->boundingBox = swapRightLeftToLeft;
                    (node.rightChild)->leftLeaf = buffer;
                    (node.rightChild)->maxDepthLeft = 1;

                    node.surfaceArea = SAHs[3];
                    (node.rightChild)->surfaceArea = leftSA + rightRightSA + swapRightLeftToLeft.getSA();
                    return true;
                }
            }
            case 4: {
                if (isNodeLeft(node)) {
                    auto buffer = std::move(node.leftChild);
                    if (isNodeRight(*node.rightChild)) {
                        node.leftChild = std::move((node.rightChild)->rightChild);
                        node.maxDepthLeft = std::max(node.leftChild->maxDepthLeft, node.leftChild->maxDepthRight) + 1;
                    } else {
                        node.leftLeaf = (node.rightChild)->rightLeaf;
                        node.maxDepthLeft = 1;
                    }
                    (node.rightChild)->boundingBox = swapRightRightToLeft;
                    (node.rightChild)->maxDepthRight = std::max(buffer->maxDepthLeft, buffer->maxDepthRight) + 1;
                    (node.rightChild)->rightChild = std::move(buffer);

                    node.surfaceArea = SAHs[4];
                    (node.rightChild)->surfaceArea = leftSA + rightLeftSA + swapRightRightToLeft.getSA();
                    return true;
                } else {
                    auto buffer = node.leftLeaf;
                    if (isNodeRight(*node.rightChild)) {
                        node.leftChild = std::move((node.rightChild)->rightChild);
                        node.maxDepthLeft = std::max(node.leftChild->maxDepthLeft, node.leftChild->maxDepthRight) + 1;
                    } else {
                        node.leftLeaf = (node.rightChild)->rightLeaf;
                        node.maxDepthLeft = 1;
                    }
                    (node.rightChild)->boundingBox = swapRightRightToLeft;
                    (node.rightChild)->rightLeaf = buffer;
                    (node.rightChild)->maxDepthRight = 1;

                    node.surfaceArea = SAHs[4];
                    (node.rightChild)->surfaceArea = leftSA + rightLeftSA + swapRightRightToLeft.getSA();
                    return true;
                }
            }
            default:
                node.surfaceArea = SAHs[0];
                return false;
        }
    }

    std::vector<Vector3D> createSplittingPlanes(DBVHNode &node) {
        std::vector<Vector3D> splittingPlanes(numberOfSplittingPlanes);
        int splitsPerDimension = numberOfSplittingPlanes / 3 + 1;

        for (int dim = 0; dim < 3; dim++) {
            double split = (node.boundingBox.maxCorner[dim] - node.boundingBox.minCorner[dim]) / splitsPerDimension;
            for (int plane = 0; plane < numberOfSplittingPlanes / 3; plane++) {
                splittingPlanes[3 * dim + plane][dim] =
                        node.boundingBox.minCorner[dim] + split * (plane + 1) + std::numeric_limits<double>::min();
            }
        }

        return splittingPlanes;
    }

    std::vector<double> evaluateSplittingPlanes(const DBVHNode &node, const std::vector<Object *> &objects,
                                                const std::vector<Vector3D> &splittingPlanes,
                                                std::vector<SplitOperation> &newParent) {
        std::vector<double> SAH(numberOfSplittingPlanes);

        for (int i = 0; i < numberOfSplittingPlanes; i++) {
            SAH[i] = evaluateBucket(node, objects, splittingPlanes[i], newParent[i]);
        }

        return SAH;
    }

    int getBestSplittingPlane(const std::vector<double> &SAH) {
        double bestSAH = std::numeric_limits<double>::max();
        int bestSplittingPlane = -1;

        for (int i = 0; i < numberOfSplittingPlanes; i++) {
            if (SAH[i] < bestSAH) {
                bestSAH = SAH[i];
                bestSplittingPlane = i;
            }
        }

        return bestSplittingPlane;
    }

    void splitEven(const std::vector<Object *> &objects, std::vector<Object *> &left,
                   std::vector<Object *> &right) {
        left.reserve(objects.size() / 2);
        right.reserve((objects.size() + 1) / 2);
        for (uint64_t i = 0; i < objects.size() / 2; i++) {
            left.push_back(objects.at(i));
        }
        for (uint64_t i = objects.size() / 2; i < objects.size(); i++) {
            right.push_back(objects.at(i));
        }
    }

    bool bestSplittingPlaneExists(int bestSplittingPlane) {
        return bestSplittingPlane != -1;
    }

    void switchBoxOrder(std::vector<Object *> &leftObjects, std::vector<Object *> &rightObjects) {
        auto buffer = leftObjects;
        leftObjects = rightObjects;
        rightObjects = buffer;
    }


    std::unique_ptr<DBVHNode> moveToNewNode(DBVHNode &node) {
        auto newNode = std::make_unique<DBVHNode>();
        if (isNodeRight(node)) {
            newNode->rightChild = std::move(node.rightChild);
        } else {
            newNode->rightLeaf = node.rightLeaf;
        }
        if (isNodeLeft(node)) {
            newNode->leftChild = std::move(node.leftChild);
        } else {
            newNode->leftLeaf = node.leftLeaf;
        }
        refit(*newNode);
        return newNode;
    }

    void moveParentToNewParentsLeftChild(DBVHNode( &node)) {
        auto newNode = moveToNewNode(node);
        node.maxDepthLeft = std::max(newNode->maxDepthLeft, newNode->maxDepthRight) + 1;
        node.leftChild = std::move(newNode);
        node.rightChild = nullptr;
        node.maxDepthRight = 0;
    }

    void
    sortObjectsIntoBoxes(const SplitOperation splitOperation, const Vector3D &splittingPlane, DBVHNode &node,
                         const std::vector<Object *> &objects,
                         std::vector<Object *> &leftObjects,
                         std::vector<Object *> &rightObjects) {
        switch (splitOperation) {
            case Default:
                split(leftObjects, rightObjects, objects, splittingPlane);
                break;
            case DefaultWrongOrder: {
                split(leftObjects, rightObjects, objects, splittingPlane);
                switchBoxOrder(leftObjects, rightObjects);
                break;
            }
            case AllNewLeft: {
                leftObjects = objects;
                break;
            }
            case AllNewRight: {
                rightObjects = objects;
                break;
            }
            case SplitOldNew: {
                moveParentToNewParentsLeftChild(node);
                rightObjects = objects;
                break;
            }
            case DefaultOldLeft: {
                split(leftObjects, rightObjects, objects, splittingPlane);
                moveParentToNewParentsLeftChild(node);
                break;
            }
            case DefaultWrongOrderOldLeft: {
                split(leftObjects, rightObjects, objects, splittingPlane);
                switchBoxOrder(leftObjects, rightObjects);
                moveParentToNewParentsLeftChild(node);
                break;
            }
            default:
                throw (std::out_of_range("Undefined split operation!"));
        }
    }

    void createLeftLeaf(DBVHNode &node, const std::vector<Object *> &leftObjects) {
        node.leftLeaf = leftObjects.at(0);
        node.maxDepthLeft = 1;
    }

    void createLeftChild(DBVHNode &node) {
        auto child = std::make_unique<DBVHNode>();
        node.leftChild = std::move(child);
        node.maxDepthLeft = 2;
    }

    void
    createNewParentForLeftLeafs(DBVHNode &node, const std::vector<Object *> &leftObjects) {
        auto buffer = node.leftLeaf;
        auto parent = std::make_unique<DBVHNode>();
        parent->boundingBox = buffer->getBoundaries();
        refit(parent->boundingBox, leftObjects, 0);
        parent->leftLeaf = buffer;
        parent->rightLeaf = leftObjects.at(0);
        parent->maxDepthLeft = 1;
        parent->maxDepthRight = 1;
        node.leftChild = std::move(parent);
        node.maxDepthLeft = 2;
    }

    void createNewParentForLeftChildren(DBVHNode &node) {
        auto buffer = node.leftLeaf;
        auto parent = std::make_unique<DBVHNode>();
        parent->leftLeaf = buffer;
        parent->boundingBox = buffer->getBoundaries();
        parent->surfaceArea = parent->boundingBox.getSA() * 2;
        parent->maxDepthLeft = 1;
        node.leftChild = std::move(parent);
        node.maxDepthLeft = 2;
    }

    bool insertSingleObjectLeft(DBVHNode &node, const std::vector<Object *> &rightObjects) {
        if (isEmptyRight(node)) {
            createLeftLeaf(node, rightObjects);
        } else if (isLeafRight(node)) {
            createNewParentForLeftLeafs(node, rightObjects);
        } else {
            return false;
        }
        return true;
    }

    void createChildNodeLeft(DBVHNode &node) {
        if (isEmptyRight(node)) {
            createLeftChild(node);
        } else if (isLeafRight(node)) {
            createNewParentForLeftChildren(node);
        }
    }

    bool passObjectsToLeftChild(DBVHNode &node, const std::vector<Object *> &leftObjects) {
        if (leftObjects.size() == 1) {
            return insertSingleObjectLeft(node, leftObjects);
        } else if (!leftObjects.empty()) {
            createChildNodeLeft(node);
            return false;
        }
        return true;
    }

    void createRightLeaf(DBVHNode &node, const std::vector<Object *> &rightObjects) {
        node.rightLeaf = rightObjects.at(0);
        node.maxDepthRight = 1;
    }

    void createRightChild(DBVHNode &node) {
        auto child = std::make_unique<DBVHNode>();
        node.rightChild = std::move(child);
        node.maxDepthRight = 2;
    }

    void createNewParentForRightLeafs(DBVHNode &node, const std::vector<Object *> &rightObjects) {
        auto buffer = node.rightLeaf;
        auto parent = std::make_unique<DBVHNode>();
        parent->boundingBox = buffer->getBoundaries();
        refit(parent->boundingBox, rightObjects, 0);
        parent->leftLeaf = buffer;
        parent->rightLeaf = rightObjects.at(0);
        parent->maxDepthLeft = 1;
        parent->maxDepthRight = 1;
        node.rightChild = std::move(parent);
        node.maxDepthRight = 2;
    }

    void createNewParentForRightChildren(DBVHNode &node) {
        auto buffer = node.rightLeaf;
        auto parent = std::make_unique<DBVHNode>();
        parent->rightLeaf = buffer;
        parent->boundingBox = buffer->getBoundaries();
        parent->surfaceArea = parent->boundingBox.getSA() * 2;
        parent->maxDepthRight = 1;
        node.rightChild = std::move(parent);
        node.maxDepthRight = 2;
    }

    bool
    insertSingleObjectRight(DBVHNode &node, const std::vector<Object *> &rightObjects) {
        if (isEmptyRight(node)) {
            createRightLeaf(node, rightObjects);
        } else if (isLeafRight(node)) {
            createNewParentForRightLeafs(node, rightObjects);
        } else {
            return false;
        }
        return true;
    }

    void createChildNodeRight(DBVHNode &node) {
        if (isEmptyRight(node)) {
            createRightChild(node);
        } else if (isLeafRight(node)) {
            createNewParentForRightChildren(node);
        }
    }

    bool
    passObjectsToRightChild(DBVHNode &node, const std::vector<Object *> &rightObjects) {
        if (rightObjects.size() == 1) {
            return insertSingleObjectRight(node, rightObjects);
        } else if (!rightObjects.empty()) {
            createChildNodeRight(node);
            return false;
        }
        return true;
    }

    void checkLeftChildSurfaceAreaAndDepth(DBVHNode &node) {
        if (isLeafLeft(node)) {
            node.surfaceArea += (node.leftLeaf)->getSurfaceArea();
        } else {
            node.surfaceArea += (node.leftChild)->surfaceArea;
            node.maxDepthLeft = std::max(node.leftChild->maxDepthLeft, node.leftChild->maxDepthRight) + 1;
        }
    }

    void checkRightChildSurfaceAreaAndDepth(DBVHNode &node) {
        if (isLeafRight(node)) {
            node.surfaceArea += (node.rightLeaf)->getSurfaceArea();
        } else {
            node.surfaceArea += (node.rightChild)->surfaceArea;
            node.maxDepthRight = std::max(node.rightChild->maxDepthLeft, node.rightChild->maxDepthRight) + 1;
        }
    }

    void setNodeSurfaceAreaAndDepth(DBVHNode &node) {
        node.surfaceArea = node.boundingBox.getSA();
        checkLeftChildSurfaceAreaAndDepth(node);
        checkRightChildSurfaceAreaAndDepth(node);
    }

    void add(DBVHNode &currentNode, const std::vector<Object *> &objects, const uint8_t depth) {
        // refit current currentNode to objects
        refit(currentNode.boundingBox, objects, 0);

        // create split buckets
        auto splittingPlanes = createSplittingPlanes(currentNode);

        std::vector<SplitOperation> newParent(numberOfSplittingPlanes);

        // evaluate split buckets
        auto SAH = evaluateSplittingPlanes(currentNode, objects, splittingPlanes, newParent);

        // choose best split bucket and split currentNode accordingly
        int bestSplittingPlane = getBestSplittingPlane(SAH);

        std::vector<Object *> leftObjects;
        std::vector<Object *> rightObjects;

        if (!bestSplittingPlaneExists(bestSplittingPlane)) {
            splitEven(objects, leftObjects, rightObjects);
        } else {
            sortObjectsIntoBoxes(newParent[bestSplittingPlane], splittingPlanes[bestSplittingPlane], currentNode,
                                 objects,
                                 leftObjects, rightObjects);
        }

        // pass objects to children
        if (!passObjectsToLeftChild(currentNode, leftObjects)) {
            add(*currentNode.leftChild, leftObjects, depth + 1);
        }
        if (!passObjectsToRightChild(currentNode, rightObjects)) {
            add(*currentNode.rightChild, rightObjects, depth + 1);
        }

        // calculate surface area and tree depth going the tree back up
        setNodeSurfaceAreaAndDepth(currentNode);

        // use tree rotations going the tree back up to optimize SAH
        optimizeSAH(currentNode);
    }

    bool removeRightLeftGrandChild(DBVHNode &currentNode, DBVHNode &child,
                                   const Object &object) {
        auto grandChild = child.leftLeaf;
        if (*grandChild != object) return false;

        if (isLeafRight(child)) {
            currentNode.rightLeaf = child.rightLeaf;
        } else {
            currentNode.rightChild = std::move(child.rightChild);
        }
        refit(currentNode);
        return true;
    }

    bool removeRightRightGrandChild(DBVHNode &currentNode, DBVHNode &child,
                                    const Object &object) {
        auto grandChild = child.rightLeaf;
        if (*grandChild != object) return false;

        if (isLeafLeft(child)) {
            currentNode.rightLeaf = child.leftLeaf;
        } else {
            currentNode.rightChild = std::move(child.leftChild);
        }
        refit(currentNode);
        return true;
    }

    bool removeRightLeaf(DBVHNode &currentNode, const Object &object) {
        if (isLeafRight(currentNode)) return true;
        auto child = std::move(currentNode.rightChild);
        if (!contains(child->boundingBox, object.getBoundaries())) return false;

        if (isLeafLeft(*child)) {
            return removeRightLeftGrandChild(currentNode, *child, object);
        }
        if (isLeafRight(*child)) {
            return removeRightRightGrandChild(currentNode, *child, object);
        }
        return false;
    }

    bool removeLeftLeftGrandChild(DBVHNode &currentNode, DBVHNode &child,
                                  const Object &object) {
        auto grandChild = child.leftLeaf;
        if (*grandChild != object) return false;

        if (isLeafRight(child)) {
            currentNode.leftLeaf = child.rightLeaf;
        } else {
            currentNode.leftChild = std::move(child.rightChild);
        }
        refit(currentNode);
        return true;
    }

    bool removeLeftRightGrandChild(DBVHNode &currentNode, DBVHNode &child,
                                   const Object &object) {
        auto grandChild = child.rightLeaf;
        if (*grandChild != object) return false;

        if (isLeafLeft(child)) {
            currentNode.leftLeaf = child.leftLeaf;
        } else {
            currentNode.leftChild = std::move(child.leftChild);
        }
        refit(currentNode);
        return true;
    }

    bool removeLeftLeaf(DBVHNode &currentNode, const Object &object) {
        if (isLeafLeft(currentNode)) return true;
        auto child = std::move(currentNode.leftChild);
        if (!contains(child->boundingBox, object.getBoundaries())) return false;

        if (isLeafLeft(*child)) {
            return removeLeftLeftGrandChild(currentNode, *child, object);
        }
        if (isLeafRight(*child)) {
            return removeLeftRightGrandChild(currentNode, *child, object);
        }
        return false;
    }

    void remove(DBVHNode &currentNode, const Object &object) {
        // find object in tree by insertion
        // remove object and refit nodes going the tree back up
        if (!contains(currentNode.boundingBox, object.getBoundaries())) return;
        if (removeLeftLeaf(currentNode, object) || removeRightLeaf(currentNode, object)) return;
        remove(*currentNode.leftChild, object);
        remove(*currentNode.rightChild, object);

        refit(currentNode);
        optimizeSAH(currentNode);
    }

    struct TraversalContainer {
        const DBVHNode *node;
        double distance;
    };

    void intersectLeaf(const Ray &ray, IntersectionInfo &intersectionInfo, Object &leaf, bool &hit) {
        IntersectionInfo intersectionInformationBuffer{};
        intersectionInformationBuffer.hit = false;
        intersectionInformationBuffer.distance = std::numeric_limits<double>::max();
        intersectionInformationBuffer.position = {0, 0, 0};
        leaf.intersectFirst(intersectionInformationBuffer, ray);
        if (intersectionInformationBuffer.hit) {
            if (intersectionInformationBuffer.distance < intersectionInfo.distance) {
                intersectionInfo = intersectionInformationBuffer;
                hit = true;
            }
        }
    }

    void getChildrenIntersections(const Ray &ray, const DBVHNode &node, IntersectionInfo &intersectionInfo,
                                  double &distanceRight, double &distanceLeft, bool &right, bool &left, bool &hit) {
        if (isNodeRight(node)) {
            // TODO request child if missing
            auto rightChild = node.rightChild.get();
            right = rayBoxIntersection(rightChild->boundingBox.minCorner, rightChild->boundingBox.maxCorner, ray,
                                       distanceRight);
        } else {
            // TODO request leaf if missing
            intersectLeaf(ray, intersectionInfo, *node.rightLeaf, hit);
        }
        if (isNodeLeft(node)) {
            // TODO request child if missing
            auto leftChild = node.leftChild.get();
            left = rayBoxIntersection(leftChild->boundingBox.minCorner, leftChild->boundingBox.maxCorner, ray,
                                      distanceLeft);
        } else {
            // TODO request leaf if missing
            intersectLeaf(ray, intersectionInfo, *node.leftLeaf, hit);
        }
    }

    void
    pushIntersectionsOnStack(const DBVHNode &node, double distanceRight, double distanceLeft, bool right, bool left,
                             TraversalContainer *stack, uint64_t &stackPointer) {
        if (right && left) {
            if (distanceRight < distanceLeft) {
                stack[stackPointer++] = {node.leftChild.get(), distanceLeft};
                stack[stackPointer++] = {node.rightChild.get(), distanceRight};
            } else {
                stack[stackPointer++] = {node.rightChild.get(), distanceRight};
                stack[stackPointer++] = {node.leftChild.get(), distanceLeft};
            }
        } else if (right) {
            stack[stackPointer++] = {node.rightChild.get(), distanceRight};
        } else if (left) {
            stack[stackPointer++] = {node.leftChild.get(), distanceLeft};
        }
    }

    bool processTraversalStack(IntersectionInfo &intersectionInfo, const Ray &ray, TraversalContainer *stack,
                               uint64_t stackPointer) {
        bool hit = false;
        while (stackPointer != 0) {
            stackPointer--;
            if (stack[stackPointer].distance < intersectionInfo.distance) {
                auto node = stack[stackPointer].node;

                double distanceRight = 0;
                double distanceLeft = 0;
                bool right = false;
                bool left = false;

                getChildrenIntersections(ray, *node, intersectionInfo, distanceRight, distanceLeft, right, left, hit);

                pushIntersectionsOnStack(*node, distanceRight, distanceLeft, right, left, stack, stackPointer);
            }
        }
        return hit;
    }

    bool traverseFirst(const DBVHNode &root, IntersectionInfo &intersectionInfo, const Ray &ray) {
        if (root.maxDepthRight >= 64 || root.maxDepthLeft >= 64) {
            std::vector<TraversalContainer> stack(
                    root.maxDepthRight > root.maxDepthLeft ? root.maxDepthRight + 1 : root.maxDepthLeft + 1);
            uint64_t stackPointer = 1;
            stack[0] = {&root, 0};
            return processTraversalStack(intersectionInfo, ray, stack.data(), stackPointer);
        } else {
            TraversalContainer stack[64];
            uint64_t stackPointer = 1;
            stack[0] = {&root, 0};
            return processTraversalStack(intersectionInfo, ray, stack, stackPointer);
        }
    }

    void intersectChild(const Ray &ray, const DBVHNode **stack, uint64_t &stackPointer, const DBVHNode *child) {
        double distance = 0;
        if (rayBoxIntersection((child->boundingBox.minCorner),
                               (child->boundingBox.maxCorner), ray, distance)) {
            stack[stackPointer++] = child;
        }
    }

    bool
    getChildrenIntersection(IntersectionInfo &intersectionInfo, const Ray &ray, const DBVHNode **stack,
                            uint64_t &stackPointer) {
        bool hit = false;
        auto node = stack[stackPointer];
        if (isNodeRight(*node)) {
            // TODO request child if missing
            intersectChild(ray, stack, stackPointer, node->rightChild.get());
        } else {
            // TODO request leaf if missing
            intersectLeaf(ray, intersectionInfo,*node->rightLeaf, hit);
        }
        if (isNodeLeft(*node) && !hit) {
            // TODO request child if missing
            intersectChild(ray, stack, stackPointer, node->leftChild.get());
        } else {
            // TODO request leaf if missing
            intersectLeaf(ray, intersectionInfo,*node->leftLeaf, hit);
        }
        return hit;
    }

    bool processTraversalStack(IntersectionInfo &intersectionInfo, const Ray &ray, const DBVHNode **stack,
                               uint64_t stackPointer) {
        while (stackPointer != 0) {
            stackPointer--;

            if (getChildrenIntersection(intersectionInfo, ray, stack, stackPointer))
                return true;
        }
        return false;
    }

    bool traverseAny(const DBVHNode &root, IntersectionInfo &intersectionInfo, const Ray &ray) {
        if (root.maxDepthRight >= 64 || root.maxDepthLeft >= 64) {
            std::vector<const DBVHNode *> stack(
                    root.maxDepthRight > root.maxDepthLeft ? root.maxDepthRight + 1 :
                    root.maxDepthLeft + 1);
            uint64_t stackPointer = 1;
            stack[0] = &root;
            return processTraversalStack(intersectionInfo, ray, stack.data(), stackPointer);
        } else {
            const DBVHNode *stack[64];
            uint64_t stackPointer = 1;
            stack[0] = &root;
            return processTraversalStack(intersectionInfo, ray, stack, stackPointer);
        }
    }

    void intersectLeaf(std::vector<IntersectionInfo> &intersectionInfo, const Ray &ray, Object &leaf) {
        IntersectionInfo intersectionInformationBuffer{};
        intersectionInformationBuffer.hit = false;
        intersectionInformationBuffer.distance = std::numeric_limits<double>::max();
        intersectionInformationBuffer.position = {0, 0, 0};
        leaf.intersectFirst(intersectionInformationBuffer, ray);
        if (intersectionInformationBuffer.hit) {
            intersectionInfo.push_back(intersectionInformationBuffer);
        }
    }

    void
    getChildrenIntersection(std::vector<IntersectionInfo> &intersectionInfo, const Ray &ray, const DBVHNode **stack,
                            uint64_t &stackPointer) {
        auto node = stack[stackPointer - 1];
        if (isNodeRight(*node)) {
            // TODO request child if missing
            intersectChild(ray, stack, stackPointer, node->rightChild.get());
        } else {
            // TODO request leaf if missing
            intersectLeaf(intersectionInfo, ray, *node->rightLeaf);
        }
        if (isNodeLeft(*node)) {
            // TODO request child if missing
            intersectChild(ray, stack, stackPointer, node->leftChild.get());
        } else {
            // TODO request leaf if missing
            intersectLeaf(intersectionInfo, ray, *node->leftLeaf);
        }
    }

    void processTraversalStack(std::vector<IntersectionInfo> &intersectionInfo, const Ray &ray,
                               const DBVHNode **stack, uint64_t stackPointer) {
        while (stackPointer != 0) {
            stackPointer--;

            getChildrenIntersection(intersectionInfo, ray, stack, stackPointer);
        }
    }

    bool traverseALl(const DBVHNode &root, std::vector<IntersectionInfo> &intersectionInfo, const Ray &ray) {
        if (root.maxDepthRight >= 64 || root.maxDepthLeft >= 64) {
            std::vector<const DBVHNode *> stack(
                    root.maxDepthRight > root.maxDepthLeft ? root.maxDepthRight + 1 :
                    root.maxDepthLeft + 1);

            uint64_t stackPointer = 1;
            stack[0] = &root;

            processTraversalStack(intersectionInfo, ray, stack.data(), stackPointer);
        } else {
            const DBVHNode *stack[64];
            uint64_t stackPointer = 1;
            stack[0] = &root;

            processTraversalStack(intersectionInfo, ray, stack, stackPointer);
        }
        return false;
    }


    void removeLastChild(DBVHNode &root) {
        root.maxDepthLeft = 0;
        root.surfaceArea = 0;
        root.leftLeaf = nullptr;
    }

    void removeSecondToLastChildLeft(DBVHNode &root) {
        root.leftLeaf = root.rightLeaf;
        root.rightLeaf = nullptr;
        root.maxDepthRight = 0;
        root.boundingBox = root.leftLeaf->getBoundaries();
        root.surfaceArea = root.leftLeaf->getSurfaceArea();
    }

    void removeSecondToLastChildRight(DBVHNode &root) {
        root.rightLeaf = nullptr;
        root.maxDepthRight = 0;
        root.boundingBox = root.leftLeaf->getBoundaries();
        root.surfaceArea = root.leftLeaf->getSurfaceArea();
    }

    void replaceRootWithChild(DBVHNode &root, DBVHNode &child) {
        if (isLeafLeft(child)) {
            root.leftLeaf = child.leftLeaf;
        } else {
            root.leftChild = std::move(child.leftChild);
        }
        if (isLeafRight(child)) {
            root.rightLeaf = child.rightLeaf;
        } else {
            root.rightChild = std::move(child.rightChild);
        }

        root.maxDepthLeft = child.maxDepthLeft;
        root.maxDepthRight = child.maxDepthRight;
        root.boundingBox = child.boundingBox;
        root.surfaceArea = child.surfaceArea;
    }

    void replaceRootWithRightChild(DBVHNode &root) {
        replaceRootWithChild(root, *root.rightChild);
    }

    void replaceRootWithLeftChild(DBVHNode &root) {
        replaceRootWithChild(root, *root.leftChild);
    }

    bool removeSpecialCases(DBVHNode &root, const Object &object) {
        if (isLastElement(root)) {
            if (*root.leftLeaf == object) {
                removeLastChild(root);
                return true;
            }
        } else if (isLeafLeft(root) && *root.leftLeaf == object) {
            if (isLeafRight(root)) {
                removeSecondToLastChildLeft(root);
            } else {
                replaceRootWithRightChild(root);
            }
            return true;
        } else if (isLeafRight(root) && *root.rightLeaf == object) {
            if (isLeafLeft(root)) {
                removeSecondToLastChildRight(root);
            } else {
                replaceRootWithLeftChild(root);
            }
            return true;
        }
        return false;
    }

    bool addFirstAndOnlyElement(DBVHNode &root, const std::vector<Object *> &objects) {
        if (!isEmpty(root) || objects.size() != 1) return false;
        refit(root.boundingBox, objects, 0);
        root.leftLeaf = objects.back();
        root.maxDepthLeft = 1;
        return true;
    }

    bool addOntoSingleElement(DBVHNode &root, const std::vector<Object *> &objects) {
        if (!isLastElement(root)) return false;
        std::vector<Object *> newObjects{objects};     // TODO: make more efficient
        newObjects.push_back(root.leftLeaf);
        root.maxDepthLeft = 0;
        add(root, newObjects, 1);
        return true;
    }
}

void DBVHv2::addObjects(DBVHNode &root, const std::vector<Object *> &objects) {
    if (objects.empty() || addFirstAndOnlyElement(root, objects) || addOntoSingleElement(root, objects))
        return;

    add(root, objects, 1);
}

void DBVHv2::removeObjects(DBVHNode &root, const std::vector<Object *> &objects) {
    for (const auto &object: objects) {
        if (isEmpty(root)) return;
        if (!removeSpecialCases(root, *object)) {
            remove(root, *object);
        }
    }
}

bool DBVHv2::intersectFirst(const DBVHNode &root, IntersectionInfo &intersectionInfo, const Ray &ray) {
    if (isEmpty(root)) return false;
    bool hit;

    if (isLastElement(root)) {
        hit = root.leftLeaf->intersectFirst(intersectionInfo, ray);
    } else {
        hit = traverseFirst(root, intersectionInfo, ray);
    }

    return hit;
}

bool DBVHv2::intersectAny(const DBVHNode &root, IntersectionInfo &intersectionInfo, const Ray &ray) {
    if (isEmpty(root)) return false;
    bool hit;

    if (isLastElement(root)) {
        hit = root.leftLeaf->intersectAny(intersectionInfo, ray);
    } else {
        hit = traverseAny(root, intersectionInfo, ray);
    }

    return hit;
}

bool DBVHv2::intersectAll(const DBVHNode &root, std::vector<IntersectionInfo> &intersectionInfo, const Ray &ray) {
    if (isEmpty(root)) return false;
    bool hit;

    if (isLastElement(root)) {
        hit = root.leftLeaf->intersectAll(intersectionInfo, ray);
    } else {
        hit = traverseALl(root, intersectionInfo, ray);
    }

    return hit;
}
