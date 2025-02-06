#include "Intersectables/TriangleMeshObject.h"
#include "bvh/DBVHv2.h"
#include <cmath>
#include <cstdint>
#include <span>

class Triangle : public IIntersectable
{
  public:
    Triangle(std::span<const TriangleMeshObject::Vertex> vertices,
             std::span<const std::uint64_t, 3>           indices,
             const Material*                             material)
        : vertices_(vertices), indices_(indices), material_(material) {};

    [[nodiscard]] std::unique_ptr<IIntersectable> Clone() const override { return nullptr; }

    [[nodiscard]] BoundingBox GetBoundaries() const override
    {
        Vector3D vertex1 = vertices_[indices_[0]].position;
        Vector3D vertex2 = vertices_[indices_[1]].position;
        Vector3D vertex3 = vertices_[indices_[2]].position;

        Vector3D front{std::min(std::min(vertex1.x, vertex2.x), vertex3.x),
                       std::min(std::min(vertex1.y, vertex2.y), vertex3.y),
                       std::min(std::min(vertex1.z, vertex2.z), vertex3.z)};

        Vector3D back{std::max(std::max(vertex1.x, vertex2.x), vertex3.x),
                      std::max(std::max(vertex1.y, vertex2.y), vertex3.y),
                      std::max(std::max(vertex1.z, vertex2.z), vertex3.z)};
        return {front, back};
    }

    bool IntersectFirst(IntersectionInfo& intersectionInfo, const Ray& ray) const override
    {
        Vector3D vertex1 = vertices_[indices_[0]].position;
        Vector3D vertex2 = vertices_[indices_[1]].position;
        Vector3D vertex3 = vertices_[indices_[2]].position;

        Vector3D e1 = vertex2 - vertex1;
        Vector3D e2 = vertex3 - vertex1;

        Vector3D pvec = ray.direction.cross(e2);
        double_t det  = (pvec * e1).sum();

        double_t epsilon = 0.000001f;

        if (det < epsilon && det > -epsilon)
        {
            intersectionInfo.hit = false;
            return false;
        }

        double_t invDet = 1.0 / det;
        Vector3D tvec   = ray.origin - vertex1;
        double_t u      = invDet * (tvec * pvec).sum();

        if (u < 0.0f || u > 1.0f)
        {
            intersectionInfo.hit = false;
            return false;
        }

        Vector3D qvec = tvec.cross(e1);
        double_t v    = invDet * (qvec * ray.direction).sum();

        if (v < 0.0f || u + v > 1.0f)
        {
            intersectionInfo.hit = false;
            return false;
        }

        double t = invDet * (e2 * qvec).sum();

        if (t <= epsilon)
        {
            intersectionInfo.hit = false;
            return false;
        }

        double_t w = 1 - u - v;

        setIntersection(intersectionInfo, ray, u, v, t, w);
        return true;
    }

    bool IntersectAny(IntersectionInfo& intersectionInfo, const Ray& ray) const override
    {
        return IntersectFirst(intersectionInfo, ray);
    }

    bool IntersectAll(std::vector<IntersectionInfo>& intersectionInfo, const Ray& ray) const override
    {
        IntersectionInfo info{false, std::numeric_limits<double>::max()};
        bool             hit = IntersectFirst(info, ray);
        intersectionInfo.push_back(info);
        return hit;
    }

    [[nodiscard]] double GetSurfaceArea() const override { return GetBoundaries().getSA(); }

    bool operator==(const IIntersectable& object) const override
    {
        const auto* triangle = dynamic_cast<const Triangle*>(&object);
        if (triangle == nullptr)
        {
            return false;
        }
        else
        {
            Vector3D vertex1 = vertices_[indices_[0]].position;
            Vector3D vertex2 = vertices_[indices_[1]].position;
            Vector3D vertex3 = vertices_[indices_[2]].position;

            Vector3D otherVertex1 = triangle->vertices_[triangle->indices_[0]].position;
            Vector3D otherVertex2 = triangle->vertices_[triangle->indices_[1]].position;
            Vector3D otherVertex3 = triangle->vertices_[triangle->indices_[2]].position;

            return otherVertex1.x == vertex1.x && otherVertex1.y == vertex1.y && otherVertex1.z == vertex1.z &&
                   otherVertex2.x == vertex2.x && otherVertex2.y == vertex2.y && otherVertex2.z == vertex2.z &&
                   otherVertex3.x == vertex3.x && otherVertex3.y == vertex3.y && otherVertex3.z == vertex3.z;
        }
    }

    bool operator!=(const IIntersectable& object) const override { return !operator==(object); }

    ~Triangle() override = default;

  private:
    void setTexture(IntersectionInfo& intersectionInfo, double_t u, double_t v, double_t w) const
    {
        Vector2D texture1 = vertices_[indices_[0]].texture;
        Vector2D texture2 = vertices_[indices_[1]].texture;
        Vector2D texture3 = vertices_[indices_[2]].texture;

        intersectionInfo.texture.x = w * texture1.x + u * texture2.x + v * texture3.x;
        intersectionInfo.texture.y = w * texture1.y + u * texture2.y + v * texture3.y;
    }

    void setNormal(IntersectionInfo& intersectionInfo, double_t u, double_t v, double_t w) const
    {
        Vector3D normal1 = vertices_[indices_[0]].normal;
        Vector3D normal2 = vertices_[indices_[1]].normal;
        Vector3D normal3 = vertices_[indices_[2]].normal;

        intersectionInfo.normal = (normal1 * w) + (normal2 * u) + (normal3 * v);
        intersectionInfo.normal.normalize();
    }

    void setIntersection(IntersectionInfo& intersectionInfo,
                         const Ray&        ray,
                         double_t          u,
                         double_t          v,
                         double            t,
                         double_t          w) const
    {
        intersectionInfo.position = ray.origin + (ray.direction * t);
        intersectionInfo.distance = (ray.origin - intersectionInfo.position).getLength();
        setNormal(intersectionInfo, u, v, w);
        setTexture(intersectionInfo, u, v, w);
        intersectionInfo.material = material_;
        intersectionInfo.hit      = true;
    }

    std::span<const TriangleMeshObject::Vertex> vertices_;
    std::span<const std::uint64_t, 3>           indices_;
    const Material*                             material_;
};

class TriangleMeshObject::TrianglMeshImpl : public IIntersectable
{
  public:
    TrianglMeshImpl(std::vector<Vertex> vertices, std::vector<std::uint64_t> indices, Material material)
        : vertices_(std::move(vertices)), indices_(std::move(indices)), material_(std::move(material))
    {
        if (indices_.size() % 3 != 0)
        {
            throw std::invalid_argument("Invalid Index Count");
        }

        std::vector<IIntersectable*> objects;
        for (unsigned long i = 0; i < indices_.size() / 3; i++)
        {
            auto triangle = std::make_unique<Triangle>(
                vertices_,
                std::span<const std::uint64_t, 3>(indices_.begin() + i * 3, indices_.begin() + i * 3 + 3),
                &material_);
            objects.push_back(triangle.get());
            triangles_.push_back(std::move(triangle));
        }

        structure_ = DBVHv2(objects);
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

    std::unique_ptr<IIntersectable> Clone() const override
    {
        // TODO
        return std::make_unique<TriangleMeshObject>(vertices_, indices_, material_);
    }

    double GetSurfaceArea() const override { return structure_.GetSurfaceArea(); }

    bool operator==(const IIntersectable& object) const override
    {
        // TODO
        return false;
    }

    bool operator!=(const IIntersectable& object) const override { return !operator==(object); }

  private:
    std::vector<Vertex>   vertices_;
    std::vector<uint64_t> indices_;
    Material              material_;

    std::vector<std::unique_ptr<IIntersectable>> triangles_;
    DBVHv2                                       structure_;
};

TriangleMeshObject::TriangleMeshObject(std::vector<Vertex>        vertices,
                                       std::vector<std::uint64_t> indices,
                                       Material                   material)
{
    impl_ = std::make_unique<TrianglMeshImpl>(std::move(vertices), std::move(indices), std::move(material));
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

std::unique_ptr<IIntersectable> TriangleMeshObject::Clone() const { return impl_->Clone(); }

double TriangleMeshObject::GetSurfaceArea() const { return impl_->GetSurfaceArea(); }

bool TriangleMeshObject::operator==(const IIntersectable& object) const { return impl_->operator==(object); }

bool TriangleMeshObject::operator!=(const IIntersectable& object) const { return impl_->operator!=(object); }
