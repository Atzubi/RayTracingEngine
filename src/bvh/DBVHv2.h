#pragma once

#include "RayTraceEngine/Intersectable.h"
#include "cache/Cache.h"
#include "flat_tree/FlatTree.h"
#include <unordered_map>

enum SplitOperation
{
    Default,
    DefaultWrongOrder,
    AllNewLeft,
    AllNewRight,
    SplitOldNew,
    DefaultOldLeft,
    DefaultWrongOrderOldLeft
};

struct BoxSA
{
    BoundingBox box;
    double      sa = 0;
};

struct Rotations
{
    BoxSA       left;
    BoxSA       right;
    BoxSA       leftLeft;
    BoxSA       leftRight;
    BoxSA       rightRight;
    BoxSA       rightLeft;
    BoundingBox swapLeftLeftToRight;
    BoundingBox swapLeftRightToRight;
    BoundingBox swapRightLeftToLeft;
    BoundingBox swapRightRightToLeft;
};

class DBVHv2 : public IIntersectable
{
  private:
    FlatTree  tree;
    DBVHNode* root;
    // Cache<DBVHNode *, DBVHNode> cache;

    void replaceRootWithChild(DBVHNode& child);

    bool removeSpecialCases(const IIntersectable& object);

    void remove(DBVHNode& currentNode, const IIntersectable& object);

    bool removeLeftLeaf(DBVHNode& currentNode, const IIntersectable& object);

    bool removeRightLeaf(DBVHNode& currentNode, const IIntersectable& object);

    static bool removeLeftRightGrandChild(DBVHNode& currentNode, DBVHNode& child, const IIntersectable& object);

    static bool removeLeftLeftGrandChild(DBVHNode& currentNode, DBVHNode& child, const IIntersectable& object);

    static bool removeRightRightGrandChild(DBVHNode& currentNode, DBVHNode& child, const IIntersectable& object);

    static bool removeRightLeftGrandChild(DBVHNode& currentNode, DBVHNode& child, const IIntersectable& object);

    static void refit(DBVHNode& node);

    void add(DBVHNode& currentNode, const std::vector<IIntersectable*>& objects, uint8_t depth);

    static std::vector<double> evaluateSplittingPlanes(const DBVHNode&                     node,
                                                       const std::vector<IIntersectable*>& objects,
                                                       const std::vector<Vector3D>&        splittingPlanes,
                                                       std::vector<SplitOperation>&        newParent);

    static double evaluateBucket(const DBVHNode&                     node,
                                 const std::vector<IIntersectable*>& objects,
                                 const Vector3D&                     splittingPlane,
                                 SplitOperation&                     newParent);

    static double computeSAHWithNewParent(const DBVHNode&    node,
                                          const BoundingBox& aabbLeft,
                                          const BoundingBox& aabbRight,
                                          double             objectCostLeft,
                                          double             objectCostRight,
                                          SplitOperation&    newParent);

    static void setBoxesAndHitProbability(const DBVHNode& node,
                                          BoundingBox&    leftChildBox,
                                          BoundingBox&    rightChildBox,
                                          double&         pLeft,
                                          double&         pRight);

    bool addOntoSingleElement(const std::vector<IIntersectable*>& objects);

    void createNewParentForRightChildren(DBVHNode& node);

    bool passObjectsToRightChild(DBVHNode& node, const std::vector<IIntersectable*>& rightObjects);

    void createNewParentForRightLeafs(DBVHNode& node, const std::vector<IIntersectable*>& rightObjects);

    void createRightChild(DBVHNode& node);

    void createChildNodeRight(DBVHNode& node);

    bool insertSingleObjectRight(DBVHNode& node, const std::vector<IIntersectable*>& rightObjects);

    bool passObjectsToLeftChild(DBVHNode& node, const std::vector<IIntersectable*>& leftObjects);

    void createChildNodeLeft(DBVHNode& node);

    bool insertSingleObjectLeft(DBVHNode& node, const std::vector<IIntersectable*>& leftObjects);

    void createNewParentForLeftChildren(DBVHNode& node);

    void createNewParentForLeftLeafs(DBVHNode& node, const std::vector<IIntersectable*>& leftObjects);

    void createLeftChild(DBVHNode& node);

    void moveParentToNewParentsLeftChild(DBVHNode(&node));

    void sortObjectsIntoBoxes(SplitOperation                      splitOperation,
                              const Vector3D&                     splittingPlane,
                              DBVHNode&                           node,
                              const std::vector<IIntersectable*>& objects,
                              std::vector<IIntersectable*>&       leftObjects,
                              std::vector<IIntersectable*>&       rightObjects);

    static bool optimizeSAH(DBVHNode& node);

    static void swapRightRight(DBVHNode& node, const Rotations& rotations, const double* SAHs);

    static void setSurfaceAreaRightRight(DBVHNode& node, const Rotations& rotations, const double* SAHs);

    static void swapLeafRightRight(DBVHNode& node, const Rotations& rotations);

    static void swapChildRightRight(DBVHNode& node, const Rotations& rotations);

    static void setNodeRightRight(DBVHNode& node);

    static void swapRightLeft(DBVHNode& node, const Rotations& rotations, const double* SAHs);

    static void setSurfaceAreaRightLeft(DBVHNode& node, const Rotations& rotations, const double* SAHs);

    static void swapLeafRightLeft(DBVHNode& node, const Rotations& rotations);

    static void swapChildRightLeft(DBVHNode& node, const Rotations& rotations);

    static void setNodeRightLeft(DBVHNode& node);

    static void swapLeftRightToRight(DBVHNode& node, const Rotations& rotations, const double* SAHs);

    static void setSurfaceAreaLeftRight(DBVHNode& node, const Rotations& rotations, const double* SAHs);

    static void swapLeafLeftRight(DBVHNode& node, const Rotations& rotations);

    static void swapChildLeftRight(DBVHNode& node, const Rotations& rotations);

    static void setNodeLeftRight(DBVHNode& node);

    static void swapLeftLeftToRight(DBVHNode& node, const Rotations& rotations, const double* SAHs);

    static void setSurfaceAreaLeftLeft(DBVHNode& node, const Rotations& rotations, const double* SAHs);

    static void swapLeafLeftLeft(DBVHNode& node, const Rotations& rotations);

    static void swapChildLeftLeft(DBVHNode& node, const Rotations& rotations);

    static void setNodeLeftLeft(DBVHNode& node);

    static bool getPossibleRotations(DBVHNode& node, double* SAHs, Rotations& rotations);

    static void getRotations(DBVHNode& node, double* SAHs, Rotations& rotations);

    static void getRotationsFull(const DBVHNode& node, double* SAHs, Rotations& rotations);

    static bool getRotationsRight(DBVHNode& node, double* SAHs, Rotations& rotations);

    static void fillRightRotationBoxes(const DBVHNode& node, Rotations& rotations);

    static void fillRotationBoxes(BoxSA& left, BoxSA& right, const DBVHNode& node);

  public:
    DBVHv2();

    DBVHv2(DBVHv2&& other) noexcept;

    DBVHv2& operator=(DBVHv2&& other) noexcept;

    explicit DBVHv2(const std::vector<IIntersectable*>& objects);

    void addObjects(const std::vector<IIntersectable*>& objects);

    void removeObjects(const std::vector<IIntersectable*>& objects);

    bool IntersectFirst(IntersectionInfo& intersectionInfo, const Ray& ray) const override;

    bool IntersectAny(IntersectionInfo& intersectionInfo, const Ray& ray) const override;

    bool IntersectAll(std::vector<IntersectionInfo>& intersectionInfo, const Ray& ray) const override;

    std::unique_ptr<IIntersectable> Clone() const override;

    [[nodiscard]] BoundingBox GetBoundaries() const override;

    [[nodiscard]] double GetSurfaceArea() const override;

    bool operator==(const IIntersectable& object) const override;

    bool operator!=(const IIntersectable& object) const override;
};
