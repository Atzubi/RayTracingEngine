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

struct TraversalContainer {
    const DBVHNode *node;
    double distance;
};

class DBVHv2 {
private:


    FlatTree tree;
    DBVHNode *root;
    //Cache<DBVHNode *, DBVHNode> cache;

    void replaceRootWithChild(DBVHNode &child);

    bool removeSpecialCases(const Intersectable &object);

    bool traverseALl(std::vector<IntersectionInfo> &intersectionInfo, const Ray &ray);

    static void
    processTraversalStack(std::vector<IntersectionInfo> &intersectionInfo, const Ray &ray, const DBVHNode **stack);

    static inline void
    getChildrenIntersection(std::vector<IntersectionInfo> &intersectionInfo, const Ray &ray, const DBVHNode **stack,
                            uint64_t &stackPointer);

    bool traverseAny(IntersectionInfo &intersectionInfo, const Ray &ray);

    static bool processTraversalStack(IntersectionInfo &intersectionInfo, const Ray &ray, const DBVHNode **stack);

    static inline bool
    getChildrenIntersection(IntersectionInfo &intersectionInfo, const Ray &ray, const DBVHNode **stack,
                            uint64_t &stackPointer);

    bool traverseFirst(IntersectionInfo &intersectionInfo, const Ray &ray);

    static bool processTraversalStack(IntersectionInfo &intersectionInfo, const Ray &ray, TraversalContainer *stack);

    static inline void
    pushIntersectionsOnStack(const DBVHNode &node, double distanceRight, double distanceLeft, bool right, bool left,
                             TraversalContainer *stack, uint64_t &stackPointer);

    static inline void
    getChildrenIntersections(const Ray &ray, const DBVHNode &node, IntersectionInfo &intersectionInfo,
                             double &distanceRight, double &distanceLeft, bool &right, bool &left,
                             bool &hit);

    void remove(DBVHNode &currentNode, const Intersectable &object);

    bool removeLeftLeaf(DBVHNode &currentNode, const Intersectable &object);

    bool removeRightLeaf(DBVHNode &currentNode, const Intersectable &object);

    static bool removeLeftRightGrandChild(DBVHNode &currentNode, DBVHNode &child, const Intersectable &object);

    static bool removeLeftLeftGrandChild(DBVHNode &currentNode, DBVHNode &child, const Intersectable &object);

    static bool removeRightRightGrandChild(DBVHNode &currentNode, DBVHNode &child, const Intersectable &object);

    static bool removeRightLeftGrandChild(DBVHNode &currentNode, DBVHNode &child, const Intersectable &object);

    static void refit(DBVHNode &node);

    void add(DBVHNode &currentNode, const std::vector<Intersectable *> &objects, uint8_t depth);

    static std::vector<double>
    evaluateSplittingPlanes(const DBVHNode &node, const std::vector<Intersectable *> &objects,
                            const std::vector<Vector3D> &splittingPlanes,
                            std::vector<SplitOperation> &newParent);

    static double
    evaluateBucket(const DBVHNode &node, const std::vector<Intersectable *> &objects, const Vector3D &splittingPlane,
                   SplitOperation &newParent);

    static double
    computeSAHWithNewParent(const DBVHNode &node, const BoundingBox &aabbLeft, const BoundingBox &aabbRight,
                            double objectCostLeft, double objectCostRight, SplitOperation &newParent);

    static void
    setBoxesAndHitProbability(const DBVHNode &node, BoundingBox &leftChildBox, BoundingBox &rightChildBox,
                              double &pLeft,
                              double &pRight);

    bool addOntoSingleElement(const std::vector<Intersectable *> &objects);

    void createNewParentForRightChildren(DBVHNode &node);

    bool passObjectsToRightChild(DBVHNode &node, const std::vector<Intersectable *> &rightObjects);

    void createNewParentForRightLeafs(DBVHNode &node, const std::vector<Intersectable *> &rightObjects);

    void createRightChild(DBVHNode &node);

    void createChildNodeRight(DBVHNode &node);

    bool insertSingleObjectRight(DBVHNode &node, const std::vector<Intersectable *> &rightObjects);

    bool passObjectsToLeftChild(DBVHNode &node, const std::vector<Intersectable *> &leftObjects);

    void createChildNodeLeft(DBVHNode &node);

    bool insertSingleObjectLeft(DBVHNode &node, const std::vector<Intersectable *> &leftObjects);

    void createNewParentForLeftChildren(DBVHNode &node);

    void createNewParentForLeftLeafs(DBVHNode &node, const std::vector<Intersectable *> &leftObjects);

    void createLeftChild(DBVHNode &node);

    void moveParentToNewParentsLeftChild(DBVHNode( &node));

    void sortObjectsIntoBoxes(SplitOperation splitOperation, const Vector3D &splittingPlane, DBVHNode &node,
                              const std::vector<Intersectable *> &objects,
                              std::vector<Intersectable *> &leftObjects,
                              std::vector<Intersectable *> &rightObjects);

    static bool optimizeSAH(DBVHNode &node);

    static void swapRightRight(DBVHNode &node, const Rotations &rotations, const double *SAHs);

    static void setSurfaceAreaRightRight(DBVHNode &node, const Rotations &rotations, const double *SAHs);

    static void swapLeafRightRight(DBVHNode &node, const Rotations &rotations);

    static void swapChildRightRight(DBVHNode &node, const Rotations &rotations);

    static void setNodeRightRight(DBVHNode &node);

    static void swapRightLeft(DBVHNode &node, const Rotations &rotations, const double *SAHs);

    static void setSurfaceAreaRightLeft(DBVHNode &node, const Rotations &rotations, const double *SAHs);

    static void swapLeafRightLeft(DBVHNode &node, const Rotations &rotations);

    static void swapChildRightLeft(DBVHNode &node, const Rotations &rotations);

    static void setNodeRightLeft(DBVHNode &node);

    static void swapLeftRightToRight(DBVHNode &node, const Rotations &rotations, const double *SAHs);

    static void setSurfaceAreaLeftRight(DBVHNode &node, const Rotations &rotations, const double *SAHs);

    static void swapLeafLeftRight(DBVHNode &node, const Rotations &rotations);

    static void swapChildLeftRight(DBVHNode &node, const Rotations &rotations);

    static void setNodeLeftRight(DBVHNode &node);

    static void swapLeftLeftToRight(DBVHNode &node, const Rotations &rotations, const double *SAHs);

    static void setSurfaceAreaLeftLeft(DBVHNode &node, const Rotations &rotations, const double *SAHs);

    static void swapLeafLeftLeft(DBVHNode &node, const Rotations &rotations);

    static void swapChildLeftLeft(DBVHNode &node, const Rotations &rotations);

    static void setNodeLeftLeft(DBVHNode &node);

    static bool getPossibleRotations(DBVHNode &node, double *SAHs, Rotations &rotations);

    static void getRotations(DBVHNode &node, double *SAHs, Rotations &rotations);

    static void getRotationsFull(const DBVHNode &node, double *SAHs, Rotations &rotations);

    static bool getRotationsRight(DBVHNode &node, double *SAHs, Rotations &rotations);

    static void fillRightRotationBoxes(const DBVHNode &node, Rotations &rotations);

    static void fillRotationBoxes(BoxSA &left, BoxSA &right, const DBVHNode &node);

public:
    DBVHv2();

    DBVHv2(DBVHv2 &&other) noexcept;

    DBVHv2 &operator=(DBVHv2 &&other) noexcept;

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