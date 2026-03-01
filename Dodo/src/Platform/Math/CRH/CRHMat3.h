#pragma once

#include "Core/Math/Vector/Vec3.h"

namespace Dodo { namespace Platform {

    // Column Major 3x3 matrix
    template <typename T = float> struct CRHMat3x3 {

        union {
            T m_Elements[9];
            Math::TVec3<T> m_Columns[3];
        };

        CRHMat3x3() : m_Elements{0} {}
        CRHMat3x3(float diagonal) { Identity(diagonal); }

        CRHMat3x3(const Math::TVec3<T> &first, const Math::TVec3<T> &second, const Math::TVec3<T> &third)
        {
            m_Elements[GetIndex(0, 0)] = first.x;
            m_Elements[GetIndex(0, 1)] = first.y;
            m_Elements[GetIndex(0, 2)] = first.z;

            m_Elements[GetIndex(1, 0)] = second.x;
            m_Elements[GetIndex(1, 1)] = second.y;
            m_Elements[GetIndex(1, 2)] = second.z;

            m_Elements[GetIndex(2, 0)] = third.x;
            m_Elements[GetIndex(2, 1)] = third.y;
            m_Elements[GetIndex(2, 2)] = third.z;
        }

        CRHMat3x3(T first, T second, T third, T forth, T fifth, T sixth, T seventh, T eighth, T ninth)
        {
            m_Elements[GetIndex(0, 0)] = first;
            m_Elements[GetIndex(0, 1)] = second;
            m_Elements[GetIndex(0, 2)] = third;

            m_Elements[GetIndex(1, 0)] = forth;
            m_Elements[GetIndex(1, 1)] = fifth;
            m_Elements[GetIndex(1, 2)] = sixth;

            m_Elements[GetIndex(2, 0)] = seventh;
            m_Elements[GetIndex(2, 1)] = eighth;
            m_Elements[GetIndex(2, 2)] = ninth;
        }

        inline void Identity()
        {
            memset(m_Elements, 0, 9 * 4);

            m_Elements[GetIndex(0, 0)] = (T)1;
            m_Elements[GetIndex(1, 1)] = (T)1;
            m_Elements[GetIndex(2, 2)] = (T)1;
        }

        inline void Identity(float diagonal)
        {
            memset(m_Elements, 0, 9 * 4);

            m_Elements[GetIndex(0, 0)] = (T)diagonal;
            m_Elements[GetIndex(1, 1)] = (T)diagonal;
            m_Elements[GetIndex(2, 2)] = (T)diagonal;
        }

        // Addition

        const CRHMat3x3 &operator+=(const float scalar)
        {
            for (int i = 0; i < 9; i++)
            {
                this->m_Elements[i] += scalar;
            }
            return *this;
        }

        friend CRHMat3x3 operator+(CRHMat3x3 left, const float right) { return left += right; }

        const CRHMat3x3 &operator+=(const CRHMat3x3 &other)
        {
            for (int i = 0; i < 9; i++)
            {
                this->m_Elements[i] += other.m_Elements[i];
            }
            return *this;
        }

        friend CRHMat3x3 operator+(CRHMat3x3 left, const CRHMat3x3 &right) { return left += right; }

        // Subtraction

        const CRHMat3x3 &operator-=(const float scalar)
        {
            for (int i = 0; i < 9; i++)
            {
                this->m_Elements[i] -= scalar;
            }
            return *this;
        }

        friend CRHMat3x3 operator-(CRHMat3x3 left, const float right) { return left -= right; }

        const CRHMat3x3 &operator-=(const CRHMat3x3 &other)
        {
            for (int i = 0; i < 9; i++)
            {
                this->m_Elements[i] -= other.m_Elements[i];
            }
            return *this;
        }

        friend CRHMat3x3 operator-(CRHMat3x3 left, const CRHMat3x3 &right) { return left -= right; }

        // Multiplication

        const CRHMat3x3 &operator*=(const float scalar)
        {
            for (int i = 0; i < 9; i++)
                m_Elements[i] *= scalar;

            return *this;
        }

        friend CRHMat3x3 operator*(CRHMat3x3 left, const float right) { return left *= right; }

        const CRHMat3x3 &operator*=(const CRHMat3x3 &other)
        {
            const Math::TVec3<T> row0 =
                Math::TVec3<T>(m_Elements[GetIndex(0, 0)], m_Elements[GetIndex(1, 0)], m_Elements[GetIndex(2, 0)]);
            const Math::TVec3<T> row1 =
                Math::TVec3<T>(m_Elements[GetIndex(0, 1)], m_Elements[GetIndex(1, 1)], m_Elements[GetIndex(2, 1)]);
            const Math::TVec3<T> row2 =
                Math::TVec3<T>(m_Elements[GetIndex(0, 2)], m_Elements[GetIndex(1, 2)], m_Elements[GetIndex(2, 2)]);

            m_Elements[0] = other.m_Columns[0].Dot(row0);
            m_Elements[1] = other.m_Columns[0].Dot(row1);
            m_Elements[2] = other.m_Columns[0].Dot(row2);

            m_Elements[3] = other.m_Columns[1].Dot(row0);
            m_Elements[4] = other.m_Columns[1].Dot(row1);
            m_Elements[5] = other.m_Columns[1].Dot(row2);

            m_Elements[6] = other.m_Columns[2].Dot(row0);
            m_Elements[7] = other.m_Columns[2].Dot(row1);
            m_Elements[8] = other.m_Columns[2].Dot(row2);

            return *this;
        }

        friend CRHMat3x3 operator*(CRHMat3x3 left, const CRHMat3x3 &right) { return left *= right; }

        // Assignment

        const CRHMat3x3 &operator=(const CRHMat3x3 &other)
        {
            m_Elements[GetIndex(0, 0)] = other.m_Elements[GetIndex(0, 0)];
            m_Elements[GetIndex(0, 1)] = other.m_Elements[GetIndex(0, 1)];
            m_Elements[GetIndex(0, 2)] = other.m_Elements[GetIndex(0, 2)];

            m_Elements[GetIndex(1, 0)] = other.m_Elements[GetIndex(1, 0)];
            m_Elements[GetIndex(1, 1)] = other.m_Elements[GetIndex(1, 1)];
            m_Elements[GetIndex(1, 2)] = other.m_Elements[GetIndex(1, 2)];

            m_Elements[GetIndex(2, 0)] = other.m_Elements[GetIndex(2, 0)];
            m_Elements[GetIndex(2, 1)] = other.m_Elements[GetIndex(2, 1)];
            m_Elements[GetIndex(2, 2)] = other.m_Elements[GetIndex(2, 2)];
            return *this;
        }

        // Test

        const bool operator==(const CRHMat3x3 &other)
        {
            return (m_Elements[GetIndex(0, 0)] == other.m_Elements[GetIndex(0, 0)] &&
                    m_Elements[GetIndex(0, 1)] == other.m_Elements[GetIndex(0, 1)] &&
                    m_Elements[GetIndex(0, 2)] == other.m_Elements[GetIndex(0, 2)] &&

                    m_Elements[GetIndex(1, 0)] == other.m_Elements[GetIndex(1, 0)] &&
                    m_Elements[GetIndex(1, 1)] == other.m_Elements[GetIndex(1, 1)] &&
                    m_Elements[GetIndex(1, 2)] == other.m_Elements[GetIndex(1, 2)] &&

                    m_Elements[GetIndex(2, 0)] == other.m_Elements[GetIndex(2, 0)] &&
                    m_Elements[GetIndex(2, 1)] == other.m_Elements[GetIndex(2, 1)] &&
                    m_Elements[GetIndex(2, 2)] == other.m_Elements[GetIndex(2, 2)]);
        }

        const bool operator!=(const CRHMat3x3 &other)
        {
            return !(m_Elements[GetIndex(0, 0)] == other.m_Elements[GetIndex(0, 0)] &&
                     m_Elements[GetIndex(0, 1)] == other.m_Elements[GetIndex(0, 1)] &&
                     m_Elements[GetIndex(0, 2)] == other.m_Elements[GetIndex(0, 2)] &&

                     m_Elements[GetIndex(1, 0)] == other.m_Elements[GetIndex(1, 0)] &&
                     m_Elements[GetIndex(1, 1)] == other.m_Elements[GetIndex(1, 1)] &&
                     m_Elements[GetIndex(1, 2)] == other.m_Elements[GetIndex(1, 2)] &&

                     m_Elements[GetIndex(2, 0)] == other.m_Elements[GetIndex(2, 0)] &&
                     m_Elements[GetIndex(2, 1)] == other.m_Elements[GetIndex(2, 1)] &&
                     m_Elements[GetIndex(2, 2)] == other.m_Elements[GetIndex(2, 2)]);
        }

        ///////
        // Useful Matrices
        ///////

        static const CRHMat3x3 Transpose(const CRHMat3x3 mat)
        {
            return CRHMat3x3(
                mat.m_Elements[GetIndex(0, 0)], mat.m_Elements[GetIndex(1, 0)], mat.m_Elements[GetIndex(2, 0)],
                mat.m_Elements[GetIndex(0, 1)], mat.m_Elements[GetIndex(1, 1)], mat.m_Elements[GetIndex(2, 1)],
                mat.m_Elements[GetIndex(0, 2)], mat.m_Elements[GetIndex(1, 2)], mat.m_Elements[GetIndex(2, 2)]);
        }

        static const CRHMat3x3 Translate(const Math::TVec2<T> &translation)
        {
            CRHMat3x3 result;
            result.m_Elements[GetIndex(0, 0)] = (T)1;
            result.m_Elements[GetIndex(1, 1)] = (T)1;
            result.m_Elements[GetIndex(2, 0)] = translation.x;
            result.m_Elements[GetIndex(2, 1)] = translation.y;
            result.m_Elements[GetIndex(2, 2)] = (T)1;
            return result;
        }

        static const CRHMat3x3 Scale(const Math::TVec2<T> &scale)
        {
            CRHMat3x3 result;

            result.m_Elements[GetIndex(0, 0)] = scale.x;
            result.m_Elements[GetIndex(1, 1)] = scale.y;
            result.m_Elements[GetIndex(2, 2)] = (T)1;
            return result;
        }

        static const CRHMat3x3 Rotate(const float radians)
        {
            CRHMat3x3 result;

            const float c = cos(radians);
            const float s = sin(radians);

            result.m_Elements[GetIndex(0, 0)] = c;
            result.m_Elements[GetIndex(0, 1)] = -s;
            result.m_Elements[GetIndex(1, 0)] = s;
            result.m_Elements[GetIndex(1, 1)] = c;
            result.m_Elements[GetIndex(2, 2)] = (T)1;

            return result;
        }

      private:
        static constexpr inline int GetIndex(int column, int row) { return (column * 3) + row; }
    };
}} // namespace Dodo::Platform