#pragma once

#include "RayTraceEngine/BasicStructures.h"
#include "SFML/Graphics.hpp"

void DisplayTexture(sf::RenderWindow& window, const Texture& texture)
{
    // For displaying the texture with SFML, we may need to extend from our textures RGB to RGBA
    constexpr int channelCount = 4;

    sf::Image image;

    if (texture.bytesPerTexel == 3)
    {
        std::vector<sf::Uint8> pixels(texture.w * texture.h * channelCount);

        const sf::Uint8 fullOpacity = 255;

        for (std::uint32_t x = 0; x < texture.w; ++x)
        {
            for (std::uint32_t y = 0; y < texture.h; ++y)
            {
                pixels[(x + y * texture.w) * channelCount + 0] = (texture.image)[(x + y * texture.w) * 3 + 0];
                pixels[(x + y * texture.w) * channelCount + 1] = (texture.image)[(x + y * texture.w) * 3 + 1];
                pixels[(x + y * texture.w) * channelCount + 2] = (texture.image)[(x + y * texture.w) * 3 + 2];
                pixels[(x + y * texture.w) * channelCount + 3] = fullOpacity;
            }
        }
        // Create image
        image.create(texture.w, texture.h, pixels.data());
    }
    else if (texture.bytesPerTexel == 4)
    {
        image.create(texture.w, texture.h, texture.image.data());
    }
    else
    {
        throw std::runtime_error("Unsupported texture channel count!");
    }

    // Create texture from image
    sf::Texture tex;
    tex.loadFromImage(image);

    // Create sprite from texture
    sf::Sprite sprite;
    sprite.setTexture(tex);

    // Draw sprite on screen
    window.draw(sprite);
    window.display();
}