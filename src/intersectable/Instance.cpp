#include "intersectable/Instance.h"
#include "engine_node/EngineNode.h"

namespace
{
    inline Vector3D GetCenter(const BoundingBox& aabb) { return (aabb.maxCorner + aabb.minCorner) / 2.0; }

    void ApplyTransformToBox(BoundingBox& aabb, const Matrix4x4& transform)
    {
        const auto center = GetCenter(aabb);

        aabb.minCorner -= center;
        aabb.maxCorner -= center;

        const auto frontBottomLeft  = transform * aabb.minCorner;
        const auto frontBottomRight = transform * Vector3D{aabb.maxCorner.x, aabb.minCorner.y, aabb.minCorner.z};
        const auto frontTopLeft     = transform * Vector3D{aabb.minCorner.x, aabb.maxCorner.y, aabb.minCorner.z};
        const auto frontTopRight    = transform * Vector3D{aabb.maxCorner.x, aabb.maxCorner.y, aabb.minCorner.z};
        const auto backBottomLeft   = transform * Vector3D{aabb.minCorner.x, aabb.minCorner.y, aabb.maxCorner.z};
        const auto backBottomRight  = transform * Vector3D{aabb.maxCorner.x, aabb.minCorner.y, aabb.maxCorner.z};
        const auto backTopLeft      = transform * Vector3D{aabb.minCorner.x, aabb.maxCorner.y, aabb.maxCorner.z};
        const auto backTopRight     = transform * aabb.maxCorner;

        for (int i = 0; i < 3; ++i)
        {
            aabb.minCorner[i] = std::min(
                std::min(
                    std::min(std::min(std::min(std::min(std::min(backTopRight[i], backTopLeft[i]), backBottomRight[i]),
                                               backBottomLeft[i]),
                                      frontTopRight[i]),
                             frontTopLeft[i]),
                    frontBottomRight[i]),
                frontBottomLeft[i]);
            aabb.maxCorner[i] = std::max(
                std::max(
                    std::max(std::max(std::max(std::min(std::max(backTopRight[i], backTopLeft[i]), backBottomRight[i]),
                                               backBottomLeft[i]),
                                      frontTopRight[i]),
                             frontTopLeft[i]),
                    frontBottomRight[i]),
                frontBottomLeft[i]);
        }

        aabb.minCorner += center;
        aabb.maxCorner += center;
    }

    bool IsTransformEqual(const Matrix4x4& transform1, const Matrix4x4& transform2)
    {
        for (int x = 0; x < 4; ++x)
        {
            for (int y = 0; y < 4; ++y)
            {
                if (transform1.elements[x][y] != transform2.elements[x][y])
                    return false;
            }
        }
        return true;
    }

    inline Ray CreateTransformedRay(const Ray& ray, const Matrix4x4& inverseTransform, const Vector3D& originalMid)
    {
        auto transformedDirection = inverseTransform.MultiplyTransform(ray.direction);
        transformedDirection.Normalize();

        return {inverseTransform * (ray.origin - originalMid) + originalMid,
                transformedDirection,
                transformedDirection.GetInverse()};
    }

    inline void
    ReverseTransformHit(IntersectionInfo& info, const Ray& ray, const Matrix4x4& transform, const Vector3D& originalMid)
    {
        const auto pos = info.position - originalMid;
        info.position  = transform * pos + originalMid;
        info.normal    = transform.MultiplyTransform(info.normal);
        info.normal.Normalize();
        info.distance = (ray.origin - info.position).GetLength();
    }
} // namespace

Instance::Instance(const IIntersectable* intersectible, std::function<void()> fetchCallBack, const std::uint64_t id)
    : intersectible_(intersectible), fetchCallBack_(std::move(fetchCallBack)), id_(id)
{
    cost_             = intersectible_->GetSurfaceArea();
    boundingBox_      = intersectible_->GetBoundaries();
    transform_        = Matrix4x4::GetIdentity();
    inverseTransform_ = Matrix4x4::GetIdentity();
}

void Instance::ApplyTransform(const Matrix4x4& newTransform)
{
    boundingBox_      = intersectible_->GetBoundaries();
    transform_        = newTransform * transform_;
    inverseTransform_ = transform_.GetInverse();
    ApplyTransformToBox(boundingBox_, transform_);
}

Matrix4x4 Instance::GetTransform() const { return transform_; }

std::vector<std::uint8_t> Instance::Serialize() const
{
    return {}; // TODO
}

std::unique_ptr<IIntersectable> Instance::Deserialize(const std::span<const std::uint8_t> buffer) const
{
    return {}; // TODO
}

bool Instance::IntersectFirst(IntersectionInfo& intersectionInfo, const Ray& ray) const
{
    fetchCallBack_();
    const auto       newRay = CreateTransformedRay(ray, inverseTransform_, GetCenter(intersectible_->GetBoundaries()));
    IntersectionInfo info{false, std::numeric_limits<float>::max()};
    if (!intersectible_->IntersectFirst(info, newRay))
        return false;

    ReverseTransformHit(info, ray, transform_, GetCenter(intersectible_->GetBoundaries()));
    if (info.distance >= intersectionInfo.distance)
        return false;

    intersectionInfo            = info;
    intersectionInfo.instanceId = id_;
    return true;
}

bool Instance::IntersectAny(IntersectionInfo& intersectionInfo, const Ray& ray) const
{
    fetchCallBack_();
    const auto       newRay = CreateTransformedRay(ray, inverseTransform_, GetCenter(intersectible_->GetBoundaries()));
    IntersectionInfo info{false, std::numeric_limits<float>::max()};
    if (!intersectible_->IntersectAny(info, newRay))
        return false;
    ReverseTransformHit(info, ray, transform_, GetCenter(intersectible_->GetBoundaries()));
    intersectionInfo            = info;
    intersectionInfo.instanceId = id_;
    return true;
}

bool Instance::IntersectAll(std::vector<IntersectionInfo>& intersectionInfo, const Ray& ray) const
{
    fetchCallBack_();
    const auto newRay = CreateTransformedRay(ray, inverseTransform_, GetCenter(intersectible_->GetBoundaries()));
    std::vector<IntersectionInfo> infos;
    if (!intersectible_->IntersectAll(infos, newRay))
        return false;
    for (auto& info : infos)
    {
        ReverseTransformHit(info, ray, transform_, GetCenter(intersectible_->GetBoundaries()));
        info.instanceId = id_;
        intersectionInfo.push_back(std::move(info));
    }
    return true;
}

BoundingBox Instance::GetBoundaries() const { return boundingBox_; }

float Instance::GetSurfaceArea() const
{
    return cost_ + boundingBox_.GetSA(); // TODO: fix math
}

bool Instance::operator==(const IIntersectable& object) const
{
    const auto obj = dynamic_cast<const Instance*>(&object);
    if (obj == nullptr || obj->intersectible_ != intersectible_)
        return false;
    return IsTransformEqual(obj->transform_, transform_);
}

bool Instance::operator!=(const IIntersectable& object) const { return !operator==(object); }
