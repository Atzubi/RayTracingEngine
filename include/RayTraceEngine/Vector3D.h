#pragma once

#include <cmath>
#include <stdexcept>
#include <string>

/**
 * Contains x, y and z coordinates representing a vector in 3 dimensions.
 */
struct Vector3D
{
    float x;
    float y;
    float z;

    float operator[](int idx) const
    {
        switch (idx)
        {
            case 0:
                return x;
            case 1:
                return y;
            case 2:
                return z;
            default:
                const auto message = "Index " + std::to_string(idx) + " is out of range for Vector3D";
                throw std::out_of_range(message);
        }
    }

    float& operator[](int idx)
    {
        switch (idx)
        {
            case 0:
                return x;
            case 1:
                return y;
            case 2:
                return z;
            default:
                const auto message = "Index " + std::to_string(idx) + " is out of range for Vector3D";
                throw std::out_of_range(message);
        }
    }

    Vector3D operator+(const Vector3D& other) const { return {x + other.x, y + other.y, z + other.z}; }

    void operator+=(const Vector3D& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
    }

    Vector3D operator+(const float scalar) const { return {x + scalar, y + scalar, z + scalar}; }

    void operator+=(const float scalar)
    {
        x += scalar;
        y += scalar;
        z += scalar;
    }

    Vector3D operator-(const Vector3D& other) const { return {x - other.x, y - other.y, z - other.z}; }

    void operator-=(const Vector3D& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
    }

    Vector3D operator-(const float scalar) const { return {x - scalar, y - scalar, z - scalar}; }

    void operator-=(const float scalar)
    {
        x -= scalar;
        y -= scalar;
        z -= scalar;
    }

    Vector3D operator*(const Vector3D& other) const { return {x * other.x, y * other.y, z * other.z}; }

    void operator*=(const Vector3D& other)
    {
        x *= other.x;
        y *= other.y;
        z *= other.z;
    }

    Vector3D operator*(const float scalar) const { return {x * scalar, y * scalar, z * scalar}; }

    void operator*=(const float scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
    }

    Vector3D operator/(const Vector3D& other) const
    {
        if (other.x == 0 || other.y == 0 || other.z == 0)
            throw std::invalid_argument("Division by 0!");
        return {x / other.x, y / other.y, z / other.z};
    }

    void operator/=(const Vector3D& other)
    {
        if (other.x == 0 || other.y == 0 || other.z == 0)
            throw std::invalid_argument("Division by 0!");
        x /= other.x;
        y /= other.y;
        z /= other.z;
    }

    Vector3D operator/(const float scalar) const
    {
        if (scalar == 0)
            throw std::invalid_argument("Division by 0!");
        return {x / scalar, y / scalar, z / scalar};
    }

    void operator/=(const float scalar)
    {
        if (scalar == 0)
            throw std::invalid_argument("Division by 0!");
        x /= scalar;
        y /= scalar;
        z /= scalar;
    }

    [[nodiscard]] float GetLength() const { return std::sqrt(x * x + y * y + z * z); }

    void Normalize()
    {
        const auto length = GetLength();
        if (length != 0)
            *this /= length;
    }

    [[nodiscard]] Vector3D GetInverse() const { return {1.f / x, 1.f / y, 1.f / z}; }

    [[nodiscard]] Vector3D Cross(const Vector3D& other) const
    {
        return {y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x};
    }

    [[nodiscard]] float Sum() const { return x + y + z; }

    float Dot(const Vector3D& other) const { return x * other.x + y * other.y + z * other.z; }
};
