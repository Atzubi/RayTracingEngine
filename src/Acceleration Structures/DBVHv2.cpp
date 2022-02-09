//
// Created by Sebastian on 13.12.2021.
//

#include <algorithm>
#include <limits>
#include "DBVHv2.h"

namespace {
    constexpr int numberOfSplittingPlanes = 9;
    constexpr int numberOfPossibleRotation = 5;

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
            Vector3D scale = aabb.maxCorner - aabb.minCorner;
            aabb.minCorner -= scale * looseness;
            aabb.maxCorner += scale * looseness;
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
        Vector3D v1 = (min - ray.origin) * ray.dirfrac;
        Vector3D v2 = (max - ray.origin) * ray.dirfrac;

        double tMin = std::max(std::max(std::min(v1.x, v2.x), std::min(v1.y, v2.y)), std::min(v1.z, v2.z));
        double tMax = std::min(std::min(std::max(v1.x, v2.x), std::max(v1.y, v2.y)), std::max(v1.z, v2.z));

        distance = tMin;
        return tMax >= 0 && tMin <= tMax;
    }

    void refitChild(DBVHNode &node, DBVHNode &child) {
        refit(node.boundingBox, child.boundingBox);
        node.surfaceArea = child.surfaceArea;
        node.maxDepthRight = std::max(child.maxDepthRight, child.maxDepthLeft) + 1;
    }

    void refitLeaf(DBVHNode &node, Object &leaf) {
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

    struct BoxSA {
        BoundingBox box;
        double sa = 0;
    };

    struct Rotations {
        BoxSA left;
        BoxSA right;
        BoxSA leftLeft;
        BoxSA leftRight;
        BoxSA rightRight;
        BoxSA rightLeft;
        BoundingBox swapLeftLeftToRight;
        BoundingBox swapLeftRightToRight;
        BoundingBox swapRightLeftToLeft;
        BoundingBox swapRightRightToLeft;
    };

    BoundingBox createSwapBox(BoundingBox a, BoundingBox b) {
        return {{std::min(a.minCorner.x, b.minCorner.x), std::min(a.minCorner.y, b.minCorner.y), std::min(a.minCorner.z,
                                                                                                          b.minCorner.z)},
                {std::max(a.maxCorner.x, b.maxCorner.x), std::max(a.maxCorner.y, b.maxCorner.y), std::max(a.maxCorner.z,
                                                                                                          b.maxCorner.z)}};
    }

    void fillRotationBoxes(BoxSA &left, BoxSA &right, const DBVHNode &node) {
        if (isNodeLeft(node)) {
            left.box = (node.leftChild)->boundingBox;
            left.sa = (node.leftChild)->surfaceArea;
        } else {
            left.box = (node.leftLeaf)->getBoundaries();
            left.sa = (node.leftLeaf)->getSurfaceArea();
        }
        if (isNodeRight(node)) {
            right.box = (node.rightChild)->boundingBox;
            right.sa = (node.rightChild)->surfaceArea;
        } else {
            right.box = (node.rightLeaf)->getBoundaries();
            right.sa = (node.rightLeaf)->getSurfaceArea();
        }
    }

    enum Rotation {
        NoRotation, SwapLeftLeftToRight, SwapLeftRightToRight, SwapRightLeftToLeft, SwapRightRightToLeft
    };

    void computeSwapSAHsFull(const DBVHNode &node, double *SAHs, const Rotations &rotations) {
        SAHs[SwapLeftLeftToRight] =
                node.boundingBox.getSA() + rotations.leftLeft.sa + rotations.right.sa + rotations.leftRight.sa +
                rotations.swapLeftLeftToRight.getSA();
        SAHs[SwapLeftRightToRight] =
                node.boundingBox.getSA() + rotations.leftRight.sa + rotations.right.sa + rotations.leftLeft.sa +
                rotations.swapLeftRightToRight.getSA();
        SAHs[SwapRightLeftToLeft] = node.boundingBox.getSA() + rotations.rightLeft.sa + rotations.left.sa +
                                    rotations.rightRight.sa + rotations.swapRightLeftToLeft.getSA();
        SAHs[SwapRightRightToLeft] = node.boundingBox.getSA() + rotations.rightRight.sa + rotations.left.sa +
                                     rotations.rightLeft.sa + rotations.swapRightRightToLeft.getSA();
    }

    void computeSwapSAHsLeft(const DBVHNode &node, double *SAHs, const Rotations &rotations) {
        SAHs[SwapLeftLeftToRight] =
                node.boundingBox.getSA() + rotations.leftLeft.sa + rotations.right.sa + rotations.leftRight.sa +
                rotations.swapLeftLeftToRight.getSA();
        SAHs[SwapLeftRightToRight] =
                node.boundingBox.getSA() + rotations.leftRight.sa + rotations.right.sa + rotations.leftLeft.sa +
                rotations.swapLeftRightToRight.getSA();
        SAHs[SwapRightLeftToLeft] = std::numeric_limits<double>::max();
        SAHs[SwapRightRightToLeft] = std::numeric_limits<double>::max();
    }

    void computeSwapSAHsRight(const DBVHNode &node, double *SAHs, const Rotations &rotations) {
        SAHs[SwapLeftLeftToRight] = std::numeric_limits<double>::max();
        SAHs[SwapLeftRightToRight] = std::numeric_limits<double>::max();
        SAHs[SwapRightLeftToLeft] = node.boundingBox.getSA() + rotations.rightLeft.sa + rotations.left.sa +
                                    rotations.rightRight.sa + rotations.swapRightLeftToLeft.getSA();
        SAHs[SwapRightRightToLeft] = node.boundingBox.getSA() + rotations.rightRight.sa + rotations.left.sa +
                                     rotations.rightLeft.sa + rotations.swapRightRightToLeft.getSA();
    }

    void fillRightRotationBoxes(const DBVHNode &node, Rotations &rotations) {
        auto rightNode = node.rightChild.get();
        rotations.right.box = rightNode->boundingBox;
        rotations.right.sa = rightNode->surfaceArea;
        fillRotationBoxes(rotations.rightLeft, rotations.rightRight, *rightNode);
    }

    bool getRotationsRight(DBVHNode &node, double *SAHs, Rotations &rotations) {
        auto leftNode = node.leftLeaf;
        rotations.left.box = leftNode->getBoundaries();
        rotations.left.sa = leftNode->getSurfaceArea();

        if (isNodeRight(node)) {
            fillRightRotationBoxes(node, rotations);

            rotations.swapRightLeftToLeft = createSwapBox(rotations.left.box, rotations.rightRight.box);
            rotations.swapRightRightToLeft = createSwapBox(rotations.left.box, rotations.rightLeft.box);

            computeSwapSAHsRight(node, SAHs, rotations);
            return true;
        } else {
            return false;
        }
    }

    void getRotationsLeft(DBVHNode &node, double *SAHs, Rotations &rotations) {
        auto rightNode = node.rightLeaf;
        rotations.right.box = rightNode->getBoundaries();
        rotations.right.sa = rightNode->getSurfaceArea();

        rotations.swapLeftLeftToRight = createSwapBox(rotations.right.box, rotations.leftRight.box);
        rotations.swapLeftRightToRight = createSwapBox(rotations.right.box, rotations.leftLeft.box);

        computeSwapSAHsLeft(node, SAHs, rotations);
    }

    void getRotationsFull(const DBVHNode &node, double *SAHs, Rotations &rotations) {
        fillRightRotationBoxes(node, rotations);

        rotations.swapLeftLeftToRight = createSwapBox(rotations.right.box, rotations.leftRight.box);
        rotations.swapLeftRightToRight = createSwapBox(rotations.right.box, rotations.leftLeft.box);
        rotations.swapRightLeftToLeft = createSwapBox(rotations.left.box, rotations.rightRight.box);
        rotations.swapRightRightToLeft = createSwapBox(rotations.left.box, rotations.rightLeft.box);

        computeSwapSAHsFull(node, SAHs, rotations);
    }

    void getRotations(DBVHNode &node, double *SAHs, Rotations &rotations) {
        auto leftNode = node.leftChild.get();
        rotations.left.box = leftNode->boundingBox;
        rotations.left.sa = leftNode->surfaceArea;
        fillRotationBoxes(rotations.leftLeft, rotations.leftRight, *leftNode);

        if (isNodeRight(node)) {
            getRotationsFull(node, SAHs, rotations);
        } else {
            getRotationsLeft(node, SAHs, rotations);
        }
    }

    bool getPossibleRotations(DBVHNode &node, double *SAHs, Rotations rotations) {
        if (isNodeLeft(node)) {
            getRotations(node, SAHs, rotations);
        } else {
            return getRotationsRight(node, SAHs, rotations);
        }
        return true;
    }

    Rotation getBestSAH(const double *SAHs) {
        Rotation bestSAH = NoRotation;
        if (SAHs[SwapLeftLeftToRight] < SAHs[NoRotation]) {
            bestSAH = SwapLeftLeftToRight;
        }
        if (SAHs[SwapLeftRightToRight] < SAHs[bestSAH]) {
            bestSAH = SwapLeftRightToRight;
        }
        if (SAHs[SwapRightLeftToLeft] < SAHs[bestSAH]) {
            bestSAH = SwapRightLeftToLeft;
        }
        if (SAHs[SwapRightRightToLeft] < SAHs[bestSAH]) {
            bestSAH = SwapRightRightToLeft;
        }
        return bestSAH;
    }

    void setNodeLeftLeft(DBVHNode &node) {
        if (isNodeLeft(*node.leftChild)) {
            node.rightChild = std::move((node.leftChild)->leftChild);
            node.maxDepthRight =
                    std::max(node.rightChild->maxDepthLeft, node.rightChild->maxDepthRight) + 1;
        } else {
            node.rightLeaf = (node.leftChild)->leftLeaf;
            node.maxDepthRight = 1;
        }
    }

    void swapChildLeftLeft(DBVHNode &node, const Rotations &rotations) {
        auto buffer = std::move(node.rightChild);
        setNodeLeftLeft(node);
        (node.leftChild)->boundingBox = rotations.swapLeftLeftToRight;
        (node.leftChild)->maxDepthLeft = std::max(buffer->maxDepthLeft, buffer->maxDepthRight) + 1;
        (node.leftChild)->leftChild = std::move(buffer);
    }

    void swapLeafLeftLeft(DBVHNode &node, const Rotations &rotations) {
        auto buffer = node.rightLeaf;
        setNodeLeftLeft(node);
        (node.leftChild)->boundingBox = rotations.swapLeftLeftToRight;
        (node.leftChild)->leftLeaf = buffer;
        (node.leftChild)->maxDepthLeft = 1;
    }

    void setSurfaceAreaLeftLeft(DBVHNode &node, const Rotations &rotations, const double *SAHs) {
        node.surfaceArea = SAHs[SwapLeftLeftToRight];
        (node.leftChild)->surfaceArea =
                rotations.right.sa + rotations.leftRight.sa + rotations.swapLeftLeftToRight.getSA();
    }

    void swapLeftLeftToRight(DBVHNode &node, const Rotations &rotations, const double *SAHs) {
        if (isNodeRight(node)) {
            swapChildLeftLeft(node, rotations);
        } else {
            swapLeafLeftLeft(node, rotations);
        }
        setSurfaceAreaLeftLeft(node, rotations, SAHs);
    }

    void setNodeLeftRight(DBVHNode &node) {
        if (isNodeRight(*node.leftChild)) {
            node.rightChild = std::move((node.leftChild)->rightChild);
            node.maxDepthRight =
                    std::max(node.rightChild->maxDepthLeft, node.rightChild->maxDepthRight) + 1;
        } else {
            node.rightLeaf = (node.leftChild)->rightLeaf;
            node.maxDepthRight = 1;
        }
    }

    void swapChildLeftRight(DBVHNode &node, const Rotations &rotations) {
        auto buffer = std::move(node.rightChild);
        setNodeLeftRight(node);
        (node.leftChild)->boundingBox = rotations.swapLeftRightToRight;
        (node.leftChild)->maxDepthRight = std::max(buffer->maxDepthLeft, buffer->maxDepthRight) + 1;
        (node.leftChild)->rightChild = std::move(buffer);
    }

    void swapLeafLeftRight(DBVHNode &node, const Rotations &rotations) {
        auto buffer = node.rightLeaf;
        setNodeLeftRight(node);
        (node.leftChild)->boundingBox = rotations.swapLeftRightToRight;
        (node.leftChild)->rightLeaf = buffer;
        (node.leftChild)->maxDepthRight = 1;
    }

    void setSurfaceAreaLeftRight(DBVHNode &node, const Rotations &rotations, const double *SAHs) {
        node.surfaceArea = SAHs[SwapLeftRightToRight];
        (node.leftChild)->surfaceArea =
                rotations.right.sa + rotations.leftLeft.sa + rotations.swapLeftRightToRight.getSA();
    }

    void swapLeftRightToRight(DBVHNode &node, const Rotations &rotations, const double *SAHs) {
        if (isNodeRight(node)) {
            swapChildLeftRight(node, rotations);
        } else {
            swapLeafLeftRight(node, rotations);
        }
        setSurfaceAreaLeftRight(node, rotations, SAHs);
    }

    void setNodeRightLeft(DBVHNode &node) {
        if (isNodeLeft(*node.rightChild)) {
            node.leftChild = std::move((node.rightChild)->leftChild);
            node.maxDepthLeft = std::max(node.leftChild->maxDepthLeft, node.leftChild->maxDepthRight) + 1;
        } else {
            node.leftLeaf = (node.rightChild)->leftLeaf;
            node.maxDepthLeft = 1;
        }
    }

    void swapChildRightLeft(DBVHNode &node, const Rotations &rotations) {
        auto buffer = std::move(node.leftChild);
        setNodeRightLeft(node);
        (node.rightChild)->boundingBox = rotations.swapRightLeftToLeft;
        (node.rightChild)->maxDepthLeft = std::max(buffer->maxDepthLeft, buffer->maxDepthRight) + 1;
        (node.rightChild)->leftChild = std::move(buffer);
    }

    void swapLeafRightLeft(DBVHNode &node, const Rotations &rotations) {
        auto buffer = node.leftLeaf;
        setNodeRightLeft(node);
        (node.rightChild)->boundingBox = rotations.swapRightLeftToLeft;
        (node.rightChild)->leftLeaf = buffer;
        (node.rightChild)->maxDepthLeft = 1;
    }

    void setSurfaceAreaRightLeft(DBVHNode &node, const Rotations &rotations, const double *SAHs) {
        node.surfaceArea = SAHs[SwapRightLeftToLeft];
        (node.rightChild)->surfaceArea =
                rotations.left.sa + rotations.rightRight.sa + rotations.swapRightLeftToLeft.getSA();
    }

    void swapRightLeft(DBVHNode &node, const Rotations &rotations, const double *SAHs) {
        if (isNodeLeft(node)) {
            swapChildRightLeft(node, rotations);
        } else {
            swapLeafRightLeft(node, rotations);
        }
        setSurfaceAreaRightLeft(node, rotations, SAHs);
    }

    void setNodeRightRight(DBVHNode &node) {
        if (isNodeRight(*node.rightChild)) {
            node.leftChild = std::move((node.rightChild)->rightChild);
            node.maxDepthLeft = std::max(node.leftChild->maxDepthLeft, node.leftChild->maxDepthRight) + 1;
        } else {
            node.leftLeaf = (node.rightChild)->rightLeaf;
            node.maxDepthLeft = 1;
        }
    }

    void swapChildRightRight(DBVHNode &node, const Rotations &rotations) {
        auto buffer = std::move(node.leftChild);
        setNodeRightRight(node);
        (node.rightChild)->boundingBox = rotations.swapRightRightToLeft;
        (node.rightChild)->maxDepthRight = std::max(buffer->maxDepthLeft, buffer->maxDepthRight) + 1;
        (node.rightChild)->rightChild = std::move(buffer);
    }

    void swapLeafRightRight(DBVHNode &node, const Rotations &rotations) {
        auto buffer = node.leftLeaf;
        setNodeRightRight(node);
        (node.rightChild)->boundingBox = rotations.swapRightRightToLeft;
        (node.rightChild)->rightLeaf = buffer;
        (node.rightChild)->maxDepthRight = 1;
    }

    void setSurfaceAreaRightRight(DBVHNode &node, const Rotations &rotations, const double *SAHs) {
        node.surfaceArea = SAHs[SwapRightRightToLeft];
        (node.rightChild)->surfaceArea =
                rotations.left.sa + rotations.rightLeft.sa + rotations.swapRightRightToLeft.getSA();
    }

    void swapRightRight(DBVHNode &node, const Rotations &rotations, const double *SAHs) {
        if (isNodeLeft(node)) {
            swapChildRightRight(node, rotations);
        } else {
            swapLeafRightRight(node, rotations);
        }
        setSurfaceAreaRightRight(node, rotations, SAHs);
    }

    bool optimizeSAH(DBVHNode &node) {
        Rotations rotations;
        double SAHs[numberOfPossibleRotation];

        if (!getPossibleRotations(node, SAHs, rotations)) return false;

        SAHs[NoRotation] = node.surfaceArea;
        Rotation bestSAH = getBestSAH(SAHs);

        switch (bestSAH) {
            case NoRotation:
                node.surfaceArea = SAHs[NoRotation];
                return false;
            case SwapLeftLeftToRight: {
                swapLeftLeftToRight(node, rotations, SAHs);
                break;
            }
            case SwapLeftRightToRight: {
                swapLeftRightToRight(node, rotations, SAHs);
                break;
            }
            case SwapRightLeftToLeft: {
                swapRightLeft(node, rotations, SAHs);
                break;
            }
            case SwapRightRightToLeft: {
                swapRightRight(node, rotations, SAHs);
                break;
            }
        }
        return true;
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

    bool processTraversalStack(IntersectionInfo &intersectionInfo, const Ray &ray, TraversalContainer *stack) {
        bool hit = false;
        uint64_t stackPointer = 1;
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
            stack[0] = {&root, 0};
            return processTraversalStack(intersectionInfo, ray, stack.data());
        } else {
            TraversalContainer stack[64];
            stack[0] = {&root, 0};
            return processTraversalStack(intersectionInfo, ray, stack);
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
            intersectLeaf(ray, intersectionInfo, *node->rightLeaf, hit);
        }
        if (isNodeLeft(*node) && !hit) {
            // TODO request child if missing
            intersectChild(ray, stack, stackPointer, node->leftChild.get());
        } else {
            // TODO request leaf if missing
            intersectLeaf(ray, intersectionInfo, *node->leftLeaf, hit);
        }
        return hit;
    }

    bool processTraversalStack(IntersectionInfo &intersectionInfo, const Ray &ray, const DBVHNode **stack) {
        uint64_t stackPointer = 1;
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
            stack[0] = &root;
            return processTraversalStack(intersectionInfo, ray, stack.data());
        } else {
            const DBVHNode *stack[64];
            stack[0] = &root;
            return processTraversalStack(intersectionInfo, ray, stack);
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
                               const DBVHNode **stack) {
        uint64_t stackPointer = 1;
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

            processTraversalStack(intersectionInfo, ray, stack.data());
        } else {
            const DBVHNode *stack[64];
            uint64_t stackPointer = 1;
            stack[0] = &root;

            processTraversalStack(intersectionInfo, ray, stack);
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
