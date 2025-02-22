#pragma once

#include "RayTraceEngine/Shader.h"
#include "SFML/Graphics.hpp"
#include "Shaders/Common.h"
#include <chrono>

class Camera
{
  public:
    Camera(ShaderResourceHandle* camInfo, const float speed, const float sensitivity)
        : camInfo_(camInfo),
          speed_(speed),
          sensitivity_(sensitivity),
          lastStamp_(std::chrono::high_resolution_clock::now()),
          drag_(false),
          lastMousePos_(),
          refDir_()
    {
    }

    void BeginDrag(const sf::RenderWindow& window)
    {
        lastMousePos_ = sf::Mouse::getPosition(window);
        refDir_       = camInfo_->Map<CameraInfo>().cameraDirection;
        drag_         = true;
    }

    void EndDrag()
    {
        if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
            drag_ = false;
    }

    void Update(const sf::RenderWindow& window)
    {
        auto&      cam = camInfo_->Map<CameraInfo>();
        const auto relativeSpeed =
            speed_ * ((std::chrono::high_resolution_clock::now() - lastStamp_).count() / 1'000'000'000.f);
        const auto worldUp = Vector3D{0, 1, 0};
        auto       right   = worldUp.Cross(cam.cameraDirection);
        right.Normalize();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            cam.cameraPosition += cam.cameraDirection * relativeSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            cam.cameraPosition -= cam.cameraDirection * relativeSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            cam.cameraPosition -= right * relativeSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            cam.cameraPosition += right * relativeSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            cam.cameraPosition += worldUp * relativeSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
            cam.cameraPosition -= worldUp * relativeSpeed;

        if (drag_)
        {
            const auto  mouseDelta = sf::Mouse::getPosition(window) - lastMousePos_;
            const float jaw        = -mouseDelta.x * sensitivity_ * 3.14159f / 180.f;
            float       pitch      = -mouseDelta.y * sensitivity_;
            pitch                  = std::min(89.f, std::max(-89.f, pitch)) * 3.14159f / 180.f;

            cam.cameraDirection = {refDir_.x * std::cosf(jaw) - refDir_.z * std::sinf(jaw),
                                   refDir_.y + pitch,
                                   refDir_.x * std::sinf(jaw) + refDir_.z * std::cosf(jaw)};
            cam.cameraDirection.Normalize();
            cam.cameraUp = cam.cameraDirection.Cross(right);
            cam.cameraUp.Normalize();
        }

        lastStamp_ = std::chrono::high_resolution_clock::now();
    }

  private:
    ShaderResourceHandle* camInfo_;

    float                                              speed_;
    float                                              sensitivity_;
    std::chrono::time_point<std::chrono::steady_clock> lastStamp_;
    bool                                               drag_;
    sf::Vector2i                                       lastMousePos_;
    Vector3D                                           refDir_;
};