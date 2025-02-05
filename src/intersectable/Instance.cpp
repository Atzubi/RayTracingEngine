#include "intersectable/Instance.h"
#include "engine_node/EngineNode.h"
#include <algorithm>
#include <complex>
#include <utility>

namespace
{
    inline Vector3D getCenter(const BoundingBox& aabb) { return (aabb.maxCorner + aabb.minCorner) / 2.0; }

    void moveBoxToCenter(BoundingBox& aabb, const Vector3D& center)
    {
        aabb.minCorner -= center;
        aabb.maxCorner -= center;
    }

    void moveBoxBackToOriginalPosition(BoundingBox& aabb, const Vector3D& center)
    {
        aabb.minCorner += center;
        aabb.maxCorner += center;
    }

    void setNewBox(BoundingBox&    aabb,
                   const Vector3D& frontBottomLeft,
                   const Vector3D& frontBottomRight,
                   const Vector3D& frontTopLeft,
                   const Vector3D& frontTopRight,
                   const Vector3D& backBottomLeft,
                   const Vector3D& backBottomRight,
                   const Vector3D& backTopLeft,
                   const Vector3D& backTopRight)
    {
        for (int i = 0; i < 3; i++)
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
    }

    void transformOldBox(const BoundingBox& aabb,
                         const Matrix4x4&   transform,
                         Vector3D&          frontBottomLeft,
                         Vector3D&          frontBottomRight,
                         Vector3D&          frontTopLeft,
                         Vector3D&          frontTopRight,
                         Vector3D&          backBottomLeft,
                         Vector3D&          backBottomRight,
                         Vector3D&          backTopLeft,
                         Vector3D&          backTopRight)
    {
        frontBottomLeft  = aabb.minCorner;
        frontBottomRight = {aabb.maxCorner.x, aabb.minCorner.y, aabb.minCorner.z};
        frontTopLeft     = {aabb.minCorner.x, aabb.maxCorner.y, aabb.minCorner.z};
        frontTopRight    = {aabb.maxCorner.x, aabb.maxCorner.y, aabb.minCorner.z};
        backBottomLeft   = {aabb.minCorner.x, aabb.minCorner.y, aabb.maxCorner.z};
        backBottomRight  = {aabb.maxCorner.x, aabb.minCorner.y, aabb.maxCorner.z};
        backTopLeft      = {aabb.minCorner.x, aabb.maxCorner.y, aabb.maxCorner.z};
        backTopRight     = aabb.maxCorner;

        frontBottomLeft  = transform * frontBottomLeft;
        frontBottomRight = transform * frontBottomRight;
        frontTopLeft     = transform * frontTopLeft;
        frontTopRight    = transform * frontTopRight;
        backBottomLeft   = transform * backBottomLeft;
        backBottomRight  = transform * backBottomRight;
        backTopLeft      = transform * backTopLeft;
        backTopRight     = transform * backTopRight;
    }

    void applyTransformToBox(BoundingBox& aabb, const Matrix4x4& transform)
    {
        Vector3D frontBottomLeft{};
        Vector3D frontBottomRight{};
        Vector3D frontTopLeft{};
        Vector3D frontTopRight{};
        Vector3D backBottomLeft{};
        Vector3D backBottomRight{};
        Vector3D backTopLeft{};
        Vector3D backTopRight{};

        transformOldBox(aabb,
                        transform,
                        frontBottomLeft,
                        frontBottomRight,
                        frontTopLeft,
                        frontTopRight,
                        backBottomLeft,
                        backBottomRight,
                        backTopLeft,
                        backTopRight);

        setNewBox(aabb,
                  frontBottomLeft,
                  frontBottomRight,
                  frontTopLeft,
                  frontTopRight,
                  backBottomLeft,
                  backBottomRight,
                  backTopLeft,
                  backTopRight);
    }

    void createTransformedAABB(BoundingBox& aabb, const Matrix4x4& transform)
    {
        Vector3D center = getCenter(aabb);

        moveBoxToCenter(aabb, center);

        applyTransformToBox(aabb, transform);

        moveBoxBackToOriginalPosition(aabb, center);
    }

    bool isTransformEqual(const Matrix4x4& transform1, const Matrix4x4& transform2)
    {
        for (int x = 0; x < 4; x++)
        {
            for (int y = 0; y < 4; y++)
            {
                if (transform1.elements[x][y] != transform2.elements[x][y])
                    return false;
            }
        }
        return true;
    }

    inline void transformRay(const Vector3D& originalMid, Ray& newRay, const Matrix4x4& inverseTransform)
    {
        newRay.origin -= originalMid;
        newRay.direction += newRay.origin;

        newRay.origin    = inverseTransform * newRay.origin;
        newRay.direction = inverseTransform * newRay.direction;

        newRay.direction -= newRay.origin;
        newRay.direction.normalize();

        newRay.dirfrac = newRay.direction.getInverse();

        newRay.origin += originalMid;
    }

    inline void reverseTransformHit(const Ray& ray, IntersectionInfo& info, const Matrix4x4& transform)
    {
        Vector3D pos  = info.position;
        info.position = transform * pos;
        info.normal   = transform * (info.normal + pos) - info.position;
        info.normal.normalize();
        info.distance = (ray.origin - info.position).getLength();
    }

    inline bool overwriteClosestHit(IntersectionInfo& intersectionInfo,
                                    const Ray&        ray,
                                    IntersectionInfo& info,
                                    bool              hit,
                                    const Matrix4x4&  transform)
    {
        if (!hit || info.distance >= intersectionInfo.distance)
            return false;
        reverseTransformHit(ray, info, transform);
        intersectionInfo = info;
        return true;
    }

    inline bool overwriteAnyHit(IntersectionInfo& intersectionInfo,
                                const Ray&        ray,
                                IntersectionInfo& info,
                                bool              hit,
                                const Matrix4x4&  transform)
    {
        if (!hit)
            return false;
        reverseTransformHit(ray, info, transform);
        intersectionInfo = info;
        return true;
    }

    inline bool overwriteAllHit(std::vector<IntersectionInfo>& intersectionInfo,
                                const Ray&                     ray,
                                std::vector<IntersectionInfo>& infos,
                                bool                           hit,
                                const Matrix4x4&               transform)
    {
        if (!hit)
            return false;
        for (auto info : infos)
        {
            reverseTransformHit(ray, info, transform);
            intersectionInfo.push_back(info);
        }
        return true;
    }

    inline Ray createTransformedRay(const Ray& ray, const IIntersectable* baseObject, Matrix4x4 inverseTransform)
    {
        BoundingBox originalAABB = baseObject->GetBoundaries();
        Vector3D    originalMid  = getCenter(originalAABB);

        Ray newRay = ray;
        transformRay(originalMid, newRay, inverseTransform);
        return newRay;
    }
} // namespace

Instance::Instance(const IIntersectable* intersectible, std::function<void()> fetchCallBack)
    : intersectible_(intersectible), fetchCallBack_(fetchCallBack)
{
    cost_             = intersectible_->GetSurfaceArea();
    boundingBox_      = intersectible_->GetBoundaries();
    transform_        = Matrix4x4::getIdentity();
    inverseTransform_ = Matrix4x4::getIdentity();
}

void Instance::ApplyTransform(const Matrix4x4& newTransform)
{
    boundingBox_ = intersectible_->GetBoundaries();
    transform_.multiplyBy(newTransform);
    inverseTransform_ = transform_.getInverse();
    createTransformedAABB(boundingBox_, transform_);
}

Matrix4x4 Instance::GetTransform() const { return transform_; }

bool Instance::IntersectFirst(IntersectionInfo& intersectionInfo, const Ray& ray) const
{
    fetchCallBack_();
    Ray              newRay = createTransformedRay(ray, intersectible_, inverseTransform_);
    IntersectionInfo info{false, std::numeric_limits<double>::max()};
    bool             hit = intersectible_->IntersectFirst(info, newRay);
    return overwriteClosestHit(intersectionInfo, ray, info, hit, transform_);
}

bool Instance::IntersectAny(IntersectionInfo& intersectionInfo, const Ray& ray) const
{
    fetchCallBack_();
    Ray              newRay = createTransformedRay(ray, intersectible_, inverseTransform_);
    IntersectionInfo info{false, std::numeric_limits<double>::max()};
    bool             hit = intersectible_->IntersectAny(info, newRay);
    return overwriteAnyHit(intersectionInfo, ray, info, hit, transform_);
}

bool Instance::IntersectAll(std::vector<IntersectionInfo>& intersectionInfo, const Ray& ray) const
{
    fetchCallBack_();
    Ray                           newRay = createTransformedRay(ray, intersectible_, inverseTransform_);
    std::vector<IntersectionInfo> infos;
    bool                          hit = intersectible_->IntersectAll(infos, newRay);
    return overwriteAllHit(intersectionInfo, ray, infos, hit, transform_);
}

BoundingBox Instance::GetBoundaries() const { return boundingBox_; }

std::unique_ptr<IIntersectable> Instance::Clone() const { return nullptr; }

double Instance::GetSurfaceArea() const
{
    return cost_ + boundingBox_.getSA(); // TODO: fix math
}

bool Instance::operator==(const IIntersectable& object) const
{
    const auto obj = dynamic_cast<const Instance*>(&object);
    if (obj == nullptr || obj->intersectible_ != intersectible_)
        return false;
    return isTransformEqual(obj->transform_, transform_);
}

bool Instance::operator!=(const IIntersectable& object) const { return !operator==(object); }
