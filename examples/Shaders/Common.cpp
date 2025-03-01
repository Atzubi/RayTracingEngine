#include "Common.h"

#include <cstring>

std::vector<std::uint8_t> Material::Serialize() const
{
    const auto mapKa   = map_Ka.Serialize();
    const auto mapKd   = map_Kd.Serialize();
    const auto mapKs   = map_Ks.Serialize();
    const auto mapNs   = map_Ns.Serialize();
    const auto mapD    = map_d.Serialize();
    const auto mapBump = map_bump.Serialize();
    const auto size    = sizeof(Ka) + sizeof(Kd) + sizeof(Ks) + sizeof(Ns) + sizeof(Ni) + sizeof(d) + sizeof(illum) +
                      mapKa.size() + mapKd.size() + mapKs.size() + mapNs.size() + mapD.size() + mapBump.size();
    std::vector<std::uint8_t> buffer(size);
    std::memcpy(buffer.data(), &Ka, sizeof(Ka));
    auto offset = sizeof(Ka);
    std::memcpy(buffer.data() + offset, &Kd, sizeof(Kd));
    offset += sizeof(Kd);
    std::memcpy(buffer.data() + offset, &Ks, sizeof(Ks));
    offset += sizeof(Ks);
    std::memcpy(buffer.data() + offset, &Ns, sizeof(Ns));
    offset += sizeof(Ns);
    std::memcpy(buffer.data() + offset, &Ni, sizeof(Ni));
    offset += sizeof(Ni);
    std::memcpy(buffer.data() + offset, &d, sizeof(d));
    offset += sizeof(d);
    std::memcpy(buffer.data() + offset, &illum, sizeof(illum));
    offset += sizeof(illum);
    std::memcpy(buffer.data() + offset, mapKa.data(), mapKa.size());
    offset += mapKa.size();
    std::memcpy(buffer.data() + offset, mapKd.data(), mapKd.size());
    offset += mapKd.size();
    std::memcpy(buffer.data() + offset, mapKs.data(), mapKs.size());
    offset += mapKs.size();
    std::memcpy(buffer.data() + offset, mapNs.data(), mapNs.size());
    offset += mapNs.size();
    std::memcpy(buffer.data() + offset, mapD.data(), mapD.size());
    offset += mapD.size();
    std::memcpy(buffer.data() + offset, mapBump.data(), mapBump.size());
    return buffer;
}

std::unique_ptr<IShaderResource> Material::Deserialize(const std::span<const std::uint8_t> buffer) const
{
    auto material = std::make_unique<Material>();
    std::memcpy(&material->Ka, buffer.data(), sizeof(material->Ka));
    auto offset = sizeof(material->Kd);
    std::memcpy(&material->Kd, buffer.data() + offset, sizeof(material->Kd));
    offset += sizeof(material->Kd);
    std::memcpy(&material->Ks, buffer.data() + offset, sizeof(material->Ks));
    offset += sizeof(material->Ks);
    std::memcpy(&material->Ns, buffer.data() + offset, sizeof(material->Ns));
    offset += sizeof(material->Ns);
    std::memcpy(&material->Ni, buffer.data() + offset, sizeof(material->Ni));
    offset += sizeof(material->Ni);
    std::memcpy(&material->d, buffer.data() + offset, sizeof(material->d));
    offset += sizeof(material->d);
    std::memcpy(&material->illum, buffer.data() + offset, sizeof(material->illum));
    offset += sizeof(material->illum);
    const auto mapKa = material->map_Ka.Deserialize({buffer.begin() + offset, buffer.end()});
    material->map_Ka = *dynamic_cast<const Texture*>(mapKa.get());
    offset += sizeof(std::uint32_t) * 3 + material->map_Ka.w * material->map_Ka.h * material->map_Ka.bytesPerTexel;
    const auto mapKd = material->map_Kd.Deserialize({buffer.begin() + offset, buffer.end()});
    material->map_Kd = *dynamic_cast<const Texture*>(mapKd.get());
    offset += sizeof(std::uint32_t) * 3 + material->map_Kd.w * material->map_Kd.h * material->map_Kd.bytesPerTexel;
    const auto mapKs = material->map_Ks.Deserialize({buffer.begin() + offset, buffer.end()});
    material->map_Ks = *dynamic_cast<const Texture*>(mapKs.get());
    offset += sizeof(std::uint32_t) * 3 + material->map_Ks.w * material->map_Ks.h * material->map_Ks.bytesPerTexel;
    const auto mapNs = material->map_Ns.Deserialize({buffer.begin() + offset, buffer.end()});
    material->map_Ns = *dynamic_cast<const Texture*>(mapNs.get());
    offset += sizeof(std::uint32_t) * 3 + material->map_Ns.w * material->map_Ns.h * material->map_Ns.bytesPerTexel;
    const auto mapD = material->map_d.Deserialize({buffer.begin() + offset, buffer.end()});
    material->map_d = *dynamic_cast<const Texture*>(mapD.get());
    offset += sizeof(std::uint32_t) * 3 + material->map_d.w * material->map_d.h * material->map_d.bytesPerTexel;
    const auto mapBump = material->map_bump.Deserialize({buffer.begin() + offset, buffer.end()});
    material->map_bump = *dynamic_cast<const Texture*>(mapBump.get());
    return material;
}

std::vector<std::uint8_t> CameraInfo::Serialize() const
{
    std::vector<std::uint8_t> buffer(sizeof(Vector3D) * 3);
    std::memcpy(buffer.data(), &cameraPosition, sizeof(Vector3D));
    auto offset = sizeof(Vector3D);
    std::memcpy(buffer.data() + offset, &cameraUp, sizeof(Vector3D));
    offset += sizeof(Vector3D);
    std::memcpy(buffer.data() + offset, &cameraDirection, sizeof(Vector3D));
    return buffer;
}

std::unique_ptr<IShaderResource> CameraInfo::Deserialize(const std::span<const std::uint8_t> buffer) const
{
    auto cam = std::make_unique<CameraInfo>();
    std::memcpy(&cam->cameraPosition, buffer.data(), sizeof(Vector3D));
    auto offset = sizeof(Vector3D);
    std::memcpy(&cam->cameraUp, buffer.data() + offset, sizeof(Vector3D));
    offset += sizeof(Vector3D);
    std::memcpy(&cam->cameraDirection, buffer.data() + offset, sizeof(Vector3D));
    return cam;
}

std::vector<std::uint8_t> ViewportInfo::Serialize() const
{
    std::vector<std::uint8_t> buffer(sizeof(std::uint32_t) * 2);
    std::memcpy(buffer.data(), &viewPortWidth, sizeof(std::uint32_t));
    auto offset = sizeof(std::uint32_t);
    std::memcpy(buffer.data() + offset, &viewPortHeight, sizeof(std::uint32_t));
    return buffer;
}

std::unique_ptr<IShaderResource> ViewportInfo::Deserialize(const std::span<const std::uint8_t> buffer) const
{
    auto viewport = std::make_unique<ViewportInfo>();
    std::memcpy(&viewport->viewPortWidth, buffer.data(), sizeof(std::uint32_t));
    auto offset = sizeof(std::uint32_t);
    std::memcpy(&viewport->viewPortHeight, buffer.data() + offset, sizeof(std::uint32_t));
    return viewport;
}

std::vector<std::uint8_t> SampleCountInfo::Serialize() const
{
    std::vector<std::uint8_t> buffer(sizeof(std::uint32_t));
    std::memcpy(buffer.data(), &samplesPerPixel, sizeof(std::uint32_t));
    return buffer;
}

std::unique_ptr<IShaderResource> SampleCountInfo::Deserialize(const std::span<const std::uint8_t> buffer) const
{
    auto sampleCount = std::make_unique<SampleCountInfo>();
    std::memcpy(&sampleCount->samplesPerPixel, buffer.data(), sizeof(std::uint32_t));
    return sampleCount;
}

std::vector<std::uint8_t> PathData::Serialize() const
{
    const auto size =
        sizeof(std::size_t) * 2 + sizeof(Vector3D) * absorption.size() + sizeof(std::uint32_t) * depth.size();
    std::vector<std::uint8_t> buffer(size);
    const auto                absorptionSize = absorption.size() * sizeof(Vector3D);
    std::memcpy(buffer.data(), &absorptionSize, sizeof(absorptionSize));
    auto offset = sizeof(absorptionSize);
    std::memcpy(buffer.data() + offset, absorption.data(), absorptionSize);
    offset += absorptionSize;
    const auto depthSize = depth.size() * sizeof(std::uint32_t);
    std::memcpy(buffer.data() + offset, &depthSize, sizeof(depthSize));
    offset += sizeof(depthSize);
    std::memcpy(buffer.data() + offset, depth.data(), depthSize);
    return buffer;
}

std::unique_ptr<IShaderResource> PathData::Deserialize(const std::span<const std::uint8_t> buffer) const
{
    auto        pathData = std::make_unique<PathData>();
    std::size_t absorptionSize;
    std::memcpy(&absorptionSize, buffer.data(), sizeof(absorptionSize));
    auto offset = sizeof(absorptionSize);
    pathData->absorption.resize(absorptionSize / sizeof(Vector3D));
    std::memcpy(pathData->absorption.data(), buffer.data() + offset, absorptionSize);
    offset += absorptionSize;
    std::size_t depthSize;
    std::memcpy(&depthSize, buffer.data() + offset, sizeof(depthSize));
    offset += sizeof(depthSize);
    pathData->depth.resize(depthSize / sizeof(std::uint32_t));
    std::memcpy(pathData->depth.data(), buffer.data() + offset, depthSize);
    return pathData;
}

std::vector<std::uint8_t> MaterialMap::Serialize() const
{
    const auto                size = 2 * sizeof(std::uint64_t) * instanceToMaterial.size();
    std::vector<std::uint8_t> buffer(size);
    std::size_t               offset = 0;
    for (const auto& entry : instanceToMaterial)
    {
        std::memcpy(buffer.data() + offset, &entry.first, sizeof(entry.first));
        offset += sizeof(entry.first);
        std::memcpy(buffer.data() + offset, &entry.second, sizeof(entry.second));
        offset += sizeof(entry.second);
    }
    return buffer;
}

std::unique_ptr<IShaderResource> MaterialMap::Deserialize(const std::span<const std::uint8_t> buffer) const
{
    auto        materialMap = std::make_unique<MaterialMap>();
    std::size_t offset      = 0;
    for (std::size_t i = 0; i < buffer.size() / (sizeof(std::uint64_t) * 2); ++i)
    {
        std::uint64_t key;
        std::memcpy(&key, buffer.data() + offset, sizeof(key));
        offset += sizeof(key);
        std::uint64_t value;
        std::memcpy(&value, buffer.data() + offset, sizeof(value));
        offset += sizeof(value);
        materialMap->instanceToMaterial[key] = value;
    }
    return materialMap;
}

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

Vector3D LoadKdTexel(const IntersectionInfo& info, const Material& material)
{
    Vector3D color = material.Kd;
    if (!material.map_Kd.image.empty())
    {
        const auto w = material.map_Kd.w;
        const auto h = material.map_Kd.h;
        auto       x = fmod(info.texture.x, 1.0);
        auto       y = -fmod(info.texture.y, 1.0);
        if (x < 0)
            x = 1 + x;
        if (y < 0)
            y = 1 + y;

        const auto pixelCoordinate = ((std::uint64_t)((w - 1) * x)) + w * ((std::uint64_t)((h - 1) * y));

        color.x = material.map_Kd.image[pixelCoordinate * 3] / 255.f;
        color.y = material.map_Kd.image[pixelCoordinate * 3 + 1] / 255.f;
        color.z = material.map_Kd.image[pixelCoordinate * 3 + 2] / 255.f;
    }
    return color;
}
