#pragma once

#include "Vec2.h"

namespace Dodo::Math {

    template <typename T = float>
    struct TVec3 {
        TVec3() : x((T)0), y((T)0), z((T)0) {}

        TVec3(T* data) : x(data[0]), y(data[1]), z(data[2]) {}

        TVec3(T f) : x(f), y(f), z(f) {}

        TVec3(const TVec2<T>& vec) : x(vec.x), y(vec.y), z((T)0) {}

        TVec3(const TVec2<T>& vec, T value) : x(vec.x), y(vec.y), z(value) {}

        TVec3(T first, T second, T third) : x(first), y(second), z(third) {}

        union {
            T x;
            T r;
        };
        union {
            T y;
            T g;
        };
        union {
            T z;
            T b;
        };

        inline void Zero()
        {
            this->x = (T)0;
            this->y = (T)0;
            this->z = (T)0;
        }

        inline void One()
        {
            this->x = (T)1;
            this->y = (T)1;
            this->z = (T)1;
        }

        inline void Forward()
        {
            this->x = (T)0;
            this->y = (T)0;
            this->z = (T)1;
        }

        inline void Backwards()
        {
            this->x = (T)0;
            this->y = (T)0;
            this->z = (T)-1;
        }

        inline void Left()
        {
            this->x = (T)-1;
            this->y = (T)0;
            this->z = (T)0;
        }

        inline void Right()
        {
            this->x = (T)1;
            this->y = (T)0;
            this->z = (T)0;
        }

        inline void Down()
        {
            this->x = (T)0;
            this->y = (T)-1;
            this->z = (T)0;
        }

        inline void Up()
        {
            this->x = (T)0;
            this->y = (T)1;
            this->z = (T)0;
        }

        inline TVec3 FlipX() const { return TVec3(-x, y, z); }

        inline TVec3 FlipY() const { return TVec3(x, -y, z); }

        inline TVec3 FlipZ() const { return TVec3(x, y, -z); }

        // Vector math //

        // Return the sum of all the components squared
        inline constexpr float SquareSum() const
        {
            return (float)(this->x * this->x + this->y * this->y + this->z * this->z);
        }

        // Return the magnitude of the vector
        inline constexpr float Magnitude() const { return std::sqrt(SquareSum()); }

        // Return normalized vector
        inline TVec3 Normalize() const
        {
            float mag = Magnitude();
            if (mag > 0) {
                return *this / mag;
            }
            return *this;
        }

        // Normalize this vector
        inline void NormalizeVector()
        {
            float mag = Magnitude();
            if (mag > 0) {
                *this /= mag;
            }
        }

        // Given a target vector, return a vector that is the linear interpolation between this vector and the target
        // vector by t
        inline TVec3 Lerp(const TVec3& target, float t) const { return (1.0f - t) * *this + t * target; }

        // Return a vector with a magnitude of limit
        inline constexpr TVec3 Limit(const T limit) const { return this->Normalize() * limit; }

        // Limit the vectors magnitude
        inline void LimitVector(const T limit) { *this = this->Normalize() * limit; }

        // Get distance between this vector and other vector
        inline float Distance(const TVec3& other) const
        {
            return std::sqrt((this->x - other.x) * (this->x - other.x) + (this->y - other.y) * (this->y - other.y) +
                             (this->z - other.z) * (this->z - other.z));
        }

        // Dot product
        inline float Dot(const TVec3& other) const { return this->x * other.x + this->y * other.y + this->z * other.z; }

        // Dot product
        inline float Dot(const T otherX, const T otherY, const T otherZ) const
        {
            return this->x * otherX + this->y * otherY + this->z * otherZ;
        }

        // Cross product
        inline TVec3 Cross(const TVec3& other) const
        {
            return TVec3(this->y * other.z - this->z * other.y, this->z * other.x - this->x * other.z,
                         this->x * other.y - this->y * other.x);
        }

        // Unary operations

        const TVec3& operator-() const { return TVec3(-this->x, -this->y, -this->z); }

        // Assignment //

        const TVec3& operator=(const int other)
        {
            this->x = other;
            this->y = other;
            this->z = other;
            return *this;
        }

        const TVec3& operator=(const double other)
        {
            this->x = other;
            this->y = other;
            this->z = other;
            return *this;
        }

        const TVec3& operator=(const float other)
        {
            this->x = other;
            this->y = other;
            this->z = other;
            return *this;
        }

        const TVec3& operator=(const TVec3& other)
        {
            this->x = other.x;
            this->y = other.y;
            this->z = other.z;
            return *this;
        }

        // Addition //

        constexpr TVec3& operator+=(const int other)
        {
            this->x += other;
            this->y += other;
            this->z += other;
            return *this;
        }

        friend TVec3 operator+(TVec3 left, const int right) { return left += right; }

        constexpr TVec3& operator+=(const double other)
        {
            this->x += other;
            this->y += other;
            this->z += other;
            return *this;
        }

        friend TVec3 operator+(TVec3 left, const double right) { return left += right; }

        constexpr TVec3& operator+=(const float other)
        {
            this->x += other;
            this->y += other;
            this->z += other;
            return *this;
        }

        friend TVec3 operator+(TVec3 left, const float right) { return left += right; }

        constexpr TVec3& operator+=(const TVec3& other)
        {
            this->x += other.x;
            this->y += other.y;
            this->z += other.z;
            return *this;
        }

        friend TVec3 operator+(TVec3 left, const TVec3& right) { return left += right; }

        // Subtraction //

        constexpr TVec3& operator-=(const int other)
        {
            this->x -= other;
            this->y -= other;
            this->z -= other;
            return *this;
        }

        friend TVec3 operator-(TVec3 left, const int right) { return left -= right; }

        constexpr TVec3& operator-=(const double other)
        {
            this->x -= other;
            this->y -= other;
            this->z -= other;
            return *this;
        }

        friend TVec3 operator-(TVec3 left, const double right) { return left -= right; }

        constexpr TVec3& operator-=(const float other)
        {
            this->x -= other;
            this->y -= other;
            this->z -= other;
            return *this;
        }

        friend TVec3 operator-(TVec3 left, const float right) { return left -= right; }

        constexpr TVec3& operator-=(const TVec3& other)
        {
            this->x -= other.x;
            this->y -= other.y;
            this->z -= other.z;
            return *this;
        }

        friend TVec3 operator-(TVec3 left, const TVec3& right) { return left -= right; }

        // Multiplication //

        constexpr TVec3& operator*=(const int other)
        {
            this->x *= other;
            this->y *= other;
            this->z *= other;
            return *this;
        }

        friend TVec3 operator*(TVec3 left, const int right) { return left *= right; }

        constexpr TVec3& operator*(const double other)
        {
            this->x *= other;
            this->y *= other;
            this->z *= other;
            return *this;
        }

        friend TVec3 operator*=(TVec3 left, const double right) { return left * right; }

        const TVec3& operator*=(const float other)
        {
            this->x *= other;
            this->y *= other;
            this->z *= other;
            return *this;
        }

        friend TVec3 operator*(TVec3 left, const float right) { return left *= right; }

        constexpr TVec3& operator*=(const TVec3& other)
        {
            this->x *= other.x;
            this->y *= other.y;
            this->z *= other.z;
            return *this;
        }

        friend TVec3 operator*(TVec3 left, const TVec3& right) { return left *= right; }

        // Division //

        constexpr TVec3& operator/=(const int other)
        {
            const float temp = 1.0f / other;
            this->x *= temp;
            this->y *= temp;
            this->z *= temp;
            return *this;
        }

        friend TVec3 operator/(TVec3 left, const int right) { return left /= right; }

        constexpr TVec3& operator/=(const double other)
        {
            const double temp = 1.0 / other;
            this->x *= temp;
            this->y *= temp;
            this->z *= temp;
            return *this;
        }

        friend TVec3 operator/(TVec3 left, const double right) { return left /= right; }

        constexpr TVec3& operator/=(const float other)
        {
            const float temp = 1.0f / other;
            this->x *= temp;
            this->y *= temp;
            this->z *= temp;
            return *this;
        }

        friend TVec3 operator/(TVec3 left, const float right) { return left /= right; }

        constexpr TVec3& operator/=(const TVec3& other)
        {
            this->x /= other.x;
            this->y /= other.y;
            this->z /= other.z;
            return *this;
        }

        friend TVec3 operator/(TVec3 left, const TVec3& right) { return left /= right; }

        // Modulus //

        constexpr TVec3& operator%=(const int other)
        {
            this->x %= other;
            this->y %= other;
            this->z %= other;
            return *this;
        }

        friend TVec3 operator%(TVec3 left, const int right) { return left %= right; }

        constexpr TVec3& operator%=(const double other)
        {
            this->x %= other;
            this->y %= other;
            this->z %= other;
            return *this;
        }

        friend TVec3 operator%(TVec3 left, const double right) { return left %= right; }

        constexpr TVec3& operator%=(const float other)
        {
            this->x %= other;
            this->y %= other;
            this->z %= other;
            return *this;
        }

        friend TVec3 operator%(TVec3 left, const float right) { return left %= right; }

        constexpr TVec3& operator%=(const TVec3& other)
        {
            this->x %= other.x;
            this->y %= other.y;
            this->z %= other.z;
            return *this;
        }

        friend TVec3 operator%(TVec3 left, const TVec3& right) { return left %= right; }

        // Test //

        const bool operator==(const TVec3& other) const
        {
            return (this->x == other.x && this->y == other.y && this->z == other->z);
        }

        const bool operator!=(const TVec3& other) const
        {
            return !(this->x == other.x && this->y == other.y && this->z == other.z);
        }

        const bool operator<(const TVec3& other) const
        {
            return (this->x < other.x && this.y < other.y && this->z < other.z);
        }
        const bool operator>(const TVec3& other) const
        {
            return (this->x > other.x && this.y > other.y && this->z > other.z);
        }

        const bool operator<=(const TVec3& other) const
        {
            return (this->x <= other.x && this.y <= other.y && this.z <= other.z);
        }
        const bool operator>=(const TVec3& other) const
        {
            return (this->x >= other.x && this.y >= other.y && this.z >= other.z);
        }
    };
    using Vec3 = TVec3<float>;

    template <typename T>
    inline TVec3<T> Normalize(const TVec3<T>& vec)
    {
        float mag = vec.Magnitude();
        if (mag > 0) return vec / mag;
        return vec;
    }

    template <typename T>
    inline TVec3<T> NegativeNormalize(const TVec3<T>& vec)
    {
        float mag = vec.Magnitude();
        if (mag > 0) return vec * -(1.0f / mag);
        return vec;
    }

    template <typename T>
    inline TVec3<T> Cross(const TVec3<T>& a, const TVec3<T>& b)
    {
        return TVec3<T>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
    }

    template <typename T>
    inline TVec3<T> Lerp(const TVec3<T>& a, const TVec3<T>& b, float t)
    {
        return (1.0f - t) * a + t * b;
    }

    template <typename T>
    inline TVec3<T> ExpDecay(const TVec3<T>& a, const TVec3<T>& b, float decay, float dt)
    {
        return b + (a - b) * exp(-decay * dt);
    }

    template <typename T>
    inline float Sum(const TVec3<T>& vec)
    {
        return vec.x + vec.y + vec.z;
    }

    template <typename T>
    inline float Dot(const TVec3<T>& a, const TVec3<T>& b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    template <typename T>
    inline float Distance(const TVec3<T>& a, const TVec3<T>& b)
    {
        return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z));
    }

    inline Vec3 ToRadians(const Vec3& degrees)
    {
        return degrees * MATH_PI / 180.0f;
    }

    inline Vec3 ToDegrees(const Vec3& radians)
    {
        return radians * 180.0f / MATH_PI;
    }

} // namespace Dodo::Math
