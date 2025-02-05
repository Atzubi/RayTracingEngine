#pragma once

#include "RayTraceEngine/BasicStructures.h"
#include "RayTraceEngine/Intersectable.h"
#include "RayTraceEngine/Matrix4x4.h"

#include <functional>

class DataManagementUnitV2;

class Instance : public IIntersectable
{
  public:
    explicit Instance(const IIntersectable* intersectible, std::function<void()> fetchCallBack);

    void ApplyTransform(const Matrix4x4& newTransform);

    Matrix4x4 GetTransform() const;

    [[nodiscard]] std::unique_ptr<IIntersectable> Clone() const override;

    [[nodiscard]] BoundingBox GetBoundaries() const override;

    bool IntersectFirst(IntersectionInfo& intersectionInfo, const Ray& ray) const override;

    bool IntersectAny(IntersectionInfo& intersectionInfo, const Ray& ray) const override;

    bool IntersectAll(std::vector<IntersectionInfo>& intersectionInfo, const Ray& ray) const override;

    [[nodiscard]] double GetSurfaceArea() const override;

    bool operator==(const IIntersectable& object) const override;

    bool operator!=(const IIntersectable& object) const override;

  private:
    const IIntersectable* intersectible_;
    std::function<void()> fetchCallBack_;

    double      cost_;
    BoundingBox boundingBox_{};
    Matrix4x4   transform_{};
    Matrix4x4   inverseTransform_{};
};
