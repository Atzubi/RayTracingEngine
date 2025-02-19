#pragma once

#include "DBVHNode.h"
#include "RayTraceEngine/Intersectable.h"
#include <span>
#include <utility>
#include <vector>

class DBVHv2 : public IIntersectable
{
  public:
    DBVHv2();

    DBVHv2(DBVHv2&& other) noexcept;

    DBVHv2& operator=(DBVHv2&& other) noexcept;

    explicit DBVHv2(const std::vector<const IIntersectable*>& objects);

    void AddObjects(const std::vector<const IIntersectable*>& objects);

    void RemoveObjects(const std::vector<const IIntersectable*>& objects);

    bool IntersectFirst(IntersectionInfo& intersectionInfo, const Ray& ray) const override;

    bool IntersectAny(IntersectionInfo& intersectionInfo, const Ray& ray) const override;

    bool IntersectAll(std::vector<IntersectionInfo>& intersectionInfo, const Ray& ray) const override;

    std::unique_ptr<IIntersectable> Clone() const override;

    [[nodiscard]] BoundingBox GetBoundaries() const override;

    [[nodiscard]] float GetSurfaceArea() const override;

    bool operator==(const IIntersectable& object) const override;

    bool operator!=(const IIntersectable& object) const override;

  private:
    static constexpr int NumberOfSplittingPlanes = 9;

    enum SplitOperation : std::uint32_t
    {
        Default,
        DefaultWrongOrder,
        AllNewLeft,
        AllNewRight,
        SplitOldNew,
        DefaultOldLeft,
        DefaultWrongOrderOldLeft,
        Count
    };

    struct BoxSA
    {
        BoundingBox box;
        float       sa = 0;
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

    struct Metadata
    {
        std::uint32_t parent;
        float         surfaceArea;
    };

    void Add(std::uint32_t currentNode, const std::vector<const IIntersectable*>& objects, uint8_t depth);

    bool AddFirstAndOnlyElement(DBVHNode& root, const std::vector<const IIntersectable*>& objects);

    bool AddOntoSingleElement(const std::vector<const IIntersectable*>& objects);

    void MoveParentToNewParentsLeftChild(std::uint32_t);

    void SortObjectsIntoBoxes(SplitOperation                            splitOperation,
                              const Vector3D&                           splittingPlane,
                              std::uint32_t                             node,
                              const std::vector<const IIntersectable*>& objects,
                              std::vector<const IIntersectable*>&       leftObjects,
                              std::vector<const IIntersectable*>&       rightObjects);

    std::pair<std::vector<const IIntersectable*>, std::vector<const IIntersectable*>>
    Split(const std::vector<const IIntersectable*>& objects, std::uint32_t currentNode);

    std::array<float, NumberOfSplittingPlanes>
    EvaluateSplittingPlanes(const DBVHNode&                                      node,
                            const std::vector<const IIntersectable*>&            objects,
                            const std::array<Vector3D, NumberOfSplittingPlanes>& splittingPlanes,
                            std::array<SplitOperation, NumberOfSplittingPlanes>& newParent) const;

    float EvaluateBucket(const DBVHNode&                           node,
                         const std::vector<const IIntersectable*>& objects,
                         const Vector3D&                           splittingPlane,
                         SplitOperation&                           newParent) const;

    float ComputeSAHWithNewParent(const DBVHNode&    node,
                                  const BoundingBox& aabbLeft,
                                  const BoundingBox& aabbRight,
                                  float              objectCostLeft,
                                  float              objectCostRight,
                                  SplitOperation&    newParent) const;

    void SetBoxesAndHitProbability(const DBVHNode& node,
                                   BoundingBox&    leftChildBox,
                                   BoundingBox&    rightChildBox,
                                   float&          pLeft,
                                   float&          pRight) const;

    static SplitOperation GetBestSplitOperation(std::span<const float> SAHs);

    std::array<Vector3D, DBVHv2::NumberOfSplittingPlanes> CreateSplittingPlanes(const BoundingBox& bBox);

    int GetBestSplittingPlane(const std::array<float, NumberOfSplittingPlanes>& SAH);

    bool OptimizeSAH(std::uint32_t node);

    void SwapRightRightToLeft(std::uint32_t node, const Rotations& rotations, std::span<const float> SAHs);

    void SwapRightLeftToLeft(std::uint32_t node, const Rotations& rotations, std::span<const float> SAHs);

    void SwapLeftRightToRight(std::uint32_t node, const Rotations& rotations, std::span<const float> SAHs);

    void SwapLeftLeftToRight(std::uint32_t node, const Rotations& rotations, std::span<const float> SAHs);

    bool GetPossibleRotations(std::uint32_t node, std::span<float> SAHs, Rotations& rotations) const;

    void FillRotationBoxes(BoxSA& left, BoxSA& right, std::uint32_t node) const;

    bool RemoveSpecialCases(const IIntersectable& object);

    void Remove(std::uint32_t currentNode, const IIntersectable& object);

    bool RemoveLeftLeaf(std::uint32_t currentNode, const IIntersectable& object);

    bool RemoveRightLeaf(std::uint32_t currentNode, const IIntersectable& object);

    void RemoveNode(std::uint32_t node);

    void RemoveLeaf(std::uint32_t leaf);

    void Refit(std::uint32_t node);

    std::vector<DBVHNode>              flatTree_;
    std::vector<const IIntersectable*> leaves_;
    std::vector<Metadata>              nodeMetadata_;
    std::vector<Metadata>              leafMetadata_;
};
