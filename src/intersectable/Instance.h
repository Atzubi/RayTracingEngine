#pragma once

#include "RayTraceEngine/BasicStructures.h"
#include "RayTraceEngine/Intersectable.h"
#include "RayTraceEngine/Matrix4x4.h"

#include <functional>

class Instance : public IIntersectable
{
  public:
    Instance(const IIntersectable* intersectible, std::function<void()> fetchCallBack, std::uint64_t id);

    void ApplyTransform(const Matrix4x4& newTransform);

    Matrix4x4 GetTransform() const;

    std::vector<std::uint8_t> Serialize() const override;

    std::unique_ptr<IIntersectable> Deserialize(std::span<const std::uint8_t> buffer) const override;

    BoundingBox GetBoundaries() const override;

    bool IntersectFirst(IntersectionInfo& intersectionInfo, const Ray& ray) const override;

    bool IntersectAny(IntersectionInfo& intersectionInfo, const Ray& ray) const override;

    bool IntersectAll(std::vector<IntersectionInfo>& intersectionInfo, const Ray& ray) const override;

    float GetSurfaceArea() const override;

    bool operator==(const IIntersectable& object) const override;

    bool operator!=(const IIntersectable& object) const override;

  private:
    const IIntersectable* intersectible_;
    std::function<void()> fetchCallBack_;
    std::uint64_t         id_;

    float       cost_;
    BoundingBox boundingBox_{};
    Matrix4x4   transform_{};
    Matrix4x4   inverseTransform_{};
};
