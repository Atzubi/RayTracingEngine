#include "intersectable/Instance.h"
#include "engine_node/EngineNode.h"

namespace
{
    inline Vector3D GetCenter(const BoundingBox& aabb) { return (aabb.maxCorner + aabb.minCorner) / 2.0; }

    void ApplyTransformToBox(BoundingBox& aabb, const Matrix4x4& transform)
    {
        Vector3D center = GetCenter(aabb);

        aabb.minCorner -= center;
        aabb.maxCorner -= center;

        Vector3D frontBottomLeft  = aabb.minCorner;
        Vector3D frontBottomRight = {aabb.maxCorner.x, aabb.minCorner.y, aabb.minCorner.z};
        Vector3D frontTopLeft     = {aabb.minCorner.x, aabb.maxCorner.y, aabb.minCorner.z};
        Vector3D frontTopRight    = {aabb.maxCorner.x, aabb.maxCorner.y, aabb.minCorner.z};
        Vector3D backBottomLeft   = {aabb.minCorner.x, aabb.minCorner.y, aabb.maxCorner.z};
        Vector3D backBottomRight  = {aabb.maxCorner.x, aabb.minCorner.y, aabb.maxCorner.z};
        Vector3D backTopLeft      = {aabb.minCorner.x, aabb.maxCorner.y, aabb.maxCorner.z};
        Vector3D backTopRight     = aabb.maxCorner;

        frontBottomLeft  = transform * frontBottomLeft;
        frontBottomRight = transform * frontBottomRight;
        frontTopLeft     = transform * frontTopLeft;
        frontTopRight    = transform * frontTopRight;
        backBottomLeft   = transform * backBottomLeft;
        backBottomRight  = transform * backBottomRight;
        backTopLeft      = transform * backTopLeft;
        backTopRight     = transform * backTopRight;

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
        Ray newRay = ray;
        newRay.origin -= originalMid;
        newRay.direction += newRay.origin;

        newRay.origin    = inverseTransform * newRay.origin;
        newRay.direction = inverseTransform * newRay.direction;

        newRay.direction -= newRay.origin;
        newRay.direction.Normalize();

        newRay.dirfrac = newRay.direction.GetInverse();

        newRay.origin += originalMid;
        return newRay;
    }

    inline void ReverseTransformHit(IntersectionInfo& info, const Ray& ray, const Matrix4x4& transform)
    {
        const auto pos = info.position;
        info.position  = transform * pos;
        info.normal    = transform * (info.normal + pos) - info.position;
        info.normal.Normalize();
        info.distance = (ray.origin - info.position).GetLength();
    }
} // namespace

Instance::Instance(const IIntersectable* intersectible, std::function<void()> fetchCallBack)
    : intersectible_(intersectible), fetchCallBack_(fetchCallBack)
{
    cost_             = intersectible_->GetSurfaceArea();
    boundingBox_      = intersectible_->GetBoundaries();
    transform_        = Matrix4x4::GetIdentity();
    inverseTransform_ = Matrix4x4::GetIdentity();
}

void Instance::ApplyTransform(const Matrix4x4& newTransform)
{
    boundingBox_ = intersectible_->GetBoundaries();
    transform_.MultiplyBy(newTransform);
    inverseTransform_ = transform_.GetInverse();
    ApplyTransformToBox(boundingBox_, transform_);
}

Matrix4x4 Instance::GetTransform() const { return transform_; }

bool Instance::IntersectFirst(IntersectionInfo& intersectionInfo, const Ray& ray) const
{
    fetchCallBack_();
    const auto       newRay = CreateTransformedRay(ray, inverseTransform_, GetCenter(intersectible_->GetBoundaries()));
    IntersectionInfo info{false, std::numeric_limits<float>::max()};
    if (!intersectible_->IntersectFirst(info, newRay))
        return false;

    ReverseTransformHit(info, ray, transform_);
    if (info.distance >= intersectionInfo.distance)
        return false;

    intersectionInfo = info;
    return true;
}

bool Instance::IntersectAny(IntersectionInfo& intersectionInfo, const Ray& ray) const
{
    fetchCallBack_();
    const auto       newRay = CreateTransformedRay(ray, inverseTransform_, GetCenter(intersectible_->GetBoundaries()));
    IntersectionInfo info{false, std::numeric_limits<float>::max()};
    if (!intersectible_->IntersectAny(info, newRay))
        return false;
    ReverseTransformHit(info, ray, transform_);
    intersectionInfo = info;
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
        ReverseTransformHit(info, ray, transform_);
        intersectionInfo.push_back(std::move(info));
    }
    return true;
}

BoundingBox Instance::GetBoundaries() const { return boundingBox_; }

std::unique_ptr<IIntersectable> Instance::Clone() const { return nullptr; }

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
