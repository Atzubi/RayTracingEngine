
#pragma once

struct CameraInfo : public IShaderResource
{
    Vector3D cameraPosition;
    Vector3D cameraUp;
    Vector3D cameraDirection;

    std::unique_ptr<IShaderResource> Clone() const override
    {
        auto clone             = std::make_unique<CameraInfo>();
        clone->cameraPosition  = cameraPosition;
        clone->cameraUp        = cameraUp;
        clone->cameraDirection = cameraDirection;
        return clone;
    }
};

struct ViewportInfo : public IShaderResource
{
    std::uint32_t viewPortWidth;
    std::uint32_t viewPortHeight;

    std::unique_ptr<IShaderResource> Clone() const override
    {
        auto clone            = std::make_unique<ViewportInfo>();
        clone->viewPortWidth  = viewPortWidth;
        clone->viewPortHeight = viewPortHeight;
        return clone;
    }
};