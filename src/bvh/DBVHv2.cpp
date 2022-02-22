//
// Created by Sebastian on 13.12.2021.
//

#include <algorithm>
#include <limits>
#include "DBVHv2.h"

namespace {
    struct TraversalContainer {
        const DBVHNode *node;
        double distance;
    };

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

    void refit(BoundingBox &target, BoundingBox resizeBy) {
        target.minCorner.x = std::min(target.minCorner.x, resizeBy.minCorner.x);
        target.minCorner.y = std::min(target.minCorner.y, resizeBy.minCorner.y);
        target.minCorner.z = std::min(target.minCorner.z, resizeBy.minCorner.z);
        target.maxCorner.x = std::max(target.maxCorner.x, resizeBy.maxCorner.x);
        target.maxCorner.y = std::max(target.maxCorner.y, resizeBy.maxCorner.y);
        target.maxCorner.z = std::max(target.maxCorner.z, resizeBy.maxCorner.z);
    }

    void refit(BoundingBox &aabb, const Intersectable &object) {
        refit(aabb, object.getBoundaries());
    }

    void refit(BoundingBox &aabb, const std::vector<Intersectable *> &objects, double looseness) {
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

    bool isObjectInLeftSplit(const Vector3D &splittingPlane, int currentSplittingPlane, Intersectable *const &object) {
        return (object->getBoundaries().maxCorner[currentSplittingPlane] +
                object->getBoundaries().minCorner[currentSplittingPlane]) / 2 <
               splittingPlane[currentSplittingPlane];
    }

    void sortObjectsIntoBuckets(const std::vector<Intersectable *> &objects, const Vector3D &splittingPlane,
                                BoundingBox &aabbLeft, BoundingBox &aabbRight, double &objectCostLeft,
                                double &objectCostRight) {
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

    void split(std::vector<Intersectable *> &leftChild, std::vector<Intersectable *> &rightChild,
               const std::vector<Intersectable *> &objects, const Vector3D &splittingPlane) {
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

    void refitChild(DBVHNode &node, DBVHNode &child) {
        refit(node.boundingBox, child.boundingBox);
        node.surfaceArea += child.surfaceArea;
    }

    void refitLeaf(DBVHNode &node, Intersectable &leaf) {
        refit(node.boundingBox, leaf.getBoundaries());
        node.surfaceArea += leaf.getSurfaceArea();
    }

    BoundingBox createSwapBox(BoundingBox a, BoundingBox b) {
        return {{std::min(a.minCorner.x, b.minCorner.x), std::min(a.minCorner.y, b.minCorner.y), std::min(a.minCorner.z,
                                                                                                          b.minCorner.z)},
                {std::max(a.maxCorner.x, b.maxCorner.x), std::max(a.maxCorner.y, b.maxCorner.y), std::max(a.maxCorner.z,
                                                                                                          b.maxCorner.z)}};
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


    void getRotationsLeft(DBVHNode &node, double *SAHs, Rotations &rotations) {
        auto rightNode = node.rightLeaf;
        rotations.right.box = rightNode->getBoundaries();
        rotations.right.sa = rightNode->getSurfaceArea();

        rotations.swapLeftLeftToRight = createSwapBox(rotations.right.box, rotations.leftRight.box);
        rotations.swapLeftRightToRight = createSwapBox(rotations.right.box, rotations.leftLeft.box);

        computeSwapSAHsLeft(node, SAHs, rotations);
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

    std::vector<Vector3D> createSplittingPlanes(BoundingBox &bBox) {
        std::vector<Vector3D> splittingPlanes(numberOfSplittingPlanes);
        int splitsPerDimension = numberOfSplittingPlanes / 3 + 1;

        for (int dim = 0; dim < 3; dim++) {
            double split = (bBox.maxCorner[dim] - bBox.minCorner[dim]) / splitsPerDimension;
            for (int plane = 0; plane < numberOfSplittingPlanes / 3; plane++) {
                splittingPlanes[3 * dim + plane][dim] =
                        bBox.minCorner[dim] + split * (plane + 1) + std::numeric_limits<double>::min();
            }
        }

        return splittingPlanes;
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

    void splitEven(const std::vector<Intersectable *> &objects, std::vector<Intersectable *> &left,
                   std::vector<Intersectable *> &right) {
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

    void switchBoxOrder(std::vector<Intersectable *> &leftObjects, std::vector<Intersectable *> &rightObjects) {
        auto buffer = leftObjects;
        leftObjects = rightObjects;
        rightObjects = buffer;
    }

    void createLeftLeaf(DBVHNode &node, const std::vector<Intersectable *> &leftObjects) {
        node.leftLeaf = leftObjects.at(0);
        node.maxDepthLeft = 1;
    }

    void createRightLeaf(DBVHNode &node, const std::vector<Intersectable *> &rightObjects) {
        node.rightLeaf = rightObjects.at(0);
        node.maxDepthRight = 1;
    }

    inline void intersectLeaf(const Ray &ray, IntersectionInfo &intersectionInfo, Intersectable &leaf, bool &hit) {
        IntersectionInfo info{false, std::numeric_limits<double>::max()};
        leaf.intersectFirst(info, ray);
        if (info.hit && info.distance < intersectionInfo.distance) {
            intersectionInfo = info;
            hit = true;
        }
    }

    inline bool rayBoxIntersection(const Vector3D &min, const Vector3D &max, const Ray &ray, double &distance) {
        Vector3D v1 = (min - ray.origin) * ray.dirfrac;
        Vector3D v2 = (max - ray.origin) * ray.dirfrac;

        double tMin = std::max(std::max(std::min(v1.x, v2.x), std::min(v1.y, v2.y)), std::min(v1.z, v2.z));
        double tMax = std::min(std::min(std::max(v1.x, v2.x), std::max(v1.y, v2.y)), std::max(v1.z, v2.z));

        distance = tMin;
        return tMax >= 0 && tMin <= tMax;
    }

    inline void intersectChild(const Ray &ray, const DBVHNode **stack, uint64_t &stackPointer, const DBVHNode *child) {
        double distance = 0;
        if (rayBoxIntersection((child->boundingBox.minCorner),
                               (child->boundingBox.maxCorner), ray, distance)) {
            stack[stackPointer++] = child;
        }
    }

    inline void intersectLeaf(std::vector<IntersectionInfo> &intersectionInfo, const Ray &ray, Intersectable &leaf) {
        IntersectionInfo info{false, std::numeric_limits<double>::max()};
        leaf.intersectFirst(info, ray);
        if (info.hit) {
            intersectionInfo.push_back(info);
        }
    }

    inline void getChildrenIntersections(const Ray &ray, const DBVHNode &node, IntersectionInfo &intersectionInfo,
                                         double &distanceRight, double &distanceLeft, bool &right, bool &left,
                                         bool &hit) {
        if (isNodeRight(node)) {
            // TODO request child if missing
            auto rightChild = node.rightChild;
            right = rayBoxIntersection(rightChild->boundingBox.minCorner, rightChild->boundingBox.maxCorner, ray,
                                       distanceRight);
        } else {
            // TODO request leaf if missing
            intersectLeaf(ray, intersectionInfo, *node.rightLeaf, hit);
        }
        if (isNodeLeft(node)) {
            // TODO request child if missing
            auto leftChild = node.leftChild;
            left = rayBoxIntersection(leftChild->boundingBox.minCorner, leftChild->boundingBox.maxCorner, ray,
                                      distanceLeft);
        } else {
            // TODO request leaf if missing
            intersectLeaf(ray, intersectionInfo, *node.leftLeaf, hit);
        }
    }

    inline void
    pushIntersectionsOnStack(const DBVHNode &node, double distanceRight, double distanceLeft, bool right, bool left,
                             TraversalContainer *stack, uint64_t &stackPointer) {
        if (right && left) {
            if (distanceRight < distanceLeft) {
                stack[stackPointer++] = {node.leftChild, distanceLeft};
                stack[stackPointer++] = {node.rightChild, distanceRight};
            } else {
                stack[stackPointer++] = {node.rightChild, distanceRight};
                stack[stackPointer++] = {node.leftChild, distanceLeft};
            }
        } else if (right) {
            stack[stackPointer++] = {node.rightChild, distanceRight};
        } else if (left) {
            stack[stackPointer++] = {node.leftChild, distanceLeft};
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

    bool traverseFirst(DBVHNode &root, IntersectionInfo &intersectionInfo, const Ray &ray) {
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

    inline bool getChildrenIntersection(IntersectionInfo &intersectionInfo, const Ray &ray, const DBVHNode **stack,
                                        uint64_t &stackPointer) {
        bool hit = false;
        auto node = stack[stackPointer];
        if (isNodeRight(*node)) {
            // TODO request child if missing
            intersectChild(ray, stack, stackPointer, node->rightChild);
        } else {
            // TODO request leaf if missing
            intersectLeaf(ray, intersectionInfo, *node->rightLeaf, hit);
        }
        if (isNodeLeft(*node) && !hit) {
            // TODO request child if missing
            intersectChild(ray, stack, stackPointer, node->leftChild);
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

    bool traverseAny(DBVHNode &root, IntersectionInfo &intersectionInfo, const Ray &ray) {
        if (root.maxDepthRight >= 64 || root.maxDepthLeft >= 64) {
            std::vector<const DBVHNode *> stack(
                    root.maxDepthRight > root.maxDepthLeft ? root.maxDepthRight + 1 : root.maxDepthLeft + 1);
            stack[0] = &root;
            return processTraversalStack(intersectionInfo, ray, stack.data());
        } else {
            const DBVHNode *stack[64];
            stack[0] = &root;
            return processTraversalStack(intersectionInfo, ray, stack);
        }
    }

    inline void
    getChildrenIntersection(std::vector<IntersectionInfo> &intersectionInfo, const Ray &ray, const DBVHNode **stack,
                            uint64_t &stackPointer) {
        auto node = stack[stackPointer - 1];
        if (isNodeRight(*node)) {
            // TODO request child if missing
            intersectChild(ray, stack, stackPointer, node->rightChild);
        } else {
            // TODO request leaf if missing
            intersectLeaf(intersectionInfo, ray, *node->rightLeaf);
        }
        if (isNodeLeft(*node)) {
            // TODO request child if missing
            intersectChild(ray, stack, stackPointer, node->leftChild);
        } else {
            // TODO request leaf if missing
            intersectLeaf(intersectionInfo, ray, *node->leftLeaf);
        }
    }

    void processTraversalStack(std::vector<IntersectionInfo> &intersectionInfo, const Ray &ray, const DBVHNode **stack) {
        uint64_t stackPointer = 1;
        while (stackPointer != 0) {
            stackPointer--;

            getChildrenIntersection(intersectionInfo, ray, stack, stackPointer);
        }
    }

    bool traverseALl(DBVHNode &root, std::vector<IntersectionInfo> &intersectionInfo, const Ray &ray) {
        if (root.maxDepthRight >= 64 || root.maxDepthLeft >= 64) {
            std::vector<const DBVHNode *> stack(
                    root.maxDepthRight > root.maxDepthLeft ? root.maxDepthRight + 1 : root.maxDepthLeft + 1);
            stack[0] = &root;
            processTraversalStack(intersectionInfo, ray, stack.data());
        } else {
            const DBVHNode *stack[64];
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

    bool addFirstAndOnlyElement(DBVHNode &root, const std::vector<Intersectable *> &objects) {
        if (!isEmpty(root) || objects.size() != 1) return false;
        refit(root.boundingBox, objects, 0);
        root.leftLeaf = objects.back();
        root.maxDepthLeft = 1;
        return true;
    }
}

void DBVHv2::fillRotationBoxes(BoxSA &left, BoxSA &right, const DBVHNode &node) {
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

void DBVHv2::fillRightRotationBoxes(const DBVHNode &node, Rotations &rotations) {
    auto rightNode = node.rightChild;
    rotations.right.box = rightNode->boundingBox;
    rotations.right.sa = rightNode->surfaceArea;
    fillRotationBoxes(rotations.rightLeft, rotations.rightRight, *rightNode);
}

bool DBVHv2::getRotationsRight(DBVHNode &node, double *SAHs, Rotations &rotations) {
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

void DBVHv2::getRotationsFull(const DBVHNode &node, double *SAHs, Rotations &rotations) {
    fillRightRotationBoxes(node, rotations);

    rotations.swapLeftLeftToRight = createSwapBox(rotations.right.box, rotations.leftRight.box);
    rotations.swapLeftRightToRight = createSwapBox(rotations.right.box, rotations.leftLeft.box);
    rotations.swapRightLeftToLeft = createSwapBox(rotations.left.box, rotations.rightRight.box);
    rotations.swapRightRightToLeft = createSwapBox(rotations.left.box, rotations.rightLeft.box);

    computeSwapSAHsFull(node, SAHs, rotations);
}

void DBVHv2::getRotations(DBVHNode &node, double *SAHs, Rotations &rotations) {
    auto leftNode = node.leftChild;
    rotations.left.box = leftNode->boundingBox;
    rotations.left.sa = leftNode->surfaceArea;
    fillRotationBoxes(rotations.leftLeft, rotations.leftRight, *leftNode);

    if (isNodeRight(node)) {
        getRotationsFull(node, SAHs, rotations);
    } else {
        getRotationsLeft(node, SAHs, rotations);
    }
}

bool DBVHv2::getPossibleRotations(DBVHNode &node, double *SAHs, Rotations &rotations) {
    if (isNodeLeft(node)) {
        getRotations(node, SAHs, rotations);
    } else {
        return getRotationsRight(node, SAHs, rotations);
    }
    return true;
}

void DBVHv2::setNodeLeftLeft(DBVHNode &node) {
    if (isNodeLeft(*node.leftChild)) {
        node.rightChild = (node.leftChild)->leftChild;
        node.maxDepthRight =
                std::max(node.rightChild->maxDepthLeft, node.rightChild->maxDepthRight) + 1;
    } else {
        node.rightLeaf = (node.leftChild)->leftLeaf;
        node.maxDepthRight = 1;
    }
}

void DBVHv2::swapChildLeftLeft(DBVHNode &node, const Rotations &rotations) {
    auto buffer = node.rightChild;
    setNodeLeftLeft(node);
    (node.leftChild)->boundingBox = rotations.swapLeftLeftToRight;
    (node.leftChild)->maxDepthLeft = std::max(buffer->maxDepthLeft, buffer->maxDepthRight) + 1;
    (node.leftChild)->leftChild = buffer;
}

void DBVHv2::swapLeafLeftLeft(DBVHNode &node, const Rotations &rotations) {
    auto buffer = node.rightLeaf;
    setNodeLeftLeft(node);
    (node.leftChild)->boundingBox = rotations.swapLeftLeftToRight;
    (node.leftChild)->leftLeaf = buffer;
    (node.leftChild)->maxDepthLeft = 1;
}

void DBVHv2::setSurfaceAreaLeftLeft(DBVHNode &node, const Rotations &rotations, const double *SAHs) {
    node.surfaceArea = SAHs[SwapLeftLeftToRight];
    node.leftChild->surfaceArea =
            rotations.right.sa + rotations.leftRight.sa + rotations.swapLeftLeftToRight.getSA();
}

void DBVHv2::swapLeftLeftToRight(DBVHNode &node, const Rotations &rotations, const double *SAHs) {
    if (isNodeRight(node)) {
        swapChildLeftLeft(node, rotations);
    } else {
        swapLeafLeftLeft(node, rotations);
    }
    setSurfaceAreaLeftLeft(node, rotations, SAHs);
}

void DBVHv2::setNodeLeftRight(DBVHNode &node) {
    if (isNodeRight(*node.leftChild)) {
        node.rightChild = (node.leftChild)->rightChild;
        node.maxDepthRight =
                std::max(node.rightChild->maxDepthLeft, node.rightChild->maxDepthRight) + 1;
    } else {
        node.rightLeaf = (node.leftChild)->rightLeaf;
        node.maxDepthRight = 1;
    }
}

void DBVHv2::swapChildLeftRight(DBVHNode &node, const Rotations &rotations) {
    auto buffer = node.rightChild;
    setNodeLeftRight(node);
    (node.leftChild)->boundingBox = rotations.swapLeftRightToRight;
    (node.leftChild)->maxDepthRight = std::max(buffer->maxDepthLeft, buffer->maxDepthRight) + 1;
    (node.leftChild)->rightChild = buffer;
}

void DBVHv2::swapLeafLeftRight(DBVHNode &node, const Rotations &rotations) {
    auto buffer = node.rightLeaf;
    setNodeLeftRight(node);
    (node.leftChild)->boundingBox = rotations.swapLeftRightToRight;
    (node.leftChild)->rightLeaf = buffer;
    (node.leftChild)->maxDepthRight = 1;
}

void DBVHv2::setSurfaceAreaLeftRight(DBVHNode &node, const Rotations &rotations, const double *SAHs) {
    node.surfaceArea = SAHs[SwapLeftRightToRight];
    node.leftChild->surfaceArea =
            rotations.right.sa + rotations.leftLeft.sa + rotations.swapLeftRightToRight.getSA();
}

void DBVHv2::swapLeftRightToRight(DBVHNode &node, const Rotations &rotations, const double *SAHs) {
    if (isNodeRight(node)) {
        swapChildLeftRight(node, rotations);
    } else {
        swapLeafLeftRight(node, rotations);
    }
    setSurfaceAreaLeftRight(node, rotations, SAHs);
}

void DBVHv2::setNodeRightLeft(DBVHNode &node) {
    if (isNodeLeft(*node.rightChild)) {
        node.leftChild = (node.rightChild)->leftChild;
        node.maxDepthLeft = std::max(node.leftChild->maxDepthLeft, node.leftChild->maxDepthRight) + 1;
    } else {
        node.leftLeaf = (node.rightChild)->leftLeaf;
        node.maxDepthLeft = 1;
    }
}

void DBVHv2::swapChildRightLeft(DBVHNode &node, const Rotations &rotations) {
    auto buffer = node.leftChild;
    setNodeRightLeft(node);
    (node.rightChild)->boundingBox = rotations.swapRightLeftToLeft;
    (node.rightChild)->maxDepthLeft = std::max(buffer->maxDepthLeft, buffer->maxDepthRight) + 1;
    (node.rightChild)->leftChild = buffer;
}

void DBVHv2::swapLeafRightLeft(DBVHNode &node, const Rotations &rotations) {
    auto buffer = node.leftLeaf;
    setNodeRightLeft(node);
    (node.rightChild)->boundingBox = rotations.swapRightLeftToLeft;
    (node.rightChild)->leftLeaf = buffer;
    (node.rightChild)->maxDepthLeft = 1;
}

void DBVHv2::setSurfaceAreaRightLeft(DBVHNode &node, const Rotations &rotations, const double *SAHs) {
    node.surfaceArea = SAHs[SwapRightLeftToLeft];
    node.rightChild->surfaceArea =
            rotations.left.sa + rotations.rightRight.sa + rotations.swapRightLeftToLeft.getSA();
}

void DBVHv2::swapRightLeft(DBVHNode &node, const Rotations &rotations, const double *SAHs) {
    if (isNodeLeft(node)) {
        swapChildRightLeft(node, rotations);
    } else {
        swapLeafRightLeft(node, rotations);
    }
    setSurfaceAreaRightLeft(node, rotations, SAHs);
}

void DBVHv2::setNodeRightRight(DBVHNode &node) {
    if (isNodeRight(*node.rightChild)) {
        node.leftChild = (node.rightChild)->rightChild;
        node.maxDepthLeft = std::max(node.leftChild->maxDepthLeft, node.leftChild->maxDepthRight) + 1;
    } else {
        node.leftLeaf = (node.rightChild)->rightLeaf;
        node.maxDepthLeft = 1;
    }
}

void DBVHv2::swapChildRightRight(DBVHNode &node, const Rotations &rotations) {
    auto buffer = node.leftChild;
    setNodeRightRight(node);
    (node.rightChild)->boundingBox = rotations.swapRightRightToLeft;
    (node.rightChild)->maxDepthRight = std::max(buffer->maxDepthLeft, buffer->maxDepthRight) + 1;
    (node.rightChild)->rightChild = buffer;
}

void DBVHv2::swapLeafRightRight(DBVHNode &node, const Rotations &rotations) {
    auto buffer = node.leftLeaf;
    setNodeRightRight(node);
    (node.rightChild)->boundingBox = rotations.swapRightRightToLeft;
    (node.rightChild)->rightLeaf = buffer;
    (node.rightChild)->maxDepthRight = 1;
}

void DBVHv2::setSurfaceAreaRightRight(DBVHNode &node, const Rotations &rotations, const double *SAHs) {
    node.surfaceArea = SAHs[SwapRightRightToLeft];
    node.rightChild->surfaceArea =
            rotations.left.sa + rotations.rightLeft.sa + rotations.swapRightRightToLeft.getSA();
}

void DBVHv2::swapRightRight(DBVHNode &node, const Rotations &rotations, const double *SAHs) {
    if (isNodeLeft(node)) {
        swapChildRightRight(node, rotations);
    } else {
        swapLeafRightRight(node, rotations);
    }
    setSurfaceAreaRightRight(node, rotations, SAHs);
}

bool DBVHv2::optimizeSAH(DBVHNode &node) {
    Rotations rotations;
    double SAHs[numberOfPossibleRotation];

    if (!getPossibleRotations(node, SAHs, rotations)) return false;

    SAHs[NoRotation] = node.surfaceArea;
    Rotation bestSAH = getBestSAH(SAHs);

    switch (bestSAH) {
        case NoRotation:
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

void DBVHv2::refit(DBVHNode &node) {
    node.boundingBox = BoundingBox();
    node.surfaceArea = 0;
    if (isNodeRight(node)) {
        auto rightChild = node.rightChild;
        refitChild(node, *rightChild);
        node.maxDepthRight = std::max(rightChild->maxDepthRight, rightChild->maxDepthLeft) + 1;
    } else {
        refitLeaf(node, *node.rightLeaf);
    }
    if (isNodeLeft(node)) {
        auto leftChild = node.leftChild;
        refitChild(node, *leftChild);
        node.maxDepthLeft = std::max(leftChild->maxDepthRight, leftChild->maxDepthLeft) + 1;
    } else {
        refitLeaf(node, *node.leftLeaf);
    }
    node.surfaceArea += node.boundingBox.getSA();
}

bool DBVHv2::removeRightLeftGrandChild(DBVHNode &currentNode, DBVHNode &child, const Intersectable &object) {
    auto grandChild = child.leftLeaf;
    if (*grandChild != object) return false;

    if (isLeafRight(child)) {
        currentNode.rightLeaf = child.rightLeaf;
    } else {
        currentNode.rightChild = child.rightChild;
    }
    refit(currentNode);
    return true;
}

bool DBVHv2::removeRightRightGrandChild(DBVHNode &currentNode, DBVHNode &child, const Intersectable &object) {
    auto grandChild = child.rightLeaf;
    if (*grandChild != object) return false;

    if (isLeafLeft(child)) {
        currentNode.rightLeaf = child.leftLeaf;
    } else {
        currentNode.rightChild = child.leftChild;
    }
    refit(currentNode);
    return true;
}

bool DBVHv2::removeLeftLeftGrandChild(DBVHNode &currentNode, DBVHNode &child, const Intersectable &object) {
    auto grandChild = child.leftLeaf;
    if (*grandChild != object) return false;

    if (isLeafRight(child)) {
        currentNode.leftLeaf = child.rightLeaf;
    } else {
        currentNode.leftChild = child.rightChild;
    }
    refit(currentNode);
    return true;
}

bool DBVHv2::removeLeftRightGrandChild(DBVHNode &currentNode, DBVHNode &child, const Intersectable &object) {
    auto grandChild = child.rightLeaf;
    if (*grandChild != object) return false;

    if (isLeafLeft(child)) {
        currentNode.leftLeaf = child.leftLeaf;
    } else {
        currentNode.leftChild = child.leftChild;
    }
    refit(currentNode);
    return true;
}

bool DBVHv2::removeRightLeaf(DBVHNode &currentNode, const Intersectable &object) {
    if (isLeafRight(currentNode)) return true;
    auto child = currentNode.rightChild;
    if (!contains(child->boundingBox, object.getBoundaries())) return false;

    bool removed = false;
    if (isLeafLeft(*child)) {
        removed = removeRightLeftGrandChild(currentNode, *child, object);
    }
    if (isLeafRight(*child) && !removed) {
        removed = removeRightRightGrandChild(currentNode, *child, object);
    }
    if (removed) {
        tree.remove(*currentNode.rightChild);
    }
    return removed;
}

bool DBVHv2::removeLeftLeaf(DBVHNode &currentNode, const Intersectable &object) {
    if (isLeafLeft(currentNode)) return true;
    auto child = currentNode.leftChild;
    if (!contains(child->boundingBox, object.getBoundaries())) return false;

    bool removed = false;
    if (isLeafLeft(*child)) {
        removed = removeLeftLeftGrandChild(currentNode, *child, object);
    }
    if (isLeafRight(*child) && !removed) {
        removed = removeLeftRightGrandChild(currentNode, *child, object);
    }
    if (removed) {
        tree.remove(*currentNode.leftChild);
    }
    return removed;
}

void DBVHv2::remove(DBVHNode &currentNode, const Intersectable &object) {
    // find object in tree by insertion
    // remove object and refit nodes going the tree back up
    if (!contains(currentNode.boundingBox, object.getBoundaries())) return;
    if (removeLeftLeaf(currentNode, object) || removeRightLeaf(currentNode, object)) return;
    remove(*currentNode.leftChild, object);
    remove(*currentNode.rightChild, object);

    refit(currentNode);
    optimizeSAH(currentNode);
}

void DBVHv2::replaceRootWithChild(DBVHNode &child) {
    if (isLeafLeft(child)) {
        root->leftLeaf = child.leftLeaf;
    } else {
        tree.remove(*root->leftChild);
        root->leftChild = child.leftChild;
    }
    if (isLeafRight(child)) {
        root->rightLeaf = child.rightLeaf;
    } else {
        tree.remove(*root->rightChild);
        root->rightChild = child.rightChild;
    }

    root->maxDepthLeft = child.maxDepthLeft;
    root->maxDepthRight = child.maxDepthRight;
    root->boundingBox = child.boundingBox;
    root->surfaceArea = child.surfaceArea;
}

bool DBVHv2::removeSpecialCases(const Intersectable &object) {
    if (isLastElement(*root)) {
        if (*root->leftLeaf == object) {
            removeLastChild(*root);
            return true;
        }
    } else if (isLeafLeft(*root) && *root->leftLeaf == object) {
        if (isLeafRight(*root)) {
            removeSecondToLastChildLeft(*root);
        } else {
            replaceRootWithChild(*root->rightChild);
        }
        return true;
    } else if (isLeafRight(*root) && *root->rightLeaf == object) {
        if (isLeafLeft(*root)) {
            removeSecondToLastChildRight(*root);
        } else {
            replaceRootWithChild(*root->leftChild);
        }
        return true;
    }
    return false;
}

void DBVHv2::moveParentToNewParentsLeftChild(DBVHNode &node) {
    auto newNode = tree.newNode();
    *newNode = node;
    node.maxDepthLeft = std::max(newNode->maxDepthLeft, newNode->maxDepthRight) + 1;
    node.boundingBox = newNode->boundingBox;
    node.leftChild = newNode;
    node.maxDepthRight = 0;
}

void DBVHv2::sortObjectsIntoBoxes(const SplitOperation splitOperation, const Vector3D &splittingPlane, DBVHNode &node,
                                  const std::vector<Intersectable *> &objects,
                                  std::vector<Intersectable *> &leftObjects,
                                  std::vector<Intersectable *> &rightObjects) {
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

void DBVHv2::createLeftChild(DBVHNode &node) {
    auto child = tree.newNode();
    node.leftChild = child;
    node.maxDepthLeft = 2;
}

void DBVHv2::createNewParentForLeftLeafs(DBVHNode &node, const std::vector<Intersectable *> &leftObjects) {
    auto buffer = node.leftLeaf;
    auto parent = tree.newNode();
    parent->leftLeaf = buffer;
    parent->rightLeaf = leftObjects.at(0);
    parent->maxDepthLeft = 1;
    parent->maxDepthRight = 1;
    refit(*parent);
    node.leftChild = parent;
    node.maxDepthLeft = 2;
}

void DBVHv2::createNewParentForLeftChildren(DBVHNode &node) {
    auto buffer = node.leftLeaf;
    auto parent = tree.newNode();
    parent->leftLeaf = buffer;
    parent->boundingBox = buffer->getBoundaries();
    parent->surfaceArea = parent->boundingBox.getSA() * 2;
    parent->maxDepthLeft = 1;
    node.leftChild = parent;
    node.maxDepthLeft = 2;
}

bool DBVHv2::insertSingleObjectLeft(DBVHNode &node, const std::vector<Intersectable *> &leftObjects) {
    if (isEmptyLeft(node)) {
        createLeftLeaf(node, leftObjects);
    } else if (isLeafLeft(node)) {
        createNewParentForLeftLeafs(node, leftObjects);
    } else {
        return false;
    }
    return true;
}

void DBVHv2::createChildNodeLeft(DBVHNode &node) {
    if (isEmptyLeft(node)) {
        createLeftChild(node);
    } else if (isLeafLeft(node)) {
        createNewParentForLeftChildren(node);
    }
}

bool DBVHv2::passObjectsToLeftChild(DBVHNode &node, const std::vector<Intersectable *> &leftObjects) {
    if (leftObjects.size() == 1) {
        return insertSingleObjectLeft(node, leftObjects);
    } else if (!leftObjects.empty()) {
        createChildNodeLeft(node);
        return false;
    }
    return true;
}

void DBVHv2::createRightChild(DBVHNode &node) {
    auto child = tree.newNode();
    node.rightChild = child;
    node.maxDepthRight = 2;
}

void DBVHv2::createNewParentForRightLeafs(DBVHNode &node, const std::vector<Intersectable *> &rightObjects) {
    auto buffer = node.rightLeaf;
    auto parent = tree.newNode();
    parent->leftLeaf = buffer;
    parent->rightLeaf = rightObjects.at(0);
    parent->maxDepthLeft = 1;
    parent->maxDepthRight = 1;
    refit(*parent);
    node.rightChild = parent;
    node.maxDepthRight = 2;
}

void DBVHv2::createNewParentForRightChildren(DBVHNode &node) {
    auto buffer = node.rightLeaf;
    auto parent = tree.newNode();
    parent->rightLeaf = buffer;
    parent->boundingBox = buffer->getBoundaries();
    parent->surfaceArea = parent->boundingBox.getSA() * 2;
    parent->maxDepthRight = 1;
    node.rightChild = parent;
    node.maxDepthRight = 2;
}

bool DBVHv2::insertSingleObjectRight(DBVHNode &node, const std::vector<Intersectable *> &rightObjects) {
    if (isEmptyRight(node)) {
        createRightLeaf(node, rightObjects);
    } else if (isLeafRight(node)) {
        createNewParentForRightLeafs(node, rightObjects);
    } else {
        return false;
    }
    return true;
}

void DBVHv2::createChildNodeRight(DBVHNode &node) {
    if (isEmptyRight(node)) {
        createRightChild(node);
    } else if (isLeafRight(node)) {
        createNewParentForRightChildren(node);
    }
}

bool DBVHv2::passObjectsToRightChild(DBVHNode &node, const std::vector<Intersectable *> &rightObjects) {
    if (rightObjects.size() == 1) {
        return insertSingleObjectRight(node, rightObjects);
    } else if (!rightObjects.empty()) {
        createChildNodeRight(node);
        return false;
    }
    return true;
}

void DBVHv2::setBoxesAndHitProbability(const DBVHNode &node, BoundingBox &leftChildBox, BoundingBox &rightChildBox,
                                       double &pLeft, double &pRight) {
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

double DBVHv2::computeSAHWithNewParent(const DBVHNode &node, const BoundingBox &aabbLeft, const BoundingBox &aabbRight,
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

    ::refit(newLeftNewRight, newRight);
    ::refit(oldLeftNewLeft, newLeft);
    ::refit(oldLeftNewRight, newRight);
    ::refit(oldLeftNewLeftNewRight, newLeftNewRight);
    ::refit(oldLeftOldRight, oldRight);
    ::refit(oldRightNewLeft, newLeft);
    ::refit(oldRightNewRight, newRight);
    ::refit(oldLeftOldRightNewLeft, oldRightNewLeft);
    ::refit(oldLeftOldRightNewRight, oldRightNewRight);
    ::refit(oldRightNewLeftNewRight, newLeftNewRight);

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

double DBVHv2::evaluateBucket(const DBVHNode &node, const std::vector<Intersectable *> &objects,
                              const Vector3D &splittingPlane, SplitOperation &newParent) {
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

std::vector<double> DBVHv2::evaluateSplittingPlanes(const DBVHNode &node, const std::vector<Intersectable *> &objects,
                                                    const std::vector<Vector3D> &splittingPlanes,
                                                    std::vector<SplitOperation> &newParent) {
    std::vector<double> SAH(numberOfSplittingPlanes);

    for (int i = 0; i < numberOfSplittingPlanes; i++) {
        SAH[i] = evaluateBucket(node, objects, splittingPlanes[i], newParent[i]);
    }

    return SAH;
}

void DBVHv2::add(DBVHNode &currentNode, const std::vector<Intersectable *> &objects, uint8_t depth) {
    BoundingBox bBox = currentNode.boundingBox;
    ::refit(bBox, objects, 0);

    // create split buckets
    auto splittingPlanes = createSplittingPlanes(bBox);

    std::vector<SplitOperation> newParent(numberOfSplittingPlanes);

    // evaluate split buckets
    auto SAH = evaluateSplittingPlanes(currentNode, objects, splittingPlanes, newParent);

    // choose best split bucket and split currentNode accordingly
    int bestSplittingPlane = getBestSplittingPlane(SAH);

    std::vector<Intersectable *> leftObjects;
    std::vector<Intersectable *> rightObjects;

    if (!bestSplittingPlaneExists(bestSplittingPlane)) {
        splitEven(objects, leftObjects, rightObjects);
    } else {
        sortObjectsIntoBoxes(newParent[bestSplittingPlane], splittingPlanes[bestSplittingPlane], currentNode,
                             objects, leftObjects, rightObjects);
    }

    // pass objects to children
    if (!passObjectsToLeftChild(currentNode, leftObjects)) {
        add(*currentNode.leftChild, leftObjects, depth + 1);
    }
    if (!passObjectsToRightChild(currentNode, rightObjects)) {
        add(*currentNode.rightChild, rightObjects, depth + 1);
    }

    // calculate surface area and tree depth going the tree back up
    refit(currentNode);

    // use tree rotations going the tree back up to optimize SAH
    optimizeSAH(currentNode);
}

bool DBVHv2::addOntoSingleElement(const std::vector<Intersectable *> &objects) {
    if (!isLastElement(*root)) return false;
    std::vector<Intersectable *> newObjects{objects};     // TODO: make more efficient
    newObjects.push_back(root->leftLeaf);
    root->maxDepthLeft = 0;
    add(*root, newObjects, 1);
    return true;
}

DBVHv2::DBVHv2() : tree(FlatTree(4096)), root(tree.newNode()) {

}

DBVHv2::DBVHv2(DBVHv2 &&other) noexcept: tree(std::move(other.tree)), root(other.root) {

}

DBVHv2 &DBVHv2::operator=(DBVHv2 &&other) noexcept {
    tree = std::move(other.tree);
    root = other.root;
    return *this;
}

DBVHv2::DBVHv2(const std::vector<Intersectable *> &objects) : tree(FlatTree(4096)), root(tree.newNode()) {
    addObjects(objects);
}

void DBVHv2::addObjects(const std::vector<Intersectable *> &objects) {
    if (objects.empty() || addFirstAndOnlyElement(*root, objects) || addOntoSingleElement(objects))
        return;

    add(*root, objects, 1);
}

void DBVHv2::removeObjects(const std::vector<Intersectable *> &objects) {
    for (const auto &object: objects) {
        if (isEmpty(*root)) return;
        if (!removeSpecialCases(*object)) {
            remove(*root, *object);
        }
    }
}

bool DBVHv2::intersectFirst(IntersectionInfo &intersectionInfo, const Ray &ray) {
    if (isEmpty(*root))
        return false;
    if (isLastElement(*root))
        return root->leftLeaf->intersectFirst(intersectionInfo, ray);

    return traverseFirst(*root, intersectionInfo, ray);
}

bool DBVHv2::intersectAny(IntersectionInfo &intersectionInfo, const Ray &ray) {
    if (isEmpty(*root))
        return false;
    if (isLastElement(*root))
        return root->leftLeaf->intersectAny(intersectionInfo, ray);

    return traverseAny(*root, intersectionInfo, ray);
}

bool DBVHv2::intersectAll(std::vector<IntersectionInfo> &intersectionInfo, const Ray &ray) {
    if (isEmpty(*root))
        return false;
    if (isLastElement(*root))
        return root->leftLeaf->intersectAll(intersectionInfo, ray);

    return traverseALl(*root, intersectionInfo, ray);
}

BoundingBox DBVHv2::getBoundaries() const {
    return root->boundingBox;
}

double DBVHv2::getSurfaceArea() const {
    return root->surfaceArea;
}
