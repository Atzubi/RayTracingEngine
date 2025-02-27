
#pragma once

#include "RayTraceEngine/Shader.h"
#include "RayTraceEngine/Vector3D.h"

#include <cstdint>
#include <vector>

struct CameraInfo : public IShaderResource
{
    Vector3D cameraPosition;
    Vector3D cameraUp;
    Vector3D cameraDirection;

    std::unique_ptr<IShaderResource> Clone() const override { return std::make_unique<CameraInfo>(*this); }
};

struct ViewportInfo : public IShaderResource
{
    std::uint32_t viewPortWidth;
    std::uint32_t viewPortHeight;

    std::unique_ptr<IShaderResource> Clone() const override { return std::make_unique<ViewportInfo>(*this); }
};

struct SampleCountInfo : public IShaderResource
{
    std::uint32_t samplesPerPixel;

    std::unique_ptr<IShaderResource> Clone() const override { return std::make_unique<SampleCountInfo>(*this); }
};

struct PathData : public IShaderResource
{
    std::vector<Vector3D>     absorption;
    std::vector<std::uint8_t> depth;

    std::unique_ptr<IShaderResource> Clone() const override { return std::make_unique<PathData>(*this); }
};

Vector3D LambertReflection(const Vector3D& normal)
{
    constexpr auto scale = RAND_MAX / 2.f;
    Vector3D       lambertianVector{std::rand() / scale - 1.f, std::rand() / scale - 1.f, std::rand() / scale - 1.f};
    while (lambertianVector.Dot(lambertianVector) > 1)
        lambertianVector = {std::rand() / scale - 1.f, std::rand() / scale - 1.f, std::rand() / scale - 1.f};
    if (lambertianVector.Dot(normal) < 0)
        lambertianVector *= -1;
    lambertianVector.Normalize();
    return lambertianVector;
}

float FresnelSchlick(const float cosTheta, const float n1, const float n2)
{
    const auto R0 = powf((n1 - n2) / (n1 + n2), 2);
    return R0 + (1 - R0) * powf(1 - cosTheta, 5);
}

Vector3D Reflect(const Vector3D& incident, const Vector3D& normal)
{
    return incident - normal * 2 * incident.Dot(normal);
}

Vector3D Refract(const Vector3D& incident, const Vector3D& normal, const float n1, const float n2)
{
    const auto eta        = n1 / n2;
    const auto cosThetaI  = -incident.Dot(normal);
    const auto sin2ThetaT = eta * eta * (1 - cosThetaI * cosThetaI);

    if (sin2ThetaT > 1.0f)
        return Reflect(incident, normal); // Total internal reflection

    const auto cosThetaT = sqrt(1 - sin2ThetaT);
    return incident * eta + normal * (eta * cosThetaI - cosThetaT);
}

Vector3D LoadKdTexel(const IntersectionInfo& info)
{
    Vector3D color = info.material->Kd;
    if (info.material && info.material->map_Kd && !info.material->map_Kd->image.empty())
    {
        const auto w = info.material->map_Kd->w;
        const auto h = info.material->map_Kd->h;
        auto       x = fmod(info.texture.x, 1.0);
        auto       y = -fmod(info.texture.y, 1.0);
        if (x < 0)
            x = 1 + x;
        if (y < 0)
            y = 1 + y;

        const auto pixelCoordinate = ((std::uint64_t)((w - 1) * x)) + w * ((std::uint64_t)((h - 1) * y));

        color.x = info.material->map_Kd->image[pixelCoordinate * 3] / 255.f;
        color.y = info.material->map_Kd->image[pixelCoordinate * 3 + 1] / 255.f;
        color.z = info.material->map_Kd->image[pixelCoordinate * 3 + 2] / 255.f;
    }
    return color;
}