#include "DBVH.h"

#include <limits>

namespace
{
    bool IsEmpty(const DBVHNode& root) { return root.maxDepthLeft == 0; }

    bool IsLastElement(const DBVHNode& root) { return root.maxDepthRight == 0 && !IsEmpty(root); }

    bool IsLeafLeft(const DBVHNode& root) { return root.maxDepthLeft == 1; }

    bool IsLeafRight(const DBVHNode& root) { return root.maxDepthRight == 1; }

    bool IsNodeLeft(const DBVHNode& root) { return root.maxDepthLeft > 1; }

    bool IsNodeRight(const DBVHNode& root) { return root.maxDepthRight > 1; }

    bool IsEmptyRight(const DBVHNode& root) { return root.maxDepthRight == 0; }

    bool IsEmptyLeft(const DBVHNode& root) { return IsEmpty(root); }

    void Refit(BoundingBox& target, const BoundingBox& resizeBy)
    {
        target.minCorner.x = std::min(target.minCorner.x, resizeBy.minCorner.x);
        target.minCorner.y = std::min(target.minCorner.y, resizeBy.minCorner.y);
        target.minCorner.z = std::min(target.minCorner.z, resizeBy.minCorner.z);
        target.maxCorner.x = std::max(target.maxCorner.x, resizeBy.maxCorner.x);
        target.maxCorner.y = std::max(target.maxCorner.y, resizeBy.maxCorner.y);
        target.maxCorner.z = std::max(target.maxCorner.z, resizeBy.maxCorner.z);
    }

    void Refit(BoundingBox& aabb, const IIntersectable& object) { Refit(aabb, object.GetBoundaries()); }

    void Refit(BoundingBox& aabb, const std::vector<const IIntersectable*>& objects, const float looseness)
    {
        // refit box to fit all objects
        for (const auto& object : objects)
        {
            Refit(aabb, *object);
        }

        // increase box size by looseness factor
        if (looseness > 0)
        {
            const auto scale = aabb.maxCorner - aabb.minCorner;
            aabb.minCorner -= scale * looseness;
            aabb.maxCorner += scale * looseness;
        }
    }

    int GetCurrentSplittingPlane(const Vector3D& splittingPlane)
    {
        for (int i = 0; i < 3; ++i)
        {
            if (splittingPlane[i] != 0)
            {
                return i;
            }
        }
        throw std::invalid_argument("The given vector does not define a splitting plane");
    }

    bool
    IsObjectInLeftSplit(const Vector3D& splittingPlane, const int currentSplittingPlane, const IIntersectable* object)
    {
        return (object->GetBoundaries().maxCorner[currentSplittingPlane] +
                object->GetBoundaries().minCorner[currentSplittingPlane]) /
                   2 <
               splittingPlane[currentSplittingPlane];
    }

    void SortObjectsIntoBuckets(const std::vector<const IIntersectable*>& objects,
                                const Vector3D&                           splittingPlane,
                                BoundingBox&                              aabbLeft,
                                BoundingBox&                              aabbRight,
                                float&                                    objectCostLeft,
                                float&                                    objectCostRight)
    {
        const auto currentSplittingPlane = GetCurrentSplittingPlane(splittingPlane);

        for (const auto& object : objects)
        {
            if (IsObjectInLeftSplit(splittingPlane, currentSplittingPlane, object))
            {
                Refit(aabbLeft, *object);
                objectCostLeft += object->GetSurfaceArea();
            }
            else
            {
                Refit(aabbRight, *object);
                objectCostRight += object->GetSurfaceArea();
            }
        }
    }

    float ComputeSAH(const BoundingBox& aabbLeft,
                     const BoundingBox& aabbRight,
                     const float        objectCostLeft,
                     const float        objectCostRight)
    {
        BoundingBox combined{};
        Refit(combined, aabbLeft);
        Refit(combined, aabbRight);

        const auto pLeft  = aabbLeft.GetSA() / combined.GetSA();
        const auto pRight = aabbRight.GetSA() / combined.GetSA();

        return pLeft * objectCostLeft + pRight * objectCostRight;
    }

    void Split(std::vector<const IIntersectable*>&       leftChild,
               std::vector<const IIntersectable*>&       rightChild,
               const std::vector<const IIntersectable*>& objects,
               const Vector3D&                           splittingPlane)
    {
        const auto currentSplittingPlane = GetCurrentSplittingPlane(splittingPlane);

        for (const auto& object : objects)
        {
            if (IsObjectInLeftSplit(splittingPlane, currentSplittingPlane, object))
            {
                leftChild.push_back(object);
            }
            else
            {
                rightChild.push_back(object);
            }
        }
    }

    bool Contains(const BoundingBox& aabb1, const BoundingBox& aabb2)
    {
        return aabb1.minCorner.x <= aabb2.minCorner.x && aabb1.maxCorner.x >= aabb2.maxCorner.x &&
               aabb1.minCorner.y <= aabb2.minCorner.y && aabb1.maxCorner.y >= aabb2.maxCorner.y &&
               aabb1.minCorner.z <= aabb2.minCorner.z && aabb1.maxCorner.z >= aabb2.maxCorner.z;
    }

    BoundingBox CreateSwapBox(const BoundingBox& a, const BoundingBox& b)
    {
        return {{std::min(a.minCorner.x, b.minCorner.x),
                 std::min(a.minCorner.y, b.minCorner.y),
                 std::min(a.minCorner.z, b.minCorner.z)},
                {std::max(a.maxCorner.x, b.maxCorner.x),
                 std::max(a.maxCorner.y, b.maxCorner.y),
                 std::max(a.maxCorner.z, b.maxCorner.z)}};
    }

    enum Rotation : std::uint32_t
    {
        NoRotation,
        SwapLeftLeftToRight,
        SwapLeftRightToRight,
        SwapRightLeftToLeft,
        SwapRightRightToLeft,
        Count
    };

    Rotation GetBestSAH(const std::span<const float> SAHs)
    {
        Rotation bestSAH = NoRotation;
        if (SAHs[SwapLeftLeftToRight] < SAHs[NoRotation])
        {
            bestSAH = SwapLeftLeftToRight;
        }
        if (SAHs[SwapLeftRightToRight] < SAHs[bestSAH])
        {
            bestSAH = SwapLeftRightToRight;
        }
        if (SAHs[SwapRightLeftToLeft] < SAHs[bestSAH])
        {
            bestSAH = SwapRightLeftToLeft;
        }
        if (SAHs[SwapRightRightToLeft] < SAHs[bestSAH])
        {
            bestSAH = SwapRightRightToLeft;
        }
        return bestSAH;
    }

    void SplitEven(const std::vector<const IIntersectable*>& objects,
                   std::vector<const IIntersectable*>&       left,
                   std::vector<const IIntersectable*>&       right)
    {
        left.reserve(objects.size() / 2);
        right.reserve((objects.size() + 1) / 2);
        for (std::size_t i = 0; i < objects.size() / 2; i++)
        {
            left.push_back(objects.at(i));
        }
        for (std::size_t i = objects.size() / 2; i < objects.size(); i++)
        {
            right.push_back(objects.at(i));
        }
    }

    bool BestSplittingPlaneExists(const int bestSplittingPlane) { return bestSplittingPlane != -1; }

    void SwitchBoxOrder(std::vector<const IIntersectable*>& leftObjects,
                        std::vector<const IIntersectable*>& rightObjects)
    {
        auto buffer  = std::move(leftObjects);
        leftObjects  = rightObjects;
        rightObjects = std::move(buffer);
    }

    inline bool RayBoxIntersection(const Vector3D& min, const Vector3D& max, const Ray& ray, float& distance)
    {
        const auto v1 = (min - ray.origin) * ray.dirfrac;
        const auto v2 = (max - ray.origin) * ray.dirfrac;

        const float tMin = std::max(std::max(std::min(v1.x, v2.x), std::min(v1.y, v2.y)), std::min(v1.z, v2.z));
        const float tMax = std::min(std::min(std::max(v1.x, v2.x), std::max(v1.y, v2.y)), std::max(v1.z, v2.z));

        distance = tMin;
        return tMax >= 0 && tMin <= tMax;
    }
} // namespace

std::array<Vector3D, DBVH::NumberOfSplittingPlanes> DBVH::CreateSplittingPlanes(const BoundingBox& bBox)
{
    std::array<Vector3D, NumberOfSplittingPlanes> splittingPlanes{};
    const int                                     splitsPerDimension = NumberOfSplittingPlanes / 3 + 1;

    for (int dim = 0; dim < 3; ++dim)
    {
        const auto split = (bBox.maxCorner[dim] - bBox.minCorner[dim]) / splitsPerDimension;
        for (int plane = 0; plane < NumberOfSplittingPlanes / 3; ++plane)
        {
            splittingPlanes[3 * dim + plane][dim] =
                bBox.minCorner[dim] + split * (plane + 1) + std::numeric_limits<float>::min();
        }
    }

    return splittingPlanes;
}

int DBVH::GetBestSplittingPlane(const std::array<float, NumberOfSplittingPlanes>& SAH)
{
    auto bestSAH            = std::numeric_limits<float>::max();
    int  bestSplittingPlane = -1;

    for (int i = 0; i < NumberOfSplittingPlanes; ++i)
    {
        if (SAH[i] < bestSAH)
        {
            bestSAH            = SAH[i];
            bestSplittingPlane = i;
        }
    }

    return bestSplittingPlane;
}

DBVH::SplitOperation DBVH::GetBestSplitOperation(const std::span<const float> SAHs)
{
    float          SAH     = std::numeric_limits<float>::max();
    SplitOperation bestSAH = SplitOperation::Default;
    for (std::size_t i = 0; i < SAHs.size(); ++i)
    {
        if (SAHs[i] < SAH)
        {
            SAH     = SAHs[i];
            bestSAH = static_cast<SplitOperation>(i);
        }
    }
    return bestSAH;
}

void DBVH::FillRotationBoxes(BoxSA& left, BoxSA& right, const std::uint32_t node) const
{
    if (IsNodeLeft(flatTree_[node]))
    {
        left.box = flatTree_[flatTree_[node].leftChild].boundingBox;
        left.sa  = nodeMetadata_[flatTree_[node].leftChild].surfaceArea;
    }
    else
    {
        left.box = leaves_[flatTree_[node].leftChild]->GetBoundaries();
        left.sa  = leaves_[flatTree_[node].leftChild]->GetSurfaceArea();
    }
    if (IsNodeRight(flatTree_[node]))
    {
        right.box = flatTree_[flatTree_[node].rightChild].boundingBox;
        right.sa  = nodeMetadata_[flatTree_[node].rightChild].surfaceArea;
    }
    else
    {
        right.box = leaves_[flatTree_[node].rightChild]->GetBoundaries();
        right.sa  = leaves_[flatTree_[node].rightChild]->GetSurfaceArea();
    }
}

bool DBVH::GetPossibleRotations(const std::uint32_t node, const std::span<float> SAHs, Rotations& rotations) const
{
    const auto& n = flatTree_[node];
    if (IsNodeLeft(n))
    {
        rotations.left.box = flatTree_[n.leftChild].boundingBox;
        rotations.left.sa  = nodeMetadata_[n.leftChild].surfaceArea;
        FillRotationBoxes(rotations.leftLeft, rotations.leftRight, n.leftChild);

        if (IsNodeRight(n))
        {
            rotations.right.box = flatTree_[n.rightChild].boundingBox;
            rotations.right.sa  = nodeMetadata_[n.rightChild].surfaceArea;
            FillRotationBoxes(rotations.rightLeft, rotations.rightRight, n.rightChild);

            rotations.swapLeftLeftToRight  = CreateSwapBox(rotations.right.box, rotations.leftRight.box);
            rotations.swapLeftRightToRight = CreateSwapBox(rotations.right.box, rotations.leftLeft.box);
            rotations.swapRightLeftToLeft  = CreateSwapBox(rotations.left.box, rotations.rightRight.box);
            rotations.swapRightRightToLeft = CreateSwapBox(rotations.left.box, rotations.rightLeft.box);

            SAHs[Rotation::SwapLeftLeftToRight] = n.boundingBox.GetSA() + rotations.leftLeft.sa + rotations.right.sa +
                                                  rotations.leftRight.sa + rotations.swapLeftLeftToRight.GetSA();
            SAHs[Rotation::SwapLeftRightToRight] = n.boundingBox.GetSA() + rotations.leftRight.sa + rotations.right.sa +
                                                   rotations.leftLeft.sa + rotations.swapLeftRightToRight.GetSA();
            SAHs[Rotation::SwapRightLeftToLeft] = n.boundingBox.GetSA() + rotations.rightLeft.sa + rotations.left.sa +
                                                  rotations.rightRight.sa + rotations.swapRightLeftToLeft.GetSA();
            SAHs[Rotation::SwapRightRightToLeft] = n.boundingBox.GetSA() + rotations.rightRight.sa + rotations.left.sa +
                                                   rotations.rightLeft.sa + rotations.swapRightRightToLeft.GetSA();
        }
        else
        {
            const auto& rightNode = leaves_[n.rightChild];
            rotations.right.box   = rightNode->GetBoundaries();
            rotations.right.sa    = rightNode->GetSurfaceArea();

            rotations.swapLeftLeftToRight  = CreateSwapBox(rotations.right.box, rotations.leftRight.box);
            rotations.swapLeftRightToRight = CreateSwapBox(rotations.right.box, rotations.leftLeft.box);

            SAHs[Rotation::SwapLeftLeftToRight] = n.boundingBox.GetSA() + rotations.leftLeft.sa + rotations.right.sa +
                                                  rotations.leftRight.sa + rotations.swapLeftLeftToRight.GetSA();
            SAHs[Rotation::SwapLeftRightToRight] = n.boundingBox.GetSA() + rotations.leftRight.sa + rotations.right.sa +
                                                   rotations.leftLeft.sa + rotations.swapLeftRightToRight.GetSA();
            SAHs[Rotation::SwapRightLeftToLeft]  = std::numeric_limits<float>::max();
            SAHs[Rotation::SwapRightRightToLeft] = std::numeric_limits<float>::max();
        }
    }
    else
    {
        if (IsLeafRight(flatTree_[node]))
            return false;

        const auto& leftNode = leaves_[n.leftChild];
        rotations.left.box   = leftNode->GetBoundaries();
        rotations.left.sa    = leftNode->GetSurfaceArea();

        rotations.right.box = flatTree_[n.rightChild].boundingBox;
        rotations.right.sa  = nodeMetadata_[n.rightChild].surfaceArea;
        FillRotationBoxes(rotations.rightLeft, rotations.rightRight, n.rightChild);

        rotations.swapRightLeftToLeft  = CreateSwapBox(rotations.left.box, rotations.rightRight.box);
        rotations.swapRightRightToLeft = CreateSwapBox(rotations.left.box, rotations.rightLeft.box);

        SAHs[Rotation::SwapLeftLeftToRight]  = std::numeric_limits<float>::max();
        SAHs[Rotation::SwapLeftRightToRight] = std::numeric_limits<float>::max();
        SAHs[Rotation::SwapRightLeftToLeft]  = n.boundingBox.GetSA() + rotations.rightLeft.sa + rotations.left.sa +
                                              rotations.rightRight.sa + rotations.swapRightLeftToLeft.GetSA();
        SAHs[Rotation::SwapRightRightToLeft] = n.boundingBox.GetSA() + rotations.rightRight.sa + rotations.left.sa +
                                               rotations.rightLeft.sa + rotations.swapRightRightToLeft.GetSA();
    }
    return true;
}

void DBVH::SwapLeftLeftToRight(const std::uint32_t node, const Rotations& rotations, const std::span<const float> SAHs)
{
    auto&      n                        = flatTree_[node];
    const auto isLeafRight              = IsLeafRight(n);
    const auto childBuffer              = n.rightChild;
    const auto depthRight               = n.maxDepthRight;
    const auto isLeafLeftLeft           = IsLeafLeft(flatTree_[n.leftChild]);
    n.rightChild                        = flatTree_[n.leftChild].leftChild;
    n.maxDepthRight                     = flatTree_[n.leftChild].maxDepthLeft;
    flatTree_[n.leftChild].boundingBox  = rotations.swapLeftLeftToRight;
    flatTree_[n.leftChild].maxDepthLeft = depthRight;
    flatTree_[n.leftChild].leftChild    = childBuffer;
    n.maxDepthLeft = std::max(flatTree_[n.leftChild].maxDepthLeft, flatTree_[n.leftChild].maxDepthLeft) + 1;
    nodeMetadata_[node].surfaceArea = SAHs[Rotation::SwapLeftLeftToRight];
    nodeMetadata_[flatTree_[node].leftChild].surfaceArea =
        rotations.right.sa + rotations.leftRight.sa + rotations.swapLeftLeftToRight.GetSA();
    if (isLeafLeftLeft)
        leafMetadata_[n.rightChild].parent = node;
    else
        nodeMetadata_[n.rightChild].parent = node;
    if (isLeafRight)
        leafMetadata_[childBuffer].parent = n.leftChild;
    else
        nodeMetadata_[childBuffer].parent = n.leftChild;
}

void DBVH::SwapLeftRightToRight(const std::uint32_t node, const Rotations& rotations, const std::span<const float> SAHs)
{
    auto&      n                         = flatTree_[node];
    const auto isLeafRight               = IsLeafRight(n);
    const auto buffer                    = n.rightChild;
    const auto depthRight                = n.maxDepthRight;
    const auto isLeafLeftRight           = IsLeafRight(flatTree_[n.leftChild]);
    n.rightChild                         = flatTree_[n.leftChild].rightChild;
    n.maxDepthRight                      = flatTree_[n.leftChild].maxDepthRight;
    flatTree_[n.leftChild].boundingBox   = rotations.swapLeftRightToRight;
    flatTree_[n.leftChild].maxDepthRight = depthRight;
    flatTree_[n.leftChild].rightChild    = buffer;
    n.maxDepthLeft = std::max(flatTree_[n.leftChild].maxDepthLeft, flatTree_[n.leftChild].maxDepthLeft) + 1;
    nodeMetadata_[node].surfaceArea = SAHs[Rotation::SwapLeftRightToRight];
    nodeMetadata_[flatTree_[node].leftChild].surfaceArea =
        rotations.right.sa + rotations.leftLeft.sa + rotations.swapLeftRightToRight.GetSA();
    if (isLeafLeftRight)
        leafMetadata_[n.rightChild].parent = node;
    else
        nodeMetadata_[n.rightChild].parent = node;
    if (isLeafRight)
        leafMetadata_[buffer].parent = n.leftChild;
    else
        nodeMetadata_[buffer].parent = n.leftChild;
}

void DBVH::SwapRightLeftToLeft(const std::uint32_t node, const Rotations& rotations, const std::span<const float> SAHs)
{
    auto&      n                         = flatTree_[node];
    const auto isLeafLeft                = IsLeafLeft(n);
    const auto buffer                    = n.leftChild;
    const auto depthLeft                 = n.maxDepthLeft;
    const auto isLeafRightLeft           = IsLeafLeft(flatTree_[n.rightChild]);
    n.leftChild                          = flatTree_[n.rightChild].leftChild;
    n.maxDepthLeft                       = flatTree_[n.rightChild].maxDepthLeft;
    flatTree_[n.rightChild].boundingBox  = rotations.swapRightLeftToLeft;
    flatTree_[n.rightChild].maxDepthLeft = depthLeft;
    flatTree_[n.rightChild].leftChild    = buffer;
    n.maxDepthRight = std::max(flatTree_[n.rightChild].maxDepthLeft, flatTree_[n.rightChild].maxDepthLeft) + 1;
    nodeMetadata_[node].surfaceArea = SAHs[Rotation::SwapRightLeftToLeft];
    nodeMetadata_[flatTree_[node].rightChild].surfaceArea =
        rotations.left.sa + rotations.rightRight.sa + rotations.swapRightLeftToLeft.GetSA();
    if (isLeafRightLeft)
        leafMetadata_[n.leftChild].parent = node;
    else
        nodeMetadata_[n.leftChild].parent = node;
    if (isLeafLeft)
        leafMetadata_[buffer].parent = n.rightChild;
    else
        nodeMetadata_[buffer].parent = n.rightChild;
}

void DBVH::SwapRightRightToLeft(const std::uint32_t node, const Rotations& rotations, const std::span<const float> SAHs)
{
    auto&      n                          = flatTree_[node];
    const auto isLeafLeft                 = IsLeafLeft(n);
    const auto buffer                     = n.leftChild;
    const auto depthLeft                  = n.maxDepthLeft;
    const auto isLeafRightRight           = IsLeafRight(flatTree_[n.rightChild]);
    n.leftChild                           = flatTree_[n.rightChild].rightChild;
    n.maxDepthLeft                        = flatTree_[n.rightChild].maxDepthRight;
    flatTree_[n.rightChild].boundingBox   = rotations.swapRightRightToLeft;
    flatTree_[n.rightChild].maxDepthRight = depthLeft;
    flatTree_[n.rightChild].rightChild    = buffer;
    n.maxDepthRight = std::max(flatTree_[n.rightChild].maxDepthLeft, flatTree_[n.rightChild].maxDepthLeft) + 1;
    nodeMetadata_[node].surfaceArea = SAHs[Rotation::SwapRightRightToLeft];
    nodeMetadata_[flatTree_[node].rightChild].surfaceArea =
        rotations.left.sa + rotations.rightLeft.sa + rotations.swapRightRightToLeft.GetSA();
    if (isLeafRightRight)
        leafMetadata_[n.leftChild].parent = node;
    else
        nodeMetadata_[n.leftChild].parent = node;
    if (isLeafLeft)
        leafMetadata_[buffer].parent = n.rightChild;
    else
        nodeMetadata_[buffer].parent = n.rightChild;
}

bool DBVH::OptimizeSAH(const std::uint32_t node)
{
    Rotations                          rotations{};
    std::array<float, Rotation::Count> SAHs{};

    if (!GetPossibleRotations(node, SAHs, rotations))
        return false;

    SAHs[NoRotation]   = nodeMetadata_[node].surfaceArea;
    const auto bestSAH = GetBestSAH(SAHs);

    switch (bestSAH)
    {
        case NoRotation:
            return false;
        case Rotation::SwapLeftLeftToRight:
        {
            SwapLeftLeftToRight(node, rotations, SAHs);
            break;
        }
        case Rotation::SwapLeftRightToRight:
        {
            SwapLeftRightToRight(node, rotations, SAHs);
            break;
        }
        case Rotation::SwapRightLeftToLeft:
        {
            SwapRightLeftToLeft(node, rotations, SAHs);
            break;
        }
        case Rotation::SwapRightRightToLeft:
        {
            SwapRightRightToLeft(node, rotations, SAHs);
            break;
        }
    }
    return true;
}

void DBVH::Refit(const std::uint32_t node)
{
    auto& n                         = flatTree_[node];
    n.boundingBox                   = BoundingBox();
    nodeMetadata_[node].surfaceArea = 0;
    if (IsNodeRight(n))
    {
        const auto& rightChild = flatTree_[n.rightChild];
        ::Refit(n.boundingBox, rightChild.boundingBox);
        nodeMetadata_[node].surfaceArea += nodeMetadata_[n.rightChild].surfaceArea;
        n.maxDepthRight = std::max(rightChild.maxDepthRight, rightChild.maxDepthLeft) + 1;
    }
    else
    {
        ::Refit(n.boundingBox, leaves_[n.rightChild]->GetBoundaries());
        nodeMetadata_[node].surfaceArea += leaves_[n.rightChild]->GetSurfaceArea();
    }
    if (IsNodeLeft(n))
    {
        const auto& leftChild = flatTree_[n.leftChild];
        ::Refit(n.boundingBox, leftChild.boundingBox);
        nodeMetadata_[node].surfaceArea += nodeMetadata_[n.leftChild].surfaceArea;
        n.maxDepthLeft = std::max(leftChild.maxDepthRight, leftChild.maxDepthLeft) + 1;
    }
    else
    {
        ::Refit(n.boundingBox, leaves_[n.leftChild]->GetBoundaries());
        nodeMetadata_[node].surfaceArea += leaves_[n.leftChild]->GetSurfaceArea();
    }
    nodeMetadata_[node].surfaceArea += n.boundingBox.GetSA();
}

void DBVH::RemoveNode(const std::uint32_t node)
{
    if (node != (flatTree_.size() - 1))
    {
        flatTree_[node]     = flatTree_.back();
        auto& parentOfMoved = flatTree_[nodeMetadata_.back().parent];
        if (IsNodeLeft(parentOfMoved) && (parentOfMoved.leftChild == (flatTree_.size() - 1)))
        {
            parentOfMoved.leftChild = node;
        }
        else
        {
            parentOfMoved.rightChild = node;
        }
        nodeMetadata_[node] = nodeMetadata_.back();
        const auto& n       = flatTree_[node];
        if (IsLeafLeft(n))
            leafMetadata_[n.leftChild].parent = node;
        else
            nodeMetadata_[n.leftChild].parent = node;
        if (IsLeafRight(n))
            leafMetadata_[n.rightChild].parent = node;
        else
            nodeMetadata_[n.rightChild].parent = node;
    }
    flatTree_.pop_back();
    nodeMetadata_.pop_back();
}

void DBVH::RemoveLeaf(const std::uint32_t leaf)
{
    if (leaf != (leaves_.size() - 1))
    {
        leaves_[leaf] = leaves_.back();
        auto& parent  = flatTree_[leafMetadata_.back().parent];
        if (IsLeafLeft(parent) && (parent.leftChild == (leaves_.size() - 1)))
        {
            parent.leftChild = leaf;
        }
        else
        {
            parent.rightChild = leaf;
        }
        leafMetadata_[leaf] = leafMetadata_.back();
    }
    leaves_.pop_back();
    leafMetadata_.pop_back();
}

bool DBVH::RemoveRightLeaf(const std::uint32_t   currentNode,
                           const IIntersectable& object,
                           std::uint32_t&        rLeaf,
                           std::uint32_t&        rNode)
{
    auto& n = flatTree_[currentNode];
    if (IsLeafRight(n))
        return false;
    const auto& child = flatTree_[n.rightChild];
    if (!Contains(child.boundingBox, object.GetBoundaries()))
        return false;

    bool removed = false;
    if (removed = (IsLeafLeft(child) && (*leaves_[child.leftChild] == object)))
    {
        rLeaf        = child.leftChild;
        rNode        = n.rightChild;
        n.rightChild = child.rightChild;
    }
    else if (removed = (IsLeafRight(child) && (*leaves_[child.rightChild] == object)))
    {
        rLeaf        = child.rightChild;
        rNode        = n.rightChild;
        n.rightChild = child.leftChild;
    }
    if (removed)
    {
        --n.maxDepthRight;
        if (IsLeafRight(n))
        {
            leafMetadata_[n.rightChild].parent = currentNode;
        }
        else
        {
            nodeMetadata_[n.rightChild].parent = currentNode;
        }
        Refit(currentNode);
    }
    return removed;
}

bool DBVH::RemoveLeftLeaf(const std::uint32_t   currentNode,
                          const IIntersectable& object,
                          std::uint32_t&        rLeaf,
                          std::uint32_t&        rNode)
{
    auto& n = flatTree_[currentNode];
    if (IsLeafLeft(n))
        return false;
    const auto& child = flatTree_[n.leftChild];
    if (!Contains(child.boundingBox, object.GetBoundaries()))
        return false;

    bool removed = false;
    if (removed = (IsLeafLeft(child) && (*leaves_[child.leftChild] == object)))
    {
        rLeaf       = child.leftChild;
        rNode       = n.leftChild;
        n.leftChild = child.rightChild;
    }
    else if (removed = (IsLeafRight(child) && (*leaves_[child.rightChild] == object)))
    {
        rLeaf       = child.rightChild;
        rNode       = n.leftChild;
        n.leftChild = child.leftChild;
    }
    if (removed)
    {
        --n.maxDepthLeft;
        if (IsLeafLeft(n))
        {
            leafMetadata_[n.leftChild].parent = currentNode;
        }
        else
        {
            nodeMetadata_[n.leftChild].parent = currentNode;
        }
        Refit(currentNode);
    }
    return removed;
}

bool DBVH::Remove(const std::uint32_t   currentNode,
                  const IIntersectable& object,
                  std::uint32_t&        rLeaf,
                  std::uint32_t&        rNode)
{
    // find object in tree by insertion
    // remove object and refit nodes going the tree back up
    if (!Contains(flatTree_[currentNode].boundingBox, object.GetBoundaries()))
        return false;
    if (RemoveLeftLeaf(currentNode, object, rLeaf, rNode) || RemoveRightLeaf(currentNode, object, rLeaf, rNode))
        return true;
    if ((IsNodeLeft(flatTree_[currentNode]) && Remove(flatTree_[currentNode].leftChild, object, rLeaf, rNode)) ||
        (IsNodeRight(flatTree_[currentNode]) && Remove(flatTree_[currentNode].rightChild, object, rLeaf, rNode)))
    {
        Refit(currentNode);
        OptimizeSAH(currentNode);
        return true;
    }
    return false;
}

bool DBVH::RemoveSpecialCases(const IIntersectable& object)
{
    if (IsLastElement(flatTree_[0]) && (*leaves_[flatTree_[0].leftChild] == object))
    {
        flatTree_[0].maxDepthLeft    = 0;
        flatTree_[0].leftChild       = 0;
        nodeMetadata_[0].surfaceArea = 0;
        RemoveLeaf(flatTree_[0].leftChild);
        flatTree_[0] = {};
        return true;
    }
    else if (IsLeafLeft(flatTree_[0]) && (*leaves_[flatTree_[0].leftChild] == object))
    {
        RemoveLeaf(flatTree_[0].leftChild);
        if (IsLeafRight(flatTree_[0]))
        {
            flatTree_[0].leftChild       = flatTree_[0].rightChild;
            flatTree_[0].rightChild      = 0;
            flatTree_[0].maxDepthRight   = 0;
            flatTree_[0].boundingBox     = leaves_[flatTree_[0].leftChild]->GetBoundaries();
            nodeMetadata_[0].surfaceArea = leaves_[flatTree_[0].leftChild]->GetSurfaceArea();
        }
        else
        {
            const auto toBeRemoved       = flatTree_[0].rightChild;
            flatTree_[0]                 = flatTree_[toBeRemoved];
            nodeMetadata_[0].surfaceArea = nodeMetadata_[toBeRemoved].surfaceArea;
            if (IsLeafLeft(flatTree_[0]))
                leafMetadata_[flatTree_[0].leftChild].parent = 0;
            else
                nodeMetadata_[flatTree_[0].leftChild].parent = 0;
            if (IsLeafRight(flatTree_[0]))
                leafMetadata_[flatTree_[0].rightChild].parent = 0;
            else
                nodeMetadata_[flatTree_[0].rightChild].parent = 0;
            RemoveNode(toBeRemoved);
        }
        return true;
    }
    else if (IsLeafRight(flatTree_[0]) && *leaves_[flatTree_[0].rightChild] == object)
    {
        RemoveLeaf(flatTree_[0].rightChild);
        if (IsLeafLeft(flatTree_[0]))
        {
            flatTree_[0].rightChild      = 0;
            flatTree_[0].maxDepthRight   = 0;
            flatTree_[0].boundingBox     = leaves_[flatTree_[0].leftChild]->GetBoundaries();
            nodeMetadata_[0].surfaceArea = leaves_[flatTree_[0].leftChild]->GetSurfaceArea();
        }
        else
        {
            const auto toBeRemoved       = flatTree_[0].leftChild;
            flatTree_[0]                 = flatTree_[toBeRemoved];
            nodeMetadata_[0].surfaceArea = nodeMetadata_[toBeRemoved].surfaceArea;
            if (IsLeafLeft(flatTree_[0]))
                leafMetadata_[flatTree_[0].leftChild].parent = 0;
            else
                nodeMetadata_[flatTree_[0].leftChild].parent = 0;
            if (IsLeafRight(flatTree_[0]))
                leafMetadata_[flatTree_[0].rightChild].parent = 0;
            else
                nodeMetadata_[flatTree_[0].rightChild].parent = 0;
            RemoveNode(toBeRemoved);
        }
        return true;
    }
    return false;
}

void DBVH::MoveParentToNewParentsLeftChild(const std::uint32_t node)
{
    DBVHNode newNode = flatTree_[node];
    if (IsLeafLeft(newNode))
        leafMetadata_[newNode.leftChild].parent = flatTree_.size();
    else
        nodeMetadata_[newNode.leftChild].parent = flatTree_.size();
    if (IsLeafRight(newNode))
        leafMetadata_[newNode.rightChild].parent = flatTree_.size();
    else
        nodeMetadata_[newNode.rightChild].parent = flatTree_.size();
    flatTree_[node].maxDepthLeft = std::max(newNode.maxDepthLeft, newNode.maxDepthRight) + 1;
    flatTree_[node].boundingBox  = newNode.boundingBox;
    flatTree_[node].leftChild    = flatTree_.size();
    flatTree_.push_back(std::move(newNode));
    flatTree_[node].maxDepthRight = 0;
    nodeMetadata_.emplace_back(node, 0.f);
}

void DBVH::SortObjectsIntoBoxes(const SplitOperation                      splitOperation,
                                const Vector3D&                           splittingPlane,
                                const std::uint32_t                       node,
                                const std::vector<const IIntersectable*>& objects,
                                std::vector<const IIntersectable*>&       leftObjects,
                                std::vector<const IIntersectable*>&       rightObjects)
{
    switch (splitOperation)
    {
        case Default:
            ::Split(leftObjects, rightObjects, objects, splittingPlane);
            break;
        case DefaultWrongOrder:
        {
            ::Split(leftObjects, rightObjects, objects, splittingPlane);
            SwitchBoxOrder(leftObjects, rightObjects);
            break;
        }
        case AllNewLeft:
        {
            leftObjects = objects;
            break;
        }
        case AllNewRight:
        {
            rightObjects = objects;
            break;
        }
        case SplitOldNew:
        {
            MoveParentToNewParentsLeftChild(node);
            rightObjects = objects;
            break;
        }
        case DefaultOldLeft:
        {
            ::Split(leftObjects, rightObjects, objects, splittingPlane);
            MoveParentToNewParentsLeftChild(node);
            break;
        }
        case DefaultWrongOrderOldLeft:
        {
            ::Split(leftObjects, rightObjects, objects, splittingPlane);
            SwitchBoxOrder(leftObjects, rightObjects);
            MoveParentToNewParentsLeftChild(node);
            break;
        }
        default:
            throw(std::out_of_range("Undefined split operation!"));
    }
}

void DBVH::SetBoxesAndHitProbability(const DBVHNode& node,
                                     BoundingBox&    leftChildBox,
                                     BoundingBox&    rightChildBox,
                                     float&          pLeft,
                                     float&          pRight) const
{
    pLeft  = 0;
    pRight = 0;
    if (IsNodeLeft(node))
    {
        leftChildBox = flatTree_[node.leftChild].boundingBox;
        pLeft        = nodeMetadata_[node.leftChild].surfaceArea / leftChildBox.GetSA();
    }
    else
    {
        leftChildBox = leaves_[node.leftChild]->GetBoundaries();
        pLeft        = leaves_[node.leftChild]->GetSurfaceArea() / leftChildBox.GetSA();
    }
    if (IsNodeRight(node))
    {
        rightChildBox = flatTree_[node.rightChild].boundingBox;
        pRight        = nodeMetadata_[node.rightChild].surfaceArea / rightChildBox.GetSA();
    }
    else
    {
        rightChildBox = leaves_[node.rightChild]->GetBoundaries();
        pRight        = leaves_[node.rightChild]->GetSurfaceArea() / rightChildBox.GetSA();
    }
}

float DBVH::ComputeSAHWithNewParent(const DBVHNode&    node,
                                    const BoundingBox& aabbLeft,
                                    const BoundingBox& aabbRight,
                                    const float        objectCostLeft,
                                    const float        objectCostRight,
                                    SplitOperation&    newParent) const
{
    BoundingBox leftChildBox{};
    BoundingBox rightChildBox{};
    float       pLeft{};
    float       pRight{};
    SetBoxesAndHitProbability(node, leftChildBox, rightChildBox, pLeft, pRight);

    const auto& oldLeft                 = leftChildBox;
    auto        oldLeftNewLeft          = leftChildBox;
    auto        oldLeftNewRight         = leftChildBox;
    auto        oldLeftNewLeftNewRight  = leftChildBox;
    auto        oldLeftOldRight         = leftChildBox;
    auto        oldLeftOldRightNewLeft  = leftChildBox;
    auto        oldLeftOldRightNewRight = leftChildBox;
    auto        oldRightNewLeftNewRight = rightChildBox;
    auto        oldRightNewRight        = rightChildBox;
    auto        oldRightNewLeft         = rightChildBox;
    const auto& oldRight                = rightChildBox;
    auto        newLeftNewRight         = aabbLeft;
    const auto& newRight                = aabbRight;
    const auto& newLeft                 = aabbLeft;

    ::Refit(newLeftNewRight, newRight);
    ::Refit(oldLeftNewLeft, newLeft);
    ::Refit(oldLeftNewRight, newRight);
    ::Refit(oldLeftNewLeftNewRight, newLeftNewRight);
    ::Refit(oldLeftOldRight, oldRight);
    ::Refit(oldRightNewLeft, newLeft);
    ::Refit(oldRightNewRight, newRight);
    ::Refit(oldLeftOldRightNewLeft, oldRightNewLeft);
    ::Refit(oldLeftOldRightNewRight, oldRightNewRight);
    ::Refit(oldRightNewLeftNewRight, newLeftNewRight);

    std::array<float, SplitOperation::Count> SAHs{};

    SAHs[Default] =
        oldLeftNewLeft.GetSA() * (objectCostLeft + pLeft) + oldRightNewRight.GetSA() * (objectCostRight + pRight);
    SAHs[DefaultWrongOrder] =
        oldLeftNewRight.GetSA() * (objectCostRight + pLeft) + oldRightNewLeft.GetSA() * (objectCostLeft + pRight);
    SAHs[AllNewLeft] =
        oldLeft.GetSA() * pLeft + oldRightNewLeftNewRight.GetSA() * (objectCostLeft + objectCostRight + pRight);
    SAHs[AllNewRight] =
        oldLeftNewLeftNewRight.GetSA() * (objectCostLeft + objectCostRight + pLeft) + oldRight.GetSA() * pRight;
    SAHs[SplitOldNew] =
        oldLeftOldRight.GetSA() * (pLeft + pRight) + newLeftNewRight.GetSA() * (objectCostLeft + objectCostRight);
    SAHs[DefaultOldLeft] =
        oldLeftOldRightNewLeft.GetSA() * (objectCostLeft + pLeft + pRight) + newRight.GetSA() * objectCostRight;
    SAHs[DefaultWrongOrderOldLeft] =
        oldLeftOldRightNewRight.GetSA() * (objectCostRight + pLeft + pRight) + newLeft.GetSA() * objectCostLeft;

    const auto bestSAH = GetBestSplitOperation(SAHs);

    newParent = bestSAH;
    return SAHs[bestSAH];
}

float DBVH::EvaluateBucket(const DBVHNode&                           node,
                           const std::vector<const IIntersectable*>& objects,
                           const Vector3D&                           splittingPlane,
                           SplitOperation&                           newParent) const
{
    BoundingBox aabbLeft{};
    BoundingBox aabbRight{};

    float objectCostLeft  = 0;
    float objectCostRight = 0;

    SortObjectsIntoBuckets(objects, splittingPlane, aabbLeft, aabbRight, objectCostLeft, objectCostRight);

    if (!IsEmptyLeft(node) && !IsEmptyRight(node))
    {
        return ComputeSAHWithNewParent(node, aabbLeft, aabbRight, objectCostLeft, objectCostRight, newParent);
    }
    else
    {
        return ComputeSAH(aabbLeft, aabbRight, objectCostLeft, objectCostRight);
    }
}

std::array<float, DBVH::NumberOfSplittingPlanes>
DBVH::EvaluateSplittingPlanes(const DBVHNode&                                      node,
                              const std::vector<const IIntersectable*>&            objects,
                              const std::array<Vector3D, NumberOfSplittingPlanes>& splittingPlanes,
                              std::array<SplitOperation, NumberOfSplittingPlanes>& newParent) const
{
    std::array<float, NumberOfSplittingPlanes> SAH{};

    for (int i = 0; i < NumberOfSplittingPlanes; ++i)
    {
        SAH[i] = EvaluateBucket(node, objects, splittingPlanes[i], newParent[i]);
    }

    return SAH;
}

std::pair<std::vector<const IIntersectable*>, std::vector<const IIntersectable*>>
DBVH::Split(const std::vector<const IIntersectable*>& objects, const std::uint32_t currentNode)
{
    BoundingBox bBox = flatTree_[currentNode].boundingBox;
    ::Refit(bBox, objects, 0);

    // create split buckets
    const auto splittingPlanes = CreateSplittingPlanes(bBox);

    std::array<SplitOperation, NumberOfSplittingPlanes> newParent{};

    // evaluate split buckets
    const auto SAH = EvaluateSplittingPlanes(flatTree_[currentNode], objects, splittingPlanes, newParent);

    // choose best split bucket and split currentNode accordingly
    const auto bestSplittingPlane = GetBestSplittingPlane(SAH);

    std::vector<const IIntersectable*> leftObjects;
    std::vector<const IIntersectable*> rightObjects;

    if (!BestSplittingPlaneExists(bestSplittingPlane))
    {
        SplitEven(objects, leftObjects, rightObjects);
    }
    else
    {
        SortObjectsIntoBoxes(newParent[bestSplittingPlane],
                             splittingPlanes[bestSplittingPlane],
                             currentNode,
                             objects,
                             leftObjects,
                             rightObjects);
    }

    return {leftObjects, rightObjects};
}

void DBVH::Add(const std::uint32_t currentNode, const std::vector<const IIntersectable*>& objects, const uint8_t depth)
{
    // may add extra node if the split needs to be done between existing nodes and new objects
    const auto [leftObjects, rightObjects] = Split(objects, currentNode);

    // pass objects to children
    if ((leftObjects.size() == 1) && (IsEmptyLeft(flatTree_[currentNode]) || IsLeafLeft(flatTree_[currentNode])))
    {
        if (IsEmptyLeft(flatTree_[currentNode]))
        {
            flatTree_[currentNode].leftChild = leaves_.size();
            leaves_.push_back(leftObjects.at(0));
            leafMetadata_.emplace_back(currentNode, leftObjects.at(0)->GetSurfaceArea());
            flatTree_[currentNode].maxDepthLeft = 1;
        }
        else
        {
            const auto buffer = flatTree_[currentNode].leftChild;
            DBVHNode   parent{};
            parent.leftChild  = buffer;
            parent.rightChild = leaves_.size();
            leaves_.push_back(leftObjects.at(0));
            leafMetadata_.emplace_back(static_cast<std::uint32_t>(flatTree_.size()), 0.f);
            parent.maxDepthLeft              = 1;
            parent.maxDepthRight             = 1;
            flatTree_[currentNode].leftChild = flatTree_.size();
            leafMetadata_[buffer].parent     = flatTree_.size();
            flatTree_.push_back(std::move(parent));
            nodeMetadata_.emplace_back(currentNode, 0.f);
            Refit(flatTree_[currentNode].leftChild);
            flatTree_[currentNode].maxDepthLeft = 2;
        }
    }
    else if (!leftObjects.empty())
    {
        if (IsEmptyLeft(flatTree_[currentNode]))
        {
            flatTree_[currentNode].leftChild = flatTree_.size();
            flatTree_.emplace_back();
            flatTree_[currentNode].maxDepthLeft = 2;
            nodeMetadata_.emplace_back(currentNode, 0.f);
        }
        else if (IsLeafLeft(flatTree_[currentNode]))
        {
            const auto buffer = flatTree_[currentNode].leftChild;
            DBVHNode   parent{};
            Metadata   parentMetadata{.parent = currentNode};
            parent.leftChild                 = buffer;
            parent.boundingBox               = leaves_[buffer]->GetBoundaries();
            parentMetadata.surfaceArea       = parent.boundingBox.GetSA() * 2;
            parent.maxDepthLeft              = 1;
            flatTree_[currentNode].leftChild = flatTree_.size();
            leafMetadata_[buffer].parent     = flatTree_.size();
            flatTree_.push_back(std::move(parent));
            flatTree_[currentNode].maxDepthLeft = 2;
            nodeMetadata_.push_back(std::move(parentMetadata));
        }
        Add(flatTree_[currentNode].leftChild, leftObjects, depth + 1);
    }

    if ((rightObjects.size() == 1) && (IsEmptyRight(flatTree_[currentNode]) || IsLeafRight(flatTree_[currentNode])))
    {
        if (IsEmptyRight(flatTree_[currentNode]))
        {
            flatTree_[currentNode].rightChild = leaves_.size();
            leaves_.push_back(rightObjects.at(0));
            leafMetadata_.emplace_back(currentNode, rightObjects.at(0)->GetSurfaceArea());
            flatTree_[currentNode].maxDepthRight = 1;
        }
        else
        {
            const auto buffer = flatTree_[currentNode].rightChild;
            DBVHNode   parent{};
            parent.leftChild  = buffer;
            parent.rightChild = leaves_.size();
            leaves_.push_back(rightObjects.at(0));
            leafMetadata_.emplace_back(static_cast<std::uint32_t>(flatTree_.size()), 0.f);
            parent.maxDepthLeft               = 1;
            parent.maxDepthRight              = 1;
            flatTree_[currentNode].rightChild = flatTree_.size();
            leafMetadata_[buffer].parent      = flatTree_.size();
            flatTree_.push_back(std::move(parent));
            nodeMetadata_.emplace_back(currentNode, 0.f);
            Refit(flatTree_[currentNode].rightChild);
            flatTree_[currentNode].maxDepthRight = 2;
        }
    }
    else if (!rightObjects.empty())
    {
        if (IsEmptyRight(flatTree_[currentNode]))
        {
            flatTree_[currentNode].rightChild = flatTree_.size();
            flatTree_.emplace_back();
            flatTree_[currentNode].maxDepthRight = 2;
            nodeMetadata_.emplace_back(currentNode, 0.f);
        }
        else if (IsLeafRight(flatTree_[currentNode]))
        {
            const auto buffer = flatTree_[currentNode].rightChild;
            DBVHNode   parent{};
            Metadata   parentMetadata{.parent = currentNode};
            parent.rightChild                 = buffer;
            parent.boundingBox                = leaves_[buffer]->GetBoundaries();
            parentMetadata.surfaceArea        = parent.boundingBox.GetSA() * 2;
            parent.maxDepthRight              = 1;
            flatTree_[currentNode].rightChild = flatTree_.size();
            leafMetadata_[buffer].parent      = flatTree_.size();
            flatTree_.push_back(std::move(parent));
            flatTree_[currentNode].maxDepthRight = 2;
            nodeMetadata_.push_back(std::move(parentMetadata));
        }
        Add(flatTree_[currentNode].rightChild, rightObjects, depth + 1);
    }

    // calculate surface area and tree depth going the tree back up
    Refit(currentNode);

    // use tree rotations going the tree back up to optimize SAH
    OptimizeSAH(currentNode);
}

bool DBVH::AddOntoSingleElement(const std::vector<const IIntersectable*>& objects)
{
    if (!IsLastElement(flatTree_[0]))
        return false;
    std::vector<const IIntersectable*> newObjects{objects}; // TODO: make more efficient
    newObjects.push_back(leaves_[flatTree_[0].leftChild]);
    flatTree_[0].maxDepthLeft = 0;
    Add(0, newObjects, 1);
    return true;
}

bool DBVH::AddFirstAndOnlyElement(DBVHNode& root, const std::vector<const IIntersectable*>& objects)
{
    if (!IsEmpty(root) || objects.size() != 1)
        return false;
    ::Refit(root.boundingBox, objects, 0);
    root.leftChild = leaves_.size();
    leaves_.push_back(objects.back());
    leafMetadata_.emplace_back(0u, 0.f);
    nodeMetadata_[0].surfaceArea = objects.back()->GetSurfaceArea();
    root.maxDepthLeft            = 1;
    return true;
}

DBVH::DBVH()
{
    flatTree_.emplace_back(); // make sure there is always a root node
    nodeMetadata_.emplace_back();
}

DBVH::DBVH(DBVH&& other) noexcept
    : flatTree_(std::move(other.flatTree_)),
      leaves_(std::move(other.leaves_)),
      nodeMetadata_(std::move(other.nodeMetadata_))
{
}

DBVH& DBVH::operator=(DBVH&& other) noexcept
{
    flatTree_     = std::move(other.flatTree_);
    leaves_       = std::move(other.leaves_);
    nodeMetadata_ = std::move(other.nodeMetadata_);
    return *this;
}

DBVH::DBVH(const std::vector<const IIntersectable*>& objects) : DBVH() { AddObjects(objects); }

void DBVH::AddObjects(const std::vector<const IIntersectable*>& objects)
{
    if (objects.empty() || AddFirstAndOnlyElement(flatTree_[0], objects) || AddOntoSingleElement(objects))
        return;

    Add(0, objects, 1);
}

void DBVH::RemoveObjects(const std::vector<const IIntersectable*>& objects)
{
    for (const auto& object : objects)
    {
        if (IsEmpty(flatTree_[0]))
            return;
        if (RemoveSpecialCases(*object))
            continue;
        std::uint32_t rLeaf, rNode;
        if (Remove(0, *object, rLeaf, rNode))
        {
            RemoveLeaf(rLeaf);
            RemoveNode(rNode);
        }
    }
}

bool DBVH::IntersectFirst(IntersectionInfo& intersectionInfo, const Ray& ray) const
{
    if (IsEmpty(flatTree_[0]))
        return false;
    if (IsLastElement(flatTree_[0]))
        return leaves_[flatTree_[0].leftChild]->IntersectFirst(intersectionInfo, ray);

    thread_local std::vector<std::uint32_t> stack;
    const auto                              stackStart = stack.size();
    stack.push_back(0);

    bool hit = false;

    while (stack.size() != stackStart)
    {
        const auto& currentNode = flatTree_[stack.back()];
        stack.pop_back();

        if (IsLeafRight(currentNode))
        {
            IntersectionInfo info{false, std::numeric_limits<float>::max()};
            hit |= leaves_[currentNode.rightChild]->IntersectFirst(info, ray);
            if (info.hit && info.distance < intersectionInfo.distance)
            {
                intersectionInfo = info;
            }
        }
        else
        {
            float distance = 0;
            if (RayBoxIntersection(flatTree_[currentNode.rightChild].boundingBox.minCorner,
                                   flatTree_[currentNode.rightChild].boundingBox.maxCorner,
                                   ray,
                                   distance) &&
                distance < intersectionInfo.distance)
            {
                if (currentNode.rightChild == 1)
                    volatile int i = 0;

                stack.push_back(currentNode.rightChild);
            }
        }

        if (IsLeafLeft(currentNode))
        {
            IntersectionInfo info{false, std::numeric_limits<float>::max()};
            hit |= leaves_[currentNode.leftChild]->IntersectFirst(info, ray);
            if (info.hit && info.distance < intersectionInfo.distance)
            {
                intersectionInfo = info;
            }
        }
        else
        {
            float distance = 0;
            if (RayBoxIntersection(flatTree_[currentNode.leftChild].boundingBox.minCorner,
                                   flatTree_[currentNode.leftChild].boundingBox.maxCorner,
                                   ray,
                                   distance) &&
                distance < intersectionInfo.distance)
            {
                if (currentNode.rightChild == 1)
                    volatile int i = 0;

                stack.push_back(currentNode.leftChild);
            }
        }
        const auto test = stack.size();
    }

    return hit;
}

bool DBVH::IntersectAny(IntersectionInfo& intersectionInfo, const Ray& ray) const
{
    if (IsEmpty(flatTree_[0]))
        return false;
    if (IsLastElement(flatTree_[0]))
        return leaves_[flatTree_[0].leftChild]->IntersectFirst(intersectionInfo, ray);

    thread_local std::vector<std::uint32_t> stack;
    stack.push_back(0);

    while (!stack.empty())
    {
        const auto& currentNode = flatTree_[stack.back()];
        stack.pop_back();

        if (IsLeafRight(currentNode))
        {
            IntersectionInfo info{false, std::numeric_limits<float>::max()};
            if (leaves_[currentNode.rightChild]->IntersectAny(info, ray))
            {
                intersectionInfo = info;
                return true;
            }
        }
        else
        {
            float distance = 0;
            if (RayBoxIntersection(flatTree_[currentNode.rightChild].boundingBox.minCorner,
                                   flatTree_[currentNode.rightChild].boundingBox.maxCorner,
                                   ray,
                                   distance) &&
                distance < intersectionInfo.distance)
            {
                stack.push_back(currentNode.rightChild);
            }
        }

        if (IsLeafLeft(currentNode))
        {
            IntersectionInfo info{false, std::numeric_limits<float>::max()};
            if (leaves_[currentNode.leftChild]->IntersectAny(info, ray))
            {
                intersectionInfo = info;
                return true;
            }
        }
        else
        {
            float distance = 0;
            if (RayBoxIntersection(flatTree_[currentNode.leftChild].boundingBox.minCorner,
                                   flatTree_[currentNode.leftChild].boundingBox.maxCorner,
                                   ray,
                                   distance) &&
                distance < intersectionInfo.distance)
            {
                stack.push_back(currentNode.leftChild);
            }
        }
    }

    return false;
}

bool DBVH::IntersectAll(std::vector<IntersectionInfo>& intersectionInfos, const Ray& ray) const
{
    if (IsEmpty(flatTree_[0]))
        return false;
    if (IsLastElement(flatTree_[0]))
        return leaves_[flatTree_[0].leftChild]->IntersectAll(intersectionInfos, ray);

    thread_local std::vector<std::uint32_t> stack;
    stack.push_back(0);

    bool hit = false;

    while (!stack.empty())
    {
        const auto& currentNode = flatTree_[stack.back()];
        stack.pop_back();

        if (IsLeafRight(currentNode))
        {
            leaves_[currentNode.rightChild]->IntersectAll(intersectionInfos, ray);
        }
        else
        {
            float distance = 0;
            if (RayBoxIntersection(flatTree_[currentNode.rightChild].boundingBox.minCorner,
                                   flatTree_[currentNode.rightChild].boundingBox.maxCorner,
                                   ray,
                                   distance))
            {
                stack.push_back(currentNode.rightChild);
            }
        }

        if (IsLeafLeft(currentNode))
        {
            leaves_[currentNode.leftChild]->IntersectAll(intersectionInfos, ray);
        }
        else
        {
            float distance = 0;
            if (RayBoxIntersection(flatTree_[currentNode.leftChild].boundingBox.minCorner,
                                   flatTree_[currentNode.leftChild].boundingBox.maxCorner,
                                   ray,
                                   distance))
            {
                stack.push_back(currentNode.leftChild);
            }
        }
    }

    return hit;
}

std::vector<std::uint8_t> DBVH::Serialize() const
{
    return {}; // TODO
}

std::unique_ptr<IIntersectable> DBVH::Deserialize(const std::span<const std::uint8_t> buffer) const
{
    return {}; // TODO
}

BoundingBox DBVH::GetBoundaries() const { return flatTree_[0].boundingBox; }

float DBVH::GetSurfaceArea() const { return nodeMetadata_[0].surfaceArea; }

bool DBVH::operator==(const IIntersectable& object) const
{
    // TODO
    return false;
}

bool DBVH::operator!=(const IIntersectable& object) const
{
    // TODO
    return true;
}
