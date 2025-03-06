#pragma once

#include "BasicStructures.h"
#include "Intersectable.h"
#include "Matrix4x4.h"

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

/**
 * Contains all information necessary to create an instance in the engine.
 * intersectable:    Reference to an intersectable object.
 * transform:        The new transform of the intersectable in world space.
 * objectParameters: Further parameters.
 */
struct InstanceDescription
{
    const IntersectableObjectHandle* intersectable;
    Matrix4x4                        transform;
    ObjectParameter                  objectParameters;
};

/**
 * Resource handle. When this handle goes out of scope the resource is freed in the engine.
 */
class InstanceHandle
{
  public:
    /**
     * Get the unique id of this instance. This id will match the one in the intersection info if this instance was
     * intersected.
     * @return: The unique id.
     */
    virtual std::uint64_t GetId() const = 0;

    /**
     * Apply (not set!) a tranform.
     * @param transform: The transform to be applied.
     */
    virtual void Transform(const Matrix4x4& transform) = 0;

    /**
     * Get the current transform of the instance.
     * @return The current tansform.
     */
    virtual Matrix4x4 GetTransform() const = 0;

    virtual ~InstanceHandle() = default;
};

/**
 * Contains all information necessary to create a scene in the engine.
 * instances:    Instances to be created from intersectable objects.
 */
struct SceneDescription
{
    std::vector<InstanceHandle*> instances;
};

/**
 * Resource handle. When this handle goes out of scope the resource is freed in the engine.
 */
class SceneHandle
{
  public:
    /**
     * Adds an instance to the scene.
     * @param instance:  The instance to be added.
     */
    virtual void AddInstance(InstanceHandle& instance) = 0;

    /**
     * Removes an instance from the scene.
     * @param instance:  The instance to be removed.
     */
    virtual void RemoveInstance(InstanceHandle& instance) = 0;

    virtual ~SceneHandle() = default;
};