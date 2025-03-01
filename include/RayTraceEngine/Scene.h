#pragma once

#include "BasicStructures.h"
#include "Intersectable.h"
#include "Matrix4x4.h"

#include <span>
#include <vector>

struct IntersectableDescription
{
    const IntersectableObjectHandle& intersectable;
    Matrix4x4                        transform;
    ObjectParameter                  objectParameters;
};

struct SceneDescription
{
    std::vector<IntersectableDescription> intersectables;
};

class InstanceHandle
{
  public:
    virtual std::uint64_t GetId() const = 0;

    virtual void Transform(const Matrix4x4& transform) = 0;

    virtual Matrix4x4 GetTransform() const = 0;

    virtual ~InstanceHandle() = default;
};

class SceneHandle
{
  public:
    virtual std::span<const std::unique_ptr<InstanceHandle>> GetInstanceHandles() const = 0;

    virtual ~SceneHandle() = default;
};