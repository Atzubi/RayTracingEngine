#pragma once

#include "Intersectable.h"

struct SceneDescription
{
    std::vector<IntersectableDescription> intersectables;
};

class InstanceHandle
{
  public:
    void Update(Matrix4x4 transform);
};

class SceneHandle
{
  public:
    InstanceHandle AddIntersectable(const IntersectableDescription& desc);
};