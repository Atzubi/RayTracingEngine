#include "Intersectables/TriangleMeshObject.h"
#include "bvh/DBVHv2.h"
#include <cmath>

class Triangle : public IIntersectable
{
  private:
    void setTexture(IntersectionInfo& intersectionInfo, double_t u, double_t v, double_t w) const
    {
        Vector2D texture1 = mesh->vertices[mesh->indices[pos]].texture;
        Vector2D texture2 = mesh->vertices[mesh->indices[pos + 1]].texture;
        Vector2D texture3 = mesh->vertices[mesh->indices[pos + 2]].texture;

        intersectionInfo.texture.x = w * texture1.x + u * texture2.x + v * texture3.x;
        intersectionInfo.texture.y = w * texture1.y + u * texture2.y + v * texture3.y;
    }

    void setNormal(IntersectionInfo& intersectionInfo, double_t u, double_t v, double_t w) const
    {
        Vector3D normal1 = mesh->vertices[mesh->indices[pos]].normal;
        Vector3D normal2 = mesh->vertices[mesh->indices[pos + 1]].normal;
        Vector3D normal3 = mesh->vertices[mesh->indices[pos + 2]].normal;

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
        intersectionInfo.material = &mesh->material;
        intersectionInfo.hit      = true;
    }

  public:
    TriangleMeshObject* mesh{};
    uint64_t            pos{};

    Triangle() = default;

    [[nodiscard]] std::unique_ptr<IIntersectable> Clone() const override { return nullptr; }

    [[nodiscard]] BoundingBox GetBoundaries() const override
    {
        Vector3D vertex1 = mesh->vertices[mesh->indices[pos]].position;
        Vector3D vertex2 = mesh->vertices[mesh->indices[pos + 1]].position;
        Vector3D vertex3 = mesh->vertices[mesh->indices[pos + 2]].position;

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
        Vector3D vertex1 = mesh->vertices[mesh->indices[pos]].position;
        Vector3D vertex2 = mesh->vertices[mesh->indices[pos + 1]].position;
        Vector3D vertex3 = mesh->vertices[mesh->indices[pos + 2]].position;

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
            Vector3D vertex1 = mesh->vertices[mesh->indices[pos]].position;
            Vector3D vertex2 = mesh->vertices[mesh->indices[pos + 1]].position;
            Vector3D vertex3 = mesh->vertices[mesh->indices[pos + 2]].position;

            Vector3D otherVertex1 = triangle->mesh->vertices[triangle->mesh->indices[pos]].position;
            Vector3D otherVertex2 = triangle->mesh->vertices[triangle->mesh->indices[pos + 1]].position;
            Vector3D otherVertex3 = triangle->mesh->vertices[triangle->mesh->indices[pos + 2]].position;

            return otherVertex1.x == vertex1.x && otherVertex1.y == vertex1.y && otherVertex1.z == vertex1.z &&
                   otherVertex2.x == vertex2.x && otherVertex2.y == vertex2.y && otherVertex2.z == vertex2.z &&
                   otherVertex3.x == vertex3.x && otherVertex3.y == vertex3.y && otherVertex3.z == vertex3.z;
        }
    }

    bool operator!=(const IIntersectable& object) const override { return !operator==(object); }

    ~Triangle() override = default;
};

TriangleMeshObject::TriangleMeshObject(const std::vector<Vertex>*   vertices,
                                       const std::vector<uint64_t>* indices,
                                       const Material*              material)
{
    if (indices->size() % 3 != 0)
    {
        throw std::invalid_argument("Invalid Index Count");
    }

    this->vertices = *vertices;
    this->indices  = *indices;
    this->material = *material;

    std::vector<IIntersectable*> objects;
    for (unsigned long i = 0; i < indices->size() / 3; i++)
    {
        auto triangle  = std::make_unique<Triangle>();
        triangle->mesh = this;
        triangle->pos  = i * 3;
        objects.push_back(triangle.get());
        triangles.push_back(std::move(triangle));
    }

    structure = std::move(DBVHv2(objects));
}

TriangleMeshObject::~TriangleMeshObject() = default;

BoundingBox TriangleMeshObject::GetBoundaries() const { return structure.GetBoundaries(); }

bool TriangleMeshObject::IntersectFirst(IntersectionInfo& intersectionInfo, const Ray& ray) const
{
    return structure.IntersectFirst(intersectionInfo, ray);
}

bool TriangleMeshObject::IntersectAny(IntersectionInfo& intersectionInfo, const Ray& ray) const
{
    return structure.IntersectAny(intersectionInfo, ray);
}

bool TriangleMeshObject::IntersectAll(std::vector<IntersectionInfo>& intersectionInfo, const Ray& ray) const
{
    return structure.IntersectAll(intersectionInfo, ray);
}

std::unique_ptr<IIntersectable> TriangleMeshObject::Clone() const
{
    // TODO
    return std::make_unique<TriangleMeshObject>(&vertices, &indices, &material);
}

double TriangleMeshObject::GetSurfaceArea() const { return structure.GetSurfaceArea(); }

bool TriangleMeshObject::operator==(const IIntersectable& object) const
{
    // TODO
    return false;
}

bool TriangleMeshObject::operator!=(const IIntersectable& object) const { return !operator==(object); }
