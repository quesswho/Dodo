#pragma once

namespace Dodo {
	namespace Math {

		template<typename T = float>
		struct TVec2
		{
			TVec2()
				: x((T)0), y((T)0)
			{}

			TVec2(T f)
				: x(f), y(f)
			{}

			TVec2(T first, T second)
				: x(first), y(second)
			{}

			union { T x; T r; T u; };
			union { T y; T g; T v; };

			inline void Zero()
			{
				this->x = (T)0;
				this->y = (T)0;
			}

			inline void One()
			{
				this->x = (T)1;
				this->y = (T)1;
			}

			inline void Left()
			{
				this->x = (T)-1;
				this->y = (T)0;
			}

			inline void Right()
			{
				this->x = (T)1;
				this->y = (T)0;
			}

			inline void Down()
			{
				this->x = (T)0;
				this->y = (T)-1;
			}

			inline void Up()
			{
				this->x = (T)0;
				this->y = (T)1;
			}

			// Vector math //

			// Return the sum of all the components squared
			inline constexpr float SquareSum() const
			{
				return (float)(this->x * this->x + this->y * this->y);
			}

			// Return the magnitude of the vector
			inline constexpr float Magnitude() const
			{
				return sqrt(SquareSum());
			}

			// Return normalized vector
			inline TVec2 Normalize() const
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

			// Return a vector with a magnitude of limit
			template<typename T>
			inline TVec2 Limit(const T limit) const
			{
				return this->Normalize() * limit;
			}

			// Limit the vectors magnitude
			template<typename T>
			inline void LimitVector(const T limit)
			{
				*this = this->Normalize() * limit;
			}

			// Get distance between this vector and other vector
			inline float Distance(const TVec2& other) const
			{
				return sqrt((this->x - other->x) * (this->x - other->x) + 
					(this->y - other->y) * (this->y - other->y))
			}

			// Dot product 
			inline float Dot(const TVec2& other) const
			{
				return this->x * other.x + this->y * other.y;
			}

			// Dot product 
			inline float Dot(const T otherX, const T otherY) const
			{
				return this->x * otherX + this->y * otherY;
			}

			// Get the angle of the vector in degrees
			inline float Degrees() const
			{
				return ToDegrees(Radians());
			}

			// Get the angle between a vector in degrees
			inline float Degrees(const TVec2& other) const
			{
				return ToDegrees(Radians(other));
			}

			// Get the angle between a vector in radians
			inline float Radians(const TVec2& other) const
			{
				return this->Normalize().Dot(other.Normalize());
			}

			// Get the angle of the vector in radians
			inline float Radians() const
			{
				return atan2((float)this->y, (float)this->x);
			}

			// Unary operations

			const TVec2& operator-() const
			{
				return TVec2(-this->x, -this->y);
			}

			// Assignment //

			const TVec2& operator=(const int other)
			{
				this->x = other;
				this->y = other;
				return *this;
			}

			const TVec2& operator=(const double other)
			{
				this->x = other;
				this->y = other;
				return *this;
			}

			const TVec2& operator=(const float other)
			{
				this->x = other;
				this->y = other;
				return *this;
			}

			const TVec2& operator=(const TVec2& other)
			{
				this->x = other.x;
				this->y = other.y;
				return *this;
			}

			// Addition //


			constexpr TVec2& operator+=(const int other)
			{
				this->x += other;
				this->y += other;
				return *this;
			}

			friend TVec2 operator+(TVec2 left, const int right) { return left += right; }


			constexpr TVec2& operator+=(const double other)
			{
				this->x += other;
				this->y += other;
				return *this;
			}

			friend TVec2 operator+(TVec2 left, const double right) { return left += right; }


			constexpr TVec2& operator+=(const float other)
			{
				this->x += other;
				this->y += other;
				return *this;
			}

			friend TVec2 operator+(TVec2 left, const float right) { return left += right; }


			constexpr TVec2& operator+=(const TVec2& other)
			{
				this->x += other.x;
				this->y += other.y;
				return *this;
			}

			friend TVec2 operator+(TVec2 left, const TVec2& right) { return left += right; }

			// Subtraction //

			constexpr TVec2& operator-=(const int other)
			{
				this->x -= other;
				this->y -= other;
				return *this;
			}

			friend TVec2 operator-(TVec2 left, const int right) { return left -= right; }


			constexpr TVec2& operator-=(const double other)
			{
				this->x -= other;
				this->y -= other;
				return *this;
			}

			friend TVec2 operator-(TVec2 left, const double right) { return left -= right; }


			constexpr TVec2& operator-=(const float other)
			{
				this->x -= other;
				this->y -= other;
				return *this;
			}

			friend TVec2 operator-(TVec2 left, const float right) { return left -= right; }


			constexpr TVec2& operator-=(const TVec2& other)
			{
				this->x -= other.x;
				this->y -= other.y;
				return *this;
			}

			friend TVec2 operator-(TVec2 left, const TVec2& right) { return left -= right; }

			// Multiplication //

			constexpr TVec2& operator*=(const int other)
			{
				this->x *= other;
				this->y *= other;
				return *this;
			}

			friend TVec2 operator*(TVec2 left, const int right) { return left *= right; }


			constexpr TVec2& operator*=(const double other)
			{
				this->x *= other;
				this->y *= other;
				return *this;
			}

			friend TVec2 operator*(TVec2 left, const double right) { return left *= right; }


			const TVec2& operator*=(const float other)
			{
				this->x *= other;
				this->y *= other;
				return *this;
			}

			friend TVec2 operator*(TVec2 left, const float right) { return left *= right; }


			constexpr TVec2& operator*=(const TVec2& other)
			{
				this->x *= other.x;
				this->y *= other.y;
				return *this;
			}

			friend TVec2 operator*(TVec2 left, const TVec2& right) { return left *= right; }

			// Division //

			constexpr TVec2& operator/=(const int other)
			{
				const float temp = 1.0f / other;
				this->x *= temp;
				this->y *= temp;
				return *this;
			}

			friend TVec2 operator/(TVec2 left, const int right) { return left /= right; }


			constexpr TVec2& operator/=(const double other)
			{
				const double temp = 1.0 / other;
				this->x *= temp;
				this->y *= temp;
				return *this;
			}

			friend TVec2 operator/(TVec2 left, double right) { return left /= right; }


			constexpr TVec2& operator/=(const float other)
			{
				const float temp = 1.0f / other;
				this->x *= temp;
				this->y *= temp;
				return *this;
			}

			friend TVec2 operator/(TVec2 left, const float right) { return left /= right; }


			constexpr TVec2& operator/=(const TVec2& other)
			{
				this->x /= other.x;
				this->y /= other.y;
				return *this;
			}

			friend TVec2 operator/(TVec2 left, const TVec2& right) { return left /= right; }

			// Modulus //

			constexpr TVec2& operator%=(const int other)
			{
				this->x %= other;
				this->y %= other;
				return *this;
			}

			friend TVec2 operator%(TVec2 left, const int right) { return left %= right; } 


			constexpr TVec2& operator%=(const double other)
			{
				this->x %= other;
				this->y %= other;
				return *this;
			}

			friend TVec2 operator%(TVec2 left, const double right) { return left %= right; }


			constexpr TVec2& operator%=(const float other)
			{
				this->x %= other;
				this->y %= other;
				return *this;
			}

			friend TVec2 operator%(TVec2 left, const float right) { return left %= right; }


			constexpr TVec2& operator%=(const TVec2& other)
			{
				this->x %= other.x;
				this->y %= other.y;
				return *this;
			}

			friend TVec2 operator%(TVec2 left, const TVec2& right) { return left %= right; }

			// Test //

			const bool operator==(const TVec2& other) const
			{
				return (this->x == other.x && this->y == other.y);
			}

			const bool operator!=(const TVec2& other) const
			{
				return !(this->x == other.x && this->y == other.y);
			}

			const bool operator<(const TVec2& other) const
			{
				return (this->x < other.x && this.y < other.y);
			}
			const bool operator>(const TVec2& other) const
			{
				return (this->x > other.x && this.y > other.y);
			}

			const bool operator<=(const TVec2& other) const
			{
				return (this->x <= other.x && this.y <= other.y);
			}
			const bool operator>=(const TVec2& other) const
			{
				return (this->x >= other.x&& this.y >= other.y);
			}
		};
		using Vec2 = TVec2<float>;
	}
}
