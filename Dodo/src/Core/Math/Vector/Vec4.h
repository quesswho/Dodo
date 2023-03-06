#pragma once

#include "Vec3.h"

namespace Dodo {
	namespace Math {

		template<typename T = float>
		struct TVec4
		{
			TVec4()
				: x((T)0), y((T)0), z((T)0), w((T)0)
			{}

			TVec4(T f)
				: x(f), y(f), z(f), w(f)
			{}

			TVec4(const TVec2<T>& vec)
				: x(vec.x), y(vec.y), z((T)0), w((T)0)
			{}

			TVec4(const TVec2<T>& vec, T value, T second)
				: x(vec.x), y(vec.y), z(value), w(second)
			{}

			TVec4(const TVec3<T>& vec)
				: x(vec.x), y(vec.y), z((T)vec.z), w((T)0)
			{}

			TVec4(const TVec3<T>& vec, T value)
				: x(vec.x), y(vec.y), z(vec.z), w(value)
			{}

			TVec4(T first, T second, T third, T forth)
				: x(first), y(second), z(third), w(forth)
			{}

			union { T x; T r; };
			union { T y; T g; };
			union { T z; T b; };
			union { T w; T a; };

			inline void Zero()
			{
				this->x = (T)0;
				this->y = (T)0;
				this->z = (T)0;
				this->w = (T)0;
			}

			inline void One()
			{
				this->x = (T)1;
				this->y = (T)1;
				this->z = (T)1;
				this->w = (T)1;
			}

			inline void Forward()
			{
				this->x = (T)0;
				this->y = (T)0;
				this->z = (T)1;
				this->w = (T)0;
			}

			inline void Backwards()
			{
				this->x = (T)0;
				this->y = (T)0;
				this->z = (T)1;
				this->w = (T)0;
			}

			inline void Left()
			{
				this->x = (T)-1;
				this->y = (T)0;
				this->z = (T)0;
				this->w = (T)0;
			}

			inline void Right()
			{
				this->x = (T)1;
				this->y = (T)0;
				this->z = (T)0;
				this->w = (T)0;
			}

			inline void Down()
			{
				this->x = (T)0;
				this->y = (T)-1;
				this->z = (T)0;
				this->w = (T)0;
			}

			inline void Up()
			{
				this->x = (T)0;
				this->y = (T)1;
				this->z = (T)0;
				this->w = (T)0;
			}

			inline void Ana()
			{
				this->x = (T)0;
				this->y = (T)0;
				this->z = (T)0;
				this->w = (T)1;
			}

			inline void Kata()
			{
				this->x = (T)0;
				this->y = (T)0;
				this->z = (T)0;
				this->w = (T)-1;
			}

			// Vector math //

			// Return the sum of all the components squared
			constexpr float SquareSum() const
			{
				return (float)(this->x * this->x + this->y * this->y + this->z * this->z + this->w * this->w);
			}

			// Return the magnitude of the vector
			inline constexpr float Magnitude() const
			{
				return sqrt(SquareSum()); 
			}

			// Return normalized vector
			inline TVec4 Normalize() const
			{
				return *this * fast_isqrt(SquareSum());
			}

			// Normalize this vector
			inline void NormalizeVector()
			{
				*this *= fast_isqrt(SquareSum());
			}

			// Return a vector with a magnitude of limit
			inline constexpr TVec4 Limit(const int limit) const
			{
				return this->Normalize() * limit;
			}

			// Limit the vectors magnitude
			inline void LimitVector(const int limit)
			{
				*this = this->Normalize() * limit;
			}

			// Get distance between this vector and other vector
			inline float Distance(const TVec4& other) const
			{
				return sqrt((this->x - other->x) * (this->x - other->x) +
					(this->y - other->y) * (this->y - other->y) +
					(this->z - other->z) * (this->z - other->z) +
					(this->w - other->w) * (this->w - other->w));
			}

			// Dot product 
			inline float Dot(const TVec4& other) const
			{
				return this->x * other.x + this->y * other.y + this->z * other.z + this->w * other.w;
			}

			inline float Dot(const T otherX, const T otherY, const T otherZ, const T otherW) const
			{
				return this->x * otherX + this->y * otherY + this->z * otherZ + this->w * otherW;
			}

			// Unary operations

			const TVec4& operator-() const
			{
				return TVec4(-this->x, -this->y, -this->z, -this->z);
			}

			// Assignment //

			const TVec4& operator=(const int other)
			{
				this->x = other;
				this->y = other;
				this->z = other;
				this->w = other;
				return *this;
			}

			const TVec4& operator=(const double other)
			{
				this->x = other;
				this->y = other;
				this->z = other;
				this->w = other;
				return *this;
			}

			const TVec4& operator=(const float other)
			{
				this->x = other;
				this->y = other;
				this->z = other;
				this->w = other;
				return *this;
			}

			const TVec4& operator=(const TVec4& other)
			{
				this->x = other.x;
				this->y = other.y;
				this->z = other.z;
				this->w = other.w;
				return *this;
			}

			// Addition //


			constexpr TVec4& operator+=(const int other)
			{
				this->x += other;
				this->y += other;
				this->z += other;
				this->w += other;
				return *this;
			}

			friend TVec4 operator+(TVec4 left, const int right) { return left += right; }


			constexpr TVec4& operator+=(const double other)
			{
				this->x += other;
				this->y += other;
				this->z += other;
				this->w += other;
				return *this;
			}

			friend TVec4 operator+(TVec4 left, const double right) { return left += right; }


			constexpr TVec4& operator+=(const float other)
			{
				this->x += other;
				this->y += other;
				this->z += other;
				this->w += other;
				return *this;
			}

			friend TVec4 operator+(TVec4 left, const float right) { return left += right; }


			constexpr TVec4& operator+=(const TVec4& other)
			{
				this->x += other.x;
				this->y += other.y;
				this->z += other.z;
				this->w += other.w;
				return *this;
			}

			friend TVec4 operator+(TVec4 left, const TVec4& right) { return left += right; }

			// Subtraction //

			constexpr TVec4& operator-=(const int other)
			{
				this->x -= other;
				this->y -= other;
				this->z -= other;
				this->w -= other;
				return *this;
			}

			friend TVec4 operator-(TVec4 left, const int right) { return left -= right; }


			constexpr TVec4& operator-=(const double other)
			{
				this->x -= other;
				this->y -= other;
				this->z -= other;
				this->w -= other;
				return *this;
			}

			friend TVec4 operator-(TVec4 left, const double right) { return left -= right; }


			constexpr TVec4& operator-=(const float other)
			{
				this->x -= other;
				this->y -= other;
				this->z -= other;
				this->w -= other;
				return *this;
			}

			friend TVec4 operator-(TVec4 left, const float right) { return left -= right; }


			constexpr TVec4& operator-=(const TVec4& other)
			{
				this->x -= other.x;
				this->y -= other.y;
				this->z -= other.z;
				this->w -= other.w;
				return *this;
			}

			friend TVec4 operator-(TVec4 left, const TVec4& right) { return left -= right; }

			// Multiplication //

			constexpr TVec4& operator*=(const int other)
			{
				this->x *= other;
				this->y *= other;
				this->z *= other;
				this->w *= other;
				return *this;
			}

			friend TVec4 operator*(TVec4 left, const int right) { return left *= right; }


			constexpr TVec4& operator*=(const double other)
			{
				this->x *= other;
				this->y *= other;
				this->z *= other;
				this->w *= other;
				return *this;
			}

			friend TVec4 operator*(TVec4 left, const double right) { return left *= right; }


			const TVec4& operator*=(const float other)
			{
				this->x *= other;
				this->y *= other;
				this->z *= other;
				this->w *= other;
				return *this;
			}

			friend TVec4 operator*(TVec4 left, const float right) { return left *= right; }


			constexpr TVec4& operator*=(const TVec4& other)
			{
				this->x *= other.x;
				this->y *= other.y;
				this->z *= other.z;
				this->w *= other.w;
				return *this;
			}

			friend TVec4 operator*(TVec4 left, const TVec4& right) { return left *= right; }


			// Division //

			constexpr TVec4& operator/=(const int other)
			{
				const float temp = 1.0f / other;
				this->x *= temp;
				this->y *= temp;
				this->z *= temp;
				this->w *= temp;
				return *this;
			}

			friend TVec4 operator/(TVec4 left, const int right) { return left /= right; }


			constexpr TVec4& operator/=(const double other)
			{
				const float temp = 1.0f / other;
				this->x *= temp;
				this->y *= temp;
				this->z *= temp;
				this->w *= temp;
				return *this;
			}

			friend TVec4 operator/(TVec4 left, const double right) { return left /= right; }


			constexpr TVec4& operator/=(const float other)
			{
				const float temp = 1.0f / other;
				this->x *= temp;
				this->y *= temp;
				this->z *= temp;
				this->w *= temp;
				return *this;
			}

			friend TVec4 operator/(TVec4 left, const float right) { return left /= right; }


			constexpr TVec4& operator/=(const TVec4& other)
			{
				this->x /= other.x;
				this->y /= other.y;
				this->z /= other.z;
				this->w /= other.w;
				return *this;
			}

			friend TVec4 operator/(TVec4 left, const TVec4& right) { return left /= right; }

			// Modulus //

			constexpr TVec4& operator%=(const int other)
			{
				this->x %= other;
				this->y %= other;
				this->z %= other;
				this->w %= other;
				return *this;
			}

			friend TVec4 operator%(TVec4 left, const int right) { return left %= right; }


			constexpr TVec4& operator%=(const double other)
			{
				this->x %= other;
				this->y %= other;
				this->z %= other;
				this->w %= other;
				return *this;
			}

			friend TVec4 operator%(TVec4 left, const double right) { return left %= right; }


			constexpr TVec4& operator%=(const float other)
			{
				this->x %= other;
				this->y %= other;
				this->z %= other;
				this->w %= other;
				return *this;
			}

			friend TVec4 operator%(TVec4 left, const float right) { return left %= right; }


			constexpr TVec4& operator%=(const TVec4& other)
			{
				this->x %= other.x;
				this->y %= other.y;
				this->z %= other.z;
				this->w %= other.w;
				return *this;
			}

			friend TVec4 operator%(TVec4 left, const TVec4& right) { return left %= right; }

			// Test //

			const bool operator==(const TVec4& other) const
			{
				return (this->x == other.x && this->y == other.y && this->z == other->z && this->w == other->w);
			}

			const bool operator!=(const TVec4& other) const
			{
				return !(this->x == other.x && this->y == other.y && this->z == other.z && this->w == other->w);
			}

			const bool operator<(const TVec4& other) const
			{
				return (this->x < other.x && this->y < other.y && this->z < other.z && this->w < other.w);
			}
			const bool operator>(const TVec4& other) const
			{
				return (this->x > other.x && this->y > other.y && this->z > other.z && this->w > other.w);
			}

			const bool operator<=(const TVec4& other) const
			{
				return (this->x <= other.x && this->y <= other.y && this->z <= other.z && this->w <= other.w);
			}
			const bool operator>=(const TVec4& other) const
			{
				return (this->x >= other.x && this->y >= other.y && this->z >= other.z && this->w >= other.w);
			}
		};
		using Vec4 = TVec4<float>;
	}
}
