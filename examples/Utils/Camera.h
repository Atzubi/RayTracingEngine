#pragma once

#include "RayTraceEngine/Shader.h"
#include "RayTraceEngine/Vector3D.h"
#include "SFML/Graphics.hpp"

#include <chrono>

class Camera
{
  public:
    Camera(ShaderResourceHandle* camInfo, const float speed, const float sensitivity);

    void BeginDrag(const sf::RenderWindow& window);
    void EndDrag();

    void Update(const sf::RenderWindow& window);

  private:
    ShaderResourceHandle* camInfo_;

    float                                          speed_;
    float                                          sensitivity_;
    std::chrono::high_resolution_clock::time_point lastStamp_;
    bool                                           drag_;
    sf::Vector2i                                   lastMousePos_;
    Vector3D                                       refDir_;
};