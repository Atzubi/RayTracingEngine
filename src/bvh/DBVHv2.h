//
// Created by Sebastian on 02.12.2021.
//

#ifndef RAYTRACEENGINE_DBVHV2_H
#define RAYTRACEENGINE_DBVHV2_H

#include <unordered_map>
#include "RayTraceEngine/Intersectable.h"
#include "flat_tree/FlatTree.h"
#include "cache/Cache.h"

enum SplitOperation {
    Default, DefaultWrongOrder, AllNewLeft, AllNewRight, SplitOldNew, DefaultOldLeft, DefaultWrongOrderOldLeft
};

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

class DBVHv2 {
private:
    struct TraversalContainer {
        const DBVHNode *node;
        double distance;
    };

    FlatTree tree;
    uint64_t rootPosition;
    Cache<DBVHNode *, DBVHNode> cache;

    void replaceRootWithChild(DBVHNode &child);

    bool removeSpecialCases(const Intersectable &object);

    bool traverseALl(const DBVHNode &root, std::vector<IntersectionInfo> &intersectionInfo, const Ray &ray);

    void processTraversalStack(std::vector<IntersectionInfo> &intersectionInfo, const Ray &ray, const DBVHNode **stack);

    inline void
    getChildrenIntersection(std::vector<IntersectionInfo> &intersectionInfo, const Ray &ray, const DBVHNode **stack,
                            uint64_t &stackPointer);

    bool traverseAny(const DBVHNode &root, IntersectionInfo &intersectionInfo, const Ray &ray);

    bool processTraversalStack(IntersectionInfo &intersectionInfo, const Ray &ray, const DBVHNode **stack);

    inline bool
    getChildrenIntersection(IntersectionInfo &intersectionInfo, const Ray &ray, const DBVHNode **stack,
                            uint64_t &stackPointer);

    bool traverseFirst(const DBVHNode &root, IntersectionInfo &intersectionInfo, const Ray &ray);

    bool processTraversalStack(IntersectionInfo &intersectionInfo, const Ray &ray, TraversalContainer *stack);

    inline void
    pushIntersectionsOnStack(const DBVHNode &node, double distanceRight, double distanceLeft, bool right, bool left,
                             TraversalContainer *stack, uint64_t &stackPointer);

    inline void getChildrenIntersections(const Ray &ray, const DBVHNode &node, IntersectionInfo &intersectionInfo,
                                         double &distanceRight, double &distanceLeft, bool &right, bool &left,
                                         bool &hit);

    void remove(DBVHNode &currentNode, const Intersectable &object);

    bool removeLeftLeaf(DBVHNode &currentNode, const Intersectable &object);

    bool removeRightLeaf(DBVHNode &currentNode, const Intersectable &object);

    bool removeLeftRightGrandChild(DBVHNode &currentNode, DBVHNode &child, const Intersectable &object);

    bool removeLeftLeftGrandChild(DBVHNode &currentNode, DBVHNode &child, const Intersectable &object);

    bool removeRightRightGrandChild(DBVHNode &currentNode, DBVHNode &child, const Intersectable &object);

    bool removeRightLeftGrandChild(DBVHNode &currentNode, DBVHNode &child, const Intersectable &object);

    void refit(DBVHNode &node);

    void add(uint64_t nodePosition, const std::vector<Intersectable *> &objects, uint8_t depth);

    std::vector<double> evaluateSplittingPlanes(const DBVHNode &node, const std::vector<Intersectable *> &objects,
                                                const std::vector<Vector3D> &splittingPlanes,
                                                std::vector<SplitOperation> &newParent);

    double
    evaluateBucket(const DBVHNode &node, const std::vector<Intersectable *> &objects, const Vector3D &splittingPlane,
                   SplitOperation &newParent);

    double computeSAHWithNewParent(const DBVHNode &node, const BoundingBox &aabbLeft, const BoundingBox &aabbRight,
                                   double objectCostLeft, double objectCostRight, SplitOperation &newParent);

    void
    setBoxesAndHitProbability(const DBVHNode &node, BoundingBox &leftChildBox, BoundingBox &rightChildBox, double &pLeft,
                              double &pRight);

    bool addOntoSingleElement(const std::vector<Intersectable *> &objects);

    void createNewParentForRightChildren(uint64_t nodePosition);

    bool passObjectsToRightChild(uint64_t nodePosition, const std::vector<Intersectable *> &rightObjects);

    void createNewParentForRightLeafs(uint64_t nodePosition, const std::vector<Intersectable *> &rightObjects);

    void createRightChild(uint64_t nodePosition);

    void createChildNodeRight(uint64_t nodePosition);

    bool insertSingleObjectRight(uint64_t nodePosition, const std::vector<Intersectable *> &rightObjects);

    bool passObjectsToLeftChild(uint64_t nodePosition, const std::vector<Intersectable *> &leftObjects);

    void createChildNodeLeft(uint64_t nodePosition);

    bool insertSingleObjectLeft(uint64_t nodePosition, const std::vector<Intersectable *> &leftObjects);

    void createNewParentForLeftChildren(uint64_t nodePosition);

    void createNewParentForLeftLeafs(uint64_t nodePosition, const std::vector<Intersectable *> &leftObjects);

    void createLeftChild(uint64_t nodePosition);

    void moveParentToNewParentsLeftChild(DBVHNode( &node));

    void sortObjectsIntoBoxes(SplitOperation splitOperation, const Vector3D &splittingPlane, DBVHNode &node,
                              const std::vector<Intersectable *> &objects,
                              std::vector<Intersectable *> &leftObjects,
                              std::vector<Intersectable *> &rightObjects);

    bool optimizeSAH(DBVHNode &node);

    void swapRightRight(DBVHNode &node, const Rotations &rotations, const double *SAHs);

    void setSurfaceAreaRightRight(DBVHNode &node, const Rotations &rotations, const double *SAHs);

    void swapLeafRightRight(DBVHNode &node, const Rotations &rotations);

    void swapChildRightRight(DBVHNode &node, const Rotations &rotations);

    void setNodeRightRight(DBVHNode &node);

    void swapRightLeft(DBVHNode &node, const Rotations &rotations, const double *SAHs);

    void setSurfaceAreaRightLeft(DBVHNode &node, const Rotations &rotations, const double *SAHs);

    void swapLeafRightLeft(DBVHNode &node, const Rotations &rotations);

    void swapChildRightLeft(DBVHNode &node, const Rotations &rotations);

    void setNodeRightLeft(DBVHNode &node);

    void swapLeftRightToRight(DBVHNode &node, const Rotations &rotations, const double *SAHs);

    void setSurfaceAreaLeftRight(DBVHNode &node, const Rotations &rotations, const double *SAHs);

    void swapLeafLeftRight(DBVHNode &node, const Rotations &rotations);

    void swapChildLeftRight(DBVHNode &node, const Rotations &rotations);

    void setNodeLeftRight(DBVHNode &node);

    void swapLeftLeftToRight(DBVHNode &node, const Rotations &rotations, const double *SAHs);

    void setSurfaceAreaLeftLeft(DBVHNode &node, const Rotations &rotations, const double *SAHs);

    void swapLeafLeftLeft(DBVHNode &node, const Rotations &rotations);

    void swapChildLeftLeft(DBVHNode &node, const Rotations &rotations);

    void setNodeLeftLeft(DBVHNode &node);

    bool getPossibleRotations(DBVHNode &node, double *SAHs, Rotations &rotations);

    void getRotations(DBVHNode &node, double *SAHs, Rotations &rotations);

    void getRotationsFull(const DBVHNode &node, double *SAHs, Rotations &rotations);

    bool getRotationsRight(DBVHNode &node, double *SAHs, Rotations &rotations);

    void fillRightRotationBoxes(const DBVHNode &node, Rotations &rotations);

    void fillRotationBoxes(BoxSA &left, BoxSA &right, const DBVHNode &node);

public:
    DBVHv2();

    explicit DBVHv2(const std::vector<Intersectable *> &objects);

    void addObjects(const std::vector<Intersectable *> &objects);

    void removeObjects(const std::vector<Intersectable *> &objects);

    bool intersectFirst(IntersectionInfo &intersectionInfo, const Ray &ray);

    bool intersectAny(IntersectionInfo &intersectionInfo, const Ray &ray);

    bool intersectAll(std::vector<IntersectionInfo> &intersectionInfo, const Ray &ray);

    [[nodiscard]] BoundingBox getBoundaries() const;

    [[nodiscard]] double getSurfaceArea() const;
};

#endif //RAYTRACEENGINE_DBVHV2_H