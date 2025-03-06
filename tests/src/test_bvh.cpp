#include <Catch2/catch_amalgamated.hpp>

#include "bvh/DBVH.h"

#include <cstring>

namespace
{
    class TestSphereIntersectable : public IIntersectable
    {
      public:
        TestSphereIntersectable(const Vector3D& center, const float radius, const std::uint64_t id)
            : center_(center), radius_(radius), id_(id)
        {
        }

        Vector3D GetCenter() const { return center_; }

        float GetRadius() const { return radius_; }

        std::uint64_t GetId() const { return id_; }

        std::vector<std::uint8_t> Serialize() const
        {
            std::vector<std::uint8_t> buffer(sizeof(center_) + sizeof(radius_));
            std::memcpy(buffer.data(), &center_, sizeof(center_));
            std::memcpy(buffer.data() + sizeof(center_), &radius_, sizeof(radius_));
            std::memcpy(buffer.data() + sizeof(center_) + sizeof(radius_), &id_, sizeof(id_));
            return buffer;
        }

        std::unique_ptr<IIntersectable> Deserialize(std::span<const std::uint8_t> buffer) const
        {
            Vector3D c;
            std::memcpy(&c, buffer.data(), sizeof(c));
            float r;
            std::memcpy(&r, buffer.data() + sizeof(c), sizeof(r));
            std::uint64_t id;
            std::memcpy(&id, buffer.data() + sizeof(c) + sizeof(r), sizeof(id));
            return std::make_unique<TestSphereIntersectable>(c, r, id);
        }

        BoundingBox GetBoundaries() const
        {
            BoundingBox bbox{};
            bbox.minCorner = center_ - Vector3D{radius_, radius_, radius_};
            bbox.maxCorner = center_ + Vector3D{radius_, radius_, radius_};
            return bbox;
        }

        bool IntersectFirst(IntersectionInfo& intersectionInfo, const Ray& ray) const
        {
            const auto toCenter = center_ - ray.origin;
            const auto tca      = toCenter.Dot(ray.direction);
            const auto d2       = toCenter.Dot(toCenter) - tca * tca;
            if (d2 > radius_ * radius_)
                return false;
            float      thc = sqrt(radius_ * radius_ - d2);
            const auto t0  = tca - thc;
            const auto t1  = tca + thc;
            if (t1 < 0)
                return false;

            intersectionInfo.hit      = true;
            intersectionInfo.distance = t0 > 0 ? t0 : t1;
            intersectionInfo.position = ray.origin + ray.direction * intersectionInfo.distance;
            intersectionInfo.normal   = intersectionInfo.position - center_;
            intersectionInfo.normal.Normalize();
            intersectionInfo.rayDirection = ray.direction;
            intersectionInfo.rayOrigin    = ray.origin;
            intersectionInfo.instanceId   = id_;

            return true;
        }

        bool IntersectAny(IntersectionInfo& intersectionInfo, const Ray& ray) const
        {
            return IntersectFirst(intersectionInfo, ray);
        }

        bool IntersectAll(std::vector<IntersectionInfo>& intersectionInfo, const Ray& ray) const
        {
            IntersectionInfo info{false, std::numeric_limits<float>::max()};
            if (IntersectFirst(info, ray))
            {
                intersectionInfo.push_back(std::move(info));
                return true;
            }
            return false;
        }

        float GetSurfaceArea() const { return GetBoundaries().GetSA(); }

        bool operator==(const IIntersectable& object) const
        {
            if (const auto* sphere = dynamic_cast<const TestSphereIntersectable*>(&object))
            {
                return (sphere->center_.x == center_.x) && (sphere->center_.y == center_.y) &&
                       (sphere->center_.z == center_.z) && (sphere->radius_ == radius_);
            }
            return false;
        }

        bool operator!=(const IIntersectable& object) const { return !(*this == object); }

      private:
        Vector3D      center_;
        float         radius_;
        std::uint64_t id_;
    };

    const TestSphereIntersectable sphere1({3.1f, -1.1f, 436.f}, 1.4f, 1);
    const TestSphereIntersectable sphere2({31.1f, -1.1f, -0.1f}, 11.1f, 2);
    const TestSphereIntersectable sphere3({-0.1f, 78.2f, -0.1f}, 2.2f, 3);
    const TestSphereIntersectable sphere4({1.f, 2.f, 3.f}, 4.f, 4);
    const TestSphereIntersectable sphere5({-41.4f, -23.2f, 3.3f}, 0.1f, 5);

    void CompareIntersections(const IntersectionInfo& i1, const IntersectionInfo& i2)
    {
        CHECK(i1.hit == i2.hit);
        CHECK(i1.distance == i2.distance);
        CHECK(((i1.normal.x == i2.normal.x) && (i1.normal.y == i2.normal.y) && (i1.normal.z == i2.normal.z)));
        CHECK(
            ((i1.position.x == i2.position.x) && (i1.position.y == i2.position.y) && (i1.position.z == i2.position.z)));
        CHECK(((i1.rayDirection.x == i2.rayDirection.x) && (i1.rayDirection.y == i2.rayDirection.y) &&
               (i1.rayDirection.z == i2.rayDirection.z)));
        CHECK(((i1.rayOrigin.x == i2.rayOrigin.x) && (i1.rayOrigin.y == i2.rayOrigin.y) &&
               (i1.rayOrigin.z == i2.rayOrigin.z)));
        CHECK(i1.instanceId == i2.instanceId);
        CHECK(((i1.texture.x == i2.texture.x) && (i1.texture.y == i2.texture.y)));
    }

    void CompareIntersections(const Vector3D&       rayOrigin,
                              const Vector3D&       rayDirection,
                              const IIntersectable& i1,
                              const IIntersectable& i2)
    {
        const Ray        ray = {rayOrigin, rayDirection, rayDirection.GetInverse()};
        IntersectionInfo info1{false, std::numeric_limits<float>::max()};
        i1.IntersectFirst(info1, ray);
        IntersectionInfo info2{false, std::numeric_limits<float>::max()};
        i2.IntersectFirst(info2, ray);
        CompareIntersections(info1, info2);
    }

    void CompareBoxes(const BoundingBox& b1, const BoundingBox& b2)
    {
        CHECK(((b1.minCorner.x == b2.minCorner.x) && (b1.minCorner.y == b2.minCorner.y) &&
               (b1.minCorner.z == b2.minCorner.z) && (b1.maxCorner.x == b2.maxCorner.x) &&
               (b1.maxCorner.y == b2.maxCorner.y) && (b1.maxCorner.z == b2.maxCorner.z)));
    }

    void CompareSingleSphereBvh(const DBVH& dbvh, const TestSphereIntersectable& sphere)
    {
        const auto bvhBox    = dbvh.GetBoundaries();
        const auto sphereBox = sphere.GetBoundaries();
        CompareBoxes(bvhBox, sphereBox);
        CHECK(dbvh.GetSurfaceArea() == sphere.GetSurfaceArea());
        constexpr Vector3D rayOrigin{2.3f, 1.9f, -3.2f};
        auto               rayDirection = (sphereBox.minCorner + sphereBox.maxCorner) / 2.f - rayOrigin;
        rayDirection.Normalize();
        CompareIntersections(rayOrigin, rayDirection, dbvh, sphere);
    }

    BoundingBox ComputeMinimalBoundingBox(const std::span<const IIntersectable*> intersectables)
    {
        BoundingBox box{};
        for (const auto* intersectable : intersectables)
        {
            const auto otherBox = intersectable->GetBoundaries();
            box.minCorner       = {std::min(box.minCorner.x, otherBox.minCorner.x),
                                   std::min(box.minCorner.y, otherBox.minCorner.y),
                                   std::min(box.minCorner.z, otherBox.minCorner.z)};
            box.maxCorner       = {std::max(box.maxCorner.x, otherBox.maxCorner.x),
                                   std::max(box.maxCorner.y, otherBox.maxCorner.y),
                                   std::max(box.maxCorner.z, otherBox.maxCorner.z)};
        }
        return box;
    }

    void CheckPierceIntersection(const Vector3D& rayOrigin, const Vector3D& rayDirection, const DBVH& dbvh)
    {
        const Ray                     ray = {rayOrigin, rayDirection, rayDirection.GetInverse()};
        std::vector<IntersectionInfo> infos{};
        dbvh.IntersectAll(infos, ray);
        REQUIRE(infos.size() == 2);
        if (infos[0].instanceId == sphere2.GetId())
        {
            CHECK(infos[0].hit);
            CHECK(infos[0].distance ==
                  Catch::Approx((sphere2.GetCenter() - ray.origin).GetLength() - sphere2.GetRadius()));
            CHECK(infos[1].instanceId == sphere4.GetId());
            CHECK(infos[1].hit);
            CHECK(infos[1].distance ==
                  Catch::Approx((sphere4.GetCenter() - ray.origin).GetLength() - sphere4.GetRadius()));
        }
        else
        {
            CHECK(infos[0].instanceId == sphere4.GetId());
            CHECK(infos[0].hit);
            CHECK(infos[0].distance ==
                  Catch::Approx((sphere4.GetCenter() - ray.origin).GetLength() - sphere4.GetRadius()));
            CHECK(infos[1].instanceId == sphere2.GetId());
            CHECK(infos[1].hit);
            CHECK(infos[1].distance ==
                  Catch::Approx((sphere2.GetCenter() - ray.origin).GetLength() - sphere2.GetRadius()));
        }
    }
} // namespace

TEST_CASE("TestSingleInsertion")
{
    const DBVH dbvh1{{&sphere1}};
    CompareSingleSphereBvh(dbvh1, sphere1);
    DBVH dbvh2{};
    dbvh2.AddObjects({&sphere1});
    CompareSingleSphereBvh(dbvh2, sphere1);
}

TEST_CASE("TestSingleInsertionAndDeletion")
{
    const BoundingBox defaultBox{};

    DBVH dbvh{{&sphere1}};
    dbvh.RemoveObjects({&sphere1});
    const auto box = dbvh.GetBoundaries();
    CompareBoxes(box, defaultBox);
    CHECK(dbvh.GetSurfaceArea() == 0.f);
    dbvh.AddObjects({&sphere1});
    CompareSingleSphereBvh(dbvh, sphere1);
}

TEST_CASE("TestMultipleInsertions")
{
    {
        const DBVH dbvh{{&sphere1, &sphere1}};
        const auto bvhBox    = dbvh.GetBoundaries();
        const auto sphereBox = sphere1.GetBoundaries();
        CompareBoxes(bvhBox, sphereBox);
        CHECK(dbvh.GetSurfaceArea() == (sphere1.GetSurfaceArea() * 2 + bvhBox.GetSA()));
        constexpr Vector3D rayOrigin{2.3f, 1.9f, -3.2f};
        auto               rayDirection = sphere1.GetCenter() - rayOrigin;
        rayDirection.Normalize();
        CompareIntersections(rayOrigin, rayDirection, dbvh, sphere1);
    }

    {
        std::vector<const IIntersectable*> spheres{&sphere1, &sphere2, &sphere3, &sphere4};
        const DBVH                         dbvh{spheres};
        const auto                         bvhBox      = dbvh.GetBoundaries();
        const BoundingBox                  combinedBox = ComputeMinimalBoundingBox(spheres);
        CompareBoxes(bvhBox, combinedBox);
        constexpr Vector3D rayOrigin{2.3f, 1.9f, -3.2f};
        auto               rayDirection = sphere3.GetCenter() - rayOrigin;
        rayDirection.Normalize();
        CompareIntersections(rayOrigin, rayDirection, dbvh, sphere3);
    }

    {
        DBVH dbvh{{&sphere1, &sphere2, &sphere3}};
        dbvh.AddObjects({&sphere4, &sphere5});
        dbvh.AddObjects({&sphere4});
        dbvh.AddObjects({&sphere4, &sphere5, &sphere1});
        dbvh.AddObjects({&sphere2});
        std::vector<const IIntersectable*> spheres{&sphere1, &sphere2, &sphere3, &sphere4, &sphere5};
        const auto                         bvhBox      = dbvh.GetBoundaries();
        const BoundingBox                  combinedBox = ComputeMinimalBoundingBox(spheres);
        CompareBoxes(bvhBox, combinedBox);
        constexpr Vector3D rayOrigin{2.3f, 1.9f, -3.2f};
        auto               rayDirection = sphere5.GetCenter() / 2.f - rayOrigin;
        rayDirection.Normalize();
        CompareIntersections(rayOrigin, rayDirection, dbvh, sphere5);
    }
}

TEST_CASE("TestMultipleInsertionsAndDeletions")
{
    DBVH dbvh{{&sphere1, &sphere2, &sphere3}};
    for (int i = 0; i < 5; ++i)
    {
        dbvh.RemoveObjects({&sphere4, &sphere5});
        dbvh.AddObjects({&sphere4, &sphere5});
        dbvh.RemoveObjects({&sphere4});
        dbvh.AddObjects({&sphere4});
        dbvh.RemoveObjects({&sphere4, &sphere5, &sphere1});
        dbvh.AddObjects({&sphere4, &sphere5, &sphere1});
        dbvh.RemoveObjects({&sphere2});
        dbvh.AddObjects({&sphere2});
    }
    std::vector<const IIntersectable*> spheres{&sphere1, &sphere2, &sphere3, &sphere4, &sphere5};
    const auto                         bvhBox      = dbvh.GetBoundaries();
    const BoundingBox                  combinedBox = ComputeMinimalBoundingBox(spheres);
    CompareBoxes(bvhBox, combinedBox);
    constexpr Vector3D rayOrigin{2.3f, 1.9f, -3.2f};
    auto               rayDirection = sphere5.GetCenter() - rayOrigin;
    rayDirection.Normalize();
    CompareIntersections(rayOrigin, rayDirection, dbvh, sphere5);
}

TEST_CASE("TestClosestIntersection")
{
    const DBVH dbvh({&sphere2, &sphere4});

    auto rayDirection = sphere2.GetCenter() - sphere4.GetCenter();
    rayDirection.Normalize();
    Vector3D rayOrigin = sphere4.GetCenter() - rayDirection * sphere4.GetRadius() * 2;

    {
        const Ray        ray = {rayOrigin, rayDirection, rayDirection.GetInverse()};
        IntersectionInfo info{false, std::numeric_limits<float>::max()};
        dbvh.IntersectFirst(info, ray);
        CHECK(info.hit);
        CHECK(info.instanceId == sphere4.GetId());
        CHECK(info.distance == Catch::Approx((sphere4.GetCenter() - ray.origin).GetLength() - sphere4.GetRadius()));
    }
    rayDirection *= -1.f;
    {
        const Ray        ray = {rayOrigin, rayDirection, rayDirection.GetInverse()};
        IntersectionInfo info{false, std::numeric_limits<float>::max()};
        dbvh.IntersectFirst(info, ray);
        CHECK(!info.hit);
    }
    rayOrigin = sphere2.GetCenter() - rayDirection * sphere2.GetRadius() * 2;
    {
        const Ray        ray = {rayOrigin, rayDirection, rayDirection.GetInverse()};
        IntersectionInfo info{false, std::numeric_limits<float>::max()};
        dbvh.IntersectFirst(info, ray);
        CHECK(info.hit);
        CHECK(info.instanceId == sphere2.GetId());
        CHECK(info.distance == Catch::Approx((sphere2.GetCenter() - ray.origin).GetLength() - sphere2.GetRadius()));
    }
}

TEST_CASE("TestOcclusionIntersection")
{
    const DBVH dbvh({&sphere2});

    auto rayDirection = sphere2.GetCenter();
    rayDirection.Normalize();
    const Vector3D rayOrigin = {0, 0, 0};

    {
        const Ray        ray = {rayOrigin, rayDirection, rayDirection.GetInverse()};
        IntersectionInfo info{false, std::numeric_limits<float>::max()};
        dbvh.IntersectAny(info, ray);
        CHECK(info.hit);
        CHECK(info.instanceId == sphere2.GetId());
        CHECK(info.distance == Catch::Approx((sphere2.GetCenter() - ray.origin).GetLength() - sphere2.GetRadius()));
    }
    rayDirection *= -1.f;
    {
        const Ray        ray = {rayOrigin, rayDirection, rayDirection.GetInverse()};
        IntersectionInfo info{false, std::numeric_limits<float>::max()};
        dbvh.IntersectAny(info, ray);
        CHECK(!info.hit);
    }
}

TEST_CASE("TestPierceIntersection")
{
    const DBVH dbvh({&sphere2, &sphere4});

    auto rayDirection = sphere2.GetCenter() - sphere4.GetCenter();
    rayDirection.Normalize();
    Vector3D rayOrigin = sphere4.GetCenter() - rayDirection * sphere4.GetRadius() * 2;

    {
        CheckPierceIntersection(rayOrigin, rayDirection, dbvh);
    }
    rayDirection *= -1.f;
    {
        const Ray                     ray = {rayOrigin, rayDirection, rayDirection.GetInverse()};
        std::vector<IntersectionInfo> infos{};
        dbvh.IntersectAll(infos, ray);
        CHECK(infos.empty());
    }
    rayOrigin = sphere2.GetCenter() - rayDirection * sphere2.GetRadius() * 2;
    {
        CheckPierceIntersection(rayOrigin, rayDirection, dbvh);
    }
}