#pragma once

#include "Core/Math/Vector/Vec2.h"
#include <Core/Common.h>

namespace Dodo::Math {

#if defined(DD_MATH_COLUMN_MAJOR) && defined(DD_COORDINATE_RIGHT_HANDED)

    // Column Major 2x2 matrix
    template <typename T = float>
    struct Mat2x2 {

        union {
            T m_Elements[4];
            Math::TVec2<T> m_Columns[2];
        };

        Mat2x2() : m_Elements{0} {}
        Mat2x2(float diagonal) { Identity(diagonal); }

        Mat2x2(const Math::TVec2<T>& first, const Math::TVec2<T>& second)
        {
            m_Elements[GetIndex(0, 0)] = first.x;
            m_Elements[GetIndex(0, 1)] = first.y;

            m_Elements[GetIndex(1, 0)] = second.x;
            m_Elements[GetIndex(1, 1)] = second.y;
        }

        Mat2x2(T first, T second, T third, T forth)
        {
            m_Elements[GetIndex(0, 0)] = first;
            m_Elements[GetIndex(0, 1)] = second;

            m_Elements[GetIndex(1, 0)] = third;
            m_Elements[GetIndex(1, 1)] = forth;
        }

        inline void Identity()
        {
            memset(m_Elements, 0, 4 * 4);

            m_Elements[GetIndex(0, 0)] = (T)1;
            m_Elements[GetIndex(1, 1)] = (T)1;
        }

        inline void Identity(float diagonal)
        {
            memset(m_Elements, 0, 4 * 4);

            m_Elements[GetIndex(0, 0)] = (T)diagonal;
            m_Elements[GetIndex(1, 1)] = (T)diagonal;
        }

        // Addition

        const Mat2x2& operator+=(const float scalar)
        {
            for (int i = 0; i < 4; i++) {
                this->m_Elements[i] += scalar;
            }
            return *this;
        }

        friend Mat2x2 operator+(Mat2x2 left, const float right) { return left += right; }

        const Mat2x2& operator+=(const Mat2x2& other)
        {
            for (int i = 0; i < 4; i++) {
                this->m_Elements[i] += other.m_Elements[i];
            }
            return *this;
        }

        friend Mat2x2 operator+(Mat2x2 left, const Mat2x2& right) { return left += right; }

        // Subtraction

        const Mat2x2& operator-=(const float scalar)
        {
            for (int i = 0; i < 4; i++) {
                this->m_Elements[i] -= scalar;
            }
            return *this;
        }

        friend Mat2x2 operator-(Mat2x2 left, const float right) { return left -= right; }

        const Mat2x2& operator-=(const Mat2x2& other)
        {
            for (int i = 0; i < 4; i++) {
                this->m_Elements[i] -= other.m_Elements[i];
            }
            return *this;
        }

        friend Mat2x2 operator-(Mat2x2 left, const Mat2x2& right) { return left -= right; }

        // Multiplication

        const Mat2x2& operator*=(const float scalar)
        {
            for (int i = 0; i < 4; i++)
                m_Elements[i] *= scalar;

            return *this;
        }

        friend Mat2x2 operator*(Mat2x2 left, const float right) { return left *= right; }

        const Mat2x2& operator*=(const Mat2x2& other)
        {
            const Math::TVec2<T> row0 = Math::TVec2<T>(m_Elements[GetIndex(0, 0)], m_Elements[GetIndex(1, 0)]);
            const Math::TVec2<T> row1 = Math::TVec2<T>(m_Elements[GetIndex(0, 1)], m_Elements[GetIndex(1, 1)]);

            m_Elements[0] = other.m_Columns[0].Dot(row0);
            m_Elements[1] = other.m_Columns[0].Dot(row1);

            m_Elements[3] = other.m_Columns[1].Dot(row0);
            m_Elements[4] = other.m_Columns[1].Dot(row1);

            return *this;
        }

        friend Mat2x2 operator*(Mat2x2 left, const Mat2x2& right) { return left *= right; }

        // Assignment

        const Mat2x2& operator=(const Mat2x2& other)
        {
            m_Elements[GetIndex(0, 0)] = other.m_Elements[GetIndex(0, 0)];
            m_Elements[GetIndex(0, 1)] = other.m_Elements[GetIndex(0, 1)];

            m_Elements[GetIndex(1, 0)] = other.m_Elements[GetIndex(1, 0)];
            m_Elements[GetIndex(1, 1)] = other.m_Elements[GetIndex(1, 1)];
            return *this;
        }

        // Test

        const bool operator==(const Mat2x2& other)
        {
            return (m_Elements[GetIndex(0, 0)] == other.m_Elements[GetIndex(0, 0)] &&
                    m_Elements[GetIndex(0, 1)] == other.m_Elements[GetIndex(0, 1)] &&

                    m_Elements[GetIndex(1, 0)] == other.m_Elements[GetIndex(1, 0)] &&
                    m_Elements[GetIndex(1, 1)] == other.m_Elements[GetIndex(1, 1)]);
        }

        const bool operator!=(const Mat2x2& other)
        {
            return !(m_Elements[GetIndex(0, 0)] == other.m_Elements[GetIndex(0, 0)] &&
                     m_Elements[GetIndex(0, 1)] == other.m_Elements[GetIndex(0, 1)] &&

                     m_Elements[GetIndex(1, 0)] == other.m_Elements[GetIndex(1, 0)] &&
                     m_Elements[GetIndex(1, 1)] == other.m_Elements[GetIndex(1, 1)]);
        }

        static const Mat2x2 Transpose(const Mat2x2 mat)
        {
            return Mat2x2(mat.m_Elements[GetIndex(0, 0)], mat.m_Elements[GetIndex(1, 0)],
                          mat.m_Elements[GetIndex(0, 1)], mat.m_Elements[GetIndex(1, 1)]);
        }

      private:
        static constexpr inline int GetIndex(int column, int row) { return (column * 2) + row; }
    };
    using Mat2 = Mat2x2<float>;
#else
#error "Unsupported matrix configuration! Define DD_MATH_COLUMN_MAJOR and DD_COORDINATE_RIGHT_HANDED for a CRH matrix"
#endif
} // namespace Dodo::Math