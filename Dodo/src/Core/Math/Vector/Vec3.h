#pragma once
#include "Vec2.h"

namespace Dodo {
	namespace Math {

		template<typename T = float>
		struct TVec3
		{
			TVec3()
				: x((T)0), y((T)0), z((T)0)
			{}

			TVec3(T f)
				: x(f), y(f), z(f)
			{}

			TVec3(const TVec2<T>& vec)
				: x(vec.x), y(vec.y), z((T)0)
			{}

			TVec3(const TVec2<T>& vec, T value)
				: x(vec.x), y(vec.y), z(value)
			{}

			TVec3(T first, T second, T third)
				: x(first), y(second), z(third)
			{}

			union { T x; T r; };
			union { T y; T g; };
			union { T z; T b; };

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

			inline TVec3 FlipX() const
			{
				return TVec3(-x, y, z);
			}

			inline TVec3 FlipY() const
			{
				return TVec3(x, -y, z);
			}

			inline TVec3 FlipZ() const
			{
				return TVec3(x, y, -z);
			}

			// Vector math //

			// Return the magnitude of the vector
			inline constexpr float Magnitude() const
			{
				return sqrt((float)(this->x * this->x + this->y * this->y + this->z * this->z));
			}

			// Return normalized vector
			inline TVec3 Normalize() const
			{
				float mag = this->Magnitude();
				if (mag > 0)
					return *this * (1.0f / mag);

				return *this; // Can't normalize a zero vector
			}

			// Normalize this vector
			inline void NormalizeVector()
			{
				float mag = this->Magnitude();
				if (mag > 0)
					*this *= (1.0f / mag);
			}

			// Limit magnitude with int
			inline constexpr TVec3 Limit(const int limit) const
			{
				return this->Normalize() * limit;
			}

			// Limit this magnitude with int
			inline void LimitVector(const int limit)
			{
				*this = this->Normalize() * limit;
			}

			// Limit magnitude with double
			inline TVec3 Limit(const double limit) const
			{
				return this->Normalize() * limit;
			}

			// Limit this magnitude with double
			inline void LimitVector(const double limit)
			{
				*this = this->Normalize() * limit;
			}

			// Limit magnitude with float
			inline TVec3 Limit(const float limit) const
			{
				return this->Normalize() * limit;
			}

			// Limit this magnitude with float
			inline void LimitVector(const float limit)
			{
				*this = this->Normalize() * limit;
			}

			// Get distance between this vector and other vector
			inline float Distance(const TVec3& other) const
			{
				return sqrt((this->x - other->x) * (this->x - other->x) +
					(this->y - other->y) * (this->y - other->y) + 
					(this->z - other->z) * (this->z - other->z))
			}

			// Dot product 
			inline float Dot(const TVec3& other) const
			{
				return this->x * other.x + this->y * other.y + this->z * other.z;
			}

			// Dot product 
			inline float Dot(const T otherX, const T otherY, const T otherZ) const
			{
				return this->x * otherX + this->y * otherY + this->z * otherZ;
			}


			// Cross product
			inline TVec3 Cross(const TVec3& other) const
			{
				return TVec3(this->y * other.z - this->z * other.y, this->z * other.x - this->x * other.z, this->x * other.y - this->y * other.x);
			}

			// Unary operations

			const TVec3& operator-() const
			{
				return TVec3(-this->x, -this->y, -this->z);
			}

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

			constexpr TVec3& operator/=(int other)
			{
				other = 1 / other;
				this->x *= other;
				this->y *= other;
				this->z *= other;
				return *this;
			}

			friend TVec3 operator/(TVec3 left, const int right) { return left /= right; }


			constexpr TVec3& operator/=(double other)
			{
				other = 1 / other;
				this->x *= other;
				this->y *= other;
				this->z *= other;
				return *this;
			}

			friend TVec3 operator/(TVec3 left, const double right) { return left /= right; }


			constexpr TVec3& operator/=(float other)
			{
				other = 1 / other;
				this->x *= other;
				this->y *= other;
				this->z *= other;
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
				return (this->x > other.x&& this.y > other.y && this->z > other.z);
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
		typedef TVec3<float> Vec3;
	}
}
