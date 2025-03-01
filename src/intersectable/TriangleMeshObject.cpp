#include "Intersectables/TriangleMeshObject.h"
#include "RayTraceEngine/BasicStructures.h"
#include "bvh/DBVH.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <span>

class Triangle : public IIntersectable
{
  public:
    Triangle(std::span<const TriangleMeshObject::Vertex> vertices, std::span<const std::uint32_t, 3> indices)
        : vertices_(vertices), indices_(indices) {};

    std::vector<std::uint8_t> Serialize() const override
    {
        return {}; // Not serializable
    }

    std::unique_ptr<IIntersectable> Deserialize(const std::span<const std::uint8_t> buffer) const override
    {
        return {}; // Not deserializable
    }

    [[nodiscard]] BoundingBox GetBoundaries() const override
    {
        const auto& vertex1 = vertices_[indices_[0]].position;
        const auto& vertex2 = vertices_[indices_[1]].position;
        const auto& vertex3 = vertices_[indices_[2]].position;

        const Vector3D front{std::min(std::min(vertex1.x, vertex2.x), vertex3.x),
                             std::min(std::min(vertex1.y, vertex2.y), vertex3.y),
                             std::min(std::min(vertex1.z, vertex2.z), vertex3.z)};

        const Vector3D back{std::max(std::max(vertex1.x, vertex2.x), vertex3.x),
                            std::max(std::max(vertex1.y, vertex2.y), vertex3.y),
                            std::max(std::max(vertex1.z, vertex2.z), vertex3.z)};
        return {front, back};
    }

    bool IntersectFirst(IntersectionInfo& intersectionInfo, const Ray& ray) const override
    {
        const auto& vertex1 = vertices_[indices_[0]].position;
        const auto& vertex2 = vertices_[indices_[1]].position;
        const auto& vertex3 = vertices_[indices_[2]].position;

        const auto& e1 = vertex2 - vertex1;
        const auto& e2 = vertex3 - vertex1;

        const auto& pvec = ray.direction.Cross(e2);
        const auto& det  = (pvec * e1).Sum();

        constexpr float epsilon = 0.000000000001f;

        if (det < epsilon && det > -epsilon)
        {
            intersectionInfo.hit = false;
            return false;
        }

        const auto invDet = 1.0 / det;
        const auto tvec   = ray.origin - vertex1;
        const auto u      = invDet * (tvec * pvec).Sum();

        if (u < 0.0f || u > 1.0f)
        {
            intersectionInfo.hit = false;
            return false;
        }

        const auto qvec = tvec.Cross(e1);
        const auto v    = invDet * (qvec * ray.direction).Sum();

        if (v < 0.0f || u + v > 1.0f)
        {
            intersectionInfo.hit = false;
            return false;
        }

        const auto t = invDet * (e2 * qvec).Sum();

        if (t <= epsilon)
        {
            intersectionInfo.hit = false;
            return false;
        }

        const auto w = 1 - u - v;

        SetIntersection(intersectionInfo, ray, u, v, t, w);
        return true;
    }

    bool IntersectAny(IntersectionInfo& intersectionInfo, const Ray& ray) const override
    {
        return IntersectFirst(intersectionInfo, ray);
    }

    bool IntersectAll(std::vector<IntersectionInfo>& intersectionInfo, const Ray& ray) const override
    {
        IntersectionInfo info{false, std::numeric_limits<float>::max()};
        const auto       hit = IntersectFirst(info, ray);
        intersectionInfo.push_back(info);
        return hit;
    }

    [[nodiscard]] float GetSurfaceArea() const override { return GetBoundaries().GetSA(); }

    bool operator==(const IIntersectable& object) const override
    {
        const auto* triangle = dynamic_cast<const Triangle*>(&object);
        if (triangle == nullptr)
        {
            return false;
        }
        else
        {
            const auto& vertex1 = vertices_[indices_[0]].position;
            const auto& vertex2 = vertices_[indices_[1]].position;
            const auto& vertex3 = vertices_[indices_[2]].position;

            const auto& otherVertex1 = triangle->vertices_[triangle->indices_[0]].position;
            const auto& otherVertex2 = triangle->vertices_[triangle->indices_[1]].position;
            const auto& otherVertex3 = triangle->vertices_[triangle->indices_[2]].position;

            return otherVertex1.x == vertex1.x && otherVertex1.y == vertex1.y && otherVertex1.z == vertex1.z &&
                   otherVertex2.x == vertex2.x && otherVertex2.y == vertex2.y && otherVertex2.z == vertex2.z &&
                   otherVertex3.x == vertex3.x && otherVertex3.y == vertex3.y && otherVertex3.z == vertex3.z;
        }
    }

    bool operator!=(const IIntersectable& object) const override { return !operator==(object); }

    ~Triangle() override = default;

  private:
    void SetTexture(IntersectionInfo& intersectionInfo, const float u, const float v, const float w) const
    {
        const auto& texture1 = vertices_[indices_[0]].texture;
        const auto& texture2 = vertices_[indices_[1]].texture;
        const auto& texture3 = vertices_[indices_[2]].texture;

        intersectionInfo.texture.x = w * texture1.x + u * texture2.x + v * texture3.x;
        intersectionInfo.texture.y = w * texture1.y + u * texture2.y + v * texture3.y;
    }

    void SetNormal(IntersectionInfo& intersectionInfo, const float u, const float v, const float w) const
    {
        const auto& normal1 = vertices_[indices_[0]].normal;
        const auto& normal2 = vertices_[indices_[1]].normal;
        const auto& normal3 = vertices_[indices_[2]].normal;

        intersectionInfo.normal = (normal1 * w) + (normal2 * u) + (normal3 * v);
        intersectionInfo.normal.Normalize();
    }

    void SetIntersection(IntersectionInfo& intersectionInfo,
                         const Ray&        ray,
                         const float       u,
                         const float       v,
                         const float       t,
                         const float       w) const
    {
        intersectionInfo.position = ray.origin + (ray.direction * t);
        intersectionInfo.distance = (ray.origin - intersectionInfo.position).GetLength();
        SetNormal(intersectionInfo, u, v, w);
        SetTexture(intersectionInfo, u, v, w);
        intersectionInfo.hit = true;
    }

    std::span<const TriangleMeshObject::Vertex> vertices_;
    std::span<const std::uint32_t, 3>           indices_;
};

class TriangleMeshObject::TrianglMeshImpl : public IIntersectable
{
  public:
    TrianglMeshImpl(std::vector<Vertex> vertices, std::vector<std::uint32_t> indices)
        : vertices_(std::move(vertices)), indices_(std::move(indices))
    {
        if (indices_.size() % 3 != 0)
        {
            throw std::invalid_argument("Invalid Index Count");
        }

        std::vector<const IIntersectable*> objects;
        for (unsigned long i = 0; i < indices_.size() / 3; i++)
        {
            auto triangle = std::make_unique<Triangle>(
                vertices_, std::span<const std::uint32_t, 3>(indices_.begin() + i * 3, indices_.begin() + i * 3 + 3));
            objects.push_back(triangle.get());
            triangles_.push_back(std::move(triangle));
        }

        structure_ = DBVH(objects);
    }

    BoundingBox GetBoundaries() const override { return structure_.GetBoundaries(); }

    bool IntersectFirst(IntersectionInfo& intersectionInfo, const Ray& ray) const override
    {
        return structure_.IntersectFirst(intersectionInfo, ray);
    }

    bool IntersectAny(IntersectionInfo& intersectionInfo, const Ray& ray) const override
    {
        return structure_.IntersectAny(intersectionInfo, ray);
    }

    bool IntersectAll(std::vector<IntersectionInfo>& intersectionInfo, const Ray& ray) const override
    {
        return structure_.IntersectAll(intersectionInfo, ray);
    }

    std::vector<std::uint8_t> Serialize() const override
    {
        const auto size =
            sizeof(std::size_t) * 2 + sizeof(Vertex) * vertices_.size() + sizeof(std::uint32_t) * indices_.size();
        std::vector<std::uint8_t> buffer(size);
        const auto                verticesSize = vertices_.size() * sizeof(Vertex);
        std::memcpy(buffer.data(), &verticesSize, sizeof(verticesSize));
        auto offset = sizeof(verticesSize);
        std::memcpy(buffer.data() + offset, vertices_.data(), verticesSize);
        offset += verticesSize;
        const auto indicesSize = indices_.size() * sizeof(std::uint32_t);
        std::memcpy(buffer.data() + offset, &indicesSize, sizeof(indicesSize));
        offset += sizeof(indicesSize);
        std::memcpy(buffer.data() + offset, indices_.data(), indicesSize);
        return buffer;
    }

    std::unique_ptr<IIntersectable> Deserialize(const std::span<const std::uint8_t> buffer) const override
    {
        std::vector<Vertex>        vertices;
        std::vector<std::uint32_t> indices;
        std::size_t                verticesSize;
        std::memcpy(&verticesSize, buffer.data(), sizeof(verticesSize));
        auto offset = sizeof(verticesSize);
        vertices.resize(verticesSize / sizeof(Vertex));
        std::memcpy(vertices.data(), buffer.data() + offset, verticesSize);
        offset += verticesSize;
        std::size_t indicesSize;
        std::memcpy(&indicesSize, buffer.data() + offset, sizeof(indicesSize));
        offset += sizeof(indicesSize);
        indices.resize(indicesSize / sizeof(std::uint32_t));
        std::memcpy(indices.data(), buffer.data() + offset, indicesSize);
        return std::make_unique<TriangleMeshObject>(std::move(vertices), std::move(indices));
    }

    float GetSurfaceArea() const override { return structure_.GetSurfaceArea(); }

    bool operator==(const IIntersectable& object) const override
    {
        // TODO
        return false;
    }

    bool operator!=(const IIntersectable& object) const override { return !operator==(object); }

  private:
    std::vector<Vertex>   vertices_;
    std::vector<uint32_t> indices_;

    std::vector<std::unique_ptr<IIntersectable>> triangles_;
    DBVH                                         structure_;
};

TriangleMeshObject::TriangleMeshObject(std::vector<Vertex> vertices, std::vector<std::uint32_t> indices)
{
    impl_ = std::make_unique<TrianglMeshImpl>(std::move(vertices), std::move(indices));
}

TriangleMeshObject::~TriangleMeshObject() = default;

BoundingBox TriangleMeshObject::GetBoundaries() const { return impl_->GetBoundaries(); }

bool TriangleMeshObject::IntersectFirst(IntersectionInfo& intersectionInfo, const Ray& ray) const
{
    return impl_->IntersectFirst(intersectionInfo, ray);
}

bool TriangleMeshObject::IntersectAny(IntersectionInfo& intersectionInfo, const Ray& ray) const
{
    return impl_->IntersectAny(intersectionInfo, ray);
}

bool TriangleMeshObject::IntersectAll(std::vector<IntersectionInfo>& intersectionInfo, const Ray& ray) const
{
    return impl_->IntersectAll(intersectionInfo, ray);
}

std::vector<std::uint8_t> TriangleMeshObject::Serialize() const { return impl_->Serialize(); }

std::unique_ptr<IIntersectable> TriangleMeshObject::Deserialize(const std::span<const std::uint8_t> buffer) const
{
    return impl_->Deserialize(buffer);
}
float TriangleMeshObject::GetSurfaceArea() const { return impl_->GetSurfaceArea(); }

bool TriangleMeshObject::operator==(const IIntersectable& object) const { return impl_->operator==(object); }

bool TriangleMeshObject::operator!=(const IIntersectable& object) const { return impl_->operator!=(object); }
