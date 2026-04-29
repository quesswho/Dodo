#pragma once

#include <Core/Common.h>

#include "Core/Math/MathFunc.h"
#include "Core/Math/Matrix/Mat3.h"
#include "Core/Math/Vector/Vec4.h"

namespace Dodo::Math {

#if defined(DD_MATH_COLUMN_MAJOR) && defined(DD_COORDINATE_RIGHT_HANDED)

    // Column Major Right hand 4x4 matrix
    template <typename T = float>
    struct Mat4x4 {

        union {
            T m_Elements[16];
            Math::TVec4<T> m_Columns[4];
        };

        Mat4x4() : m_Elements{0} {}
        Mat4x4(float diagonal) { Identity(diagonal); }

        Mat4x4(const Math::TVec4<T>& first, const Math::TVec4<T>& second, const Math::TVec4<T>& third,
               const Math::TVec4<T>& forth)
        {
            m_Elements[GetIndex(0, 0)] = first.x;
            m_Elements[GetIndex(0, 1)] = first.y;
            m_Elements[GetIndex(0, 2)] = first.z;
            m_Elements[GetIndex(0, 3)] = first.w;

            m_Elements[GetIndex(1, 0)] = second.x;
            m_Elements[GetIndex(1, 1)] = second.y;
            m_Elements[GetIndex(1, 2)] = second.z;
            m_Elements[GetIndex(1, 3)] = second.w;

            m_Elements[GetIndex(2, 0)] = third.x;
            m_Elements[GetIndex(2, 1)] = third.y;
            m_Elements[GetIndex(2, 2)] = third.z;
            m_Elements[GetIndex(2, 3)] = third.w;

            m_Elements[GetIndex(3, 0)] = forth.x;
            m_Elements[GetIndex(3, 1)] = forth.y;
            m_Elements[GetIndex(3, 2)] = forth.z;
            m_Elements[GetIndex(3, 3)] = forth.w;
        }

        Mat4x4(T first, T second, T third, T forth, T fifth, T sixth, T seventh, T eighth, T ninth, T tenth, T eleventh,
               T twelveth, T thirteenth, T fourteenth, T fifteenth, T sixteenth)
        {
            m_Elements[GetIndex(0, 0)] = first;
            m_Elements[GetIndex(0, 1)] = second;
            m_Elements[GetIndex(0, 2)] = third;
            m_Elements[GetIndex(0, 3)] = forth;

            m_Elements[GetIndex(1, 0)] = fifth;
            m_Elements[GetIndex(1, 1)] = sixth;
            m_Elements[GetIndex(1, 2)] = seventh;
            m_Elements[GetIndex(1, 3)] = eighth;

            m_Elements[GetIndex(2, 0)] = ninth;
            m_Elements[GetIndex(2, 1)] = tenth;
            m_Elements[GetIndex(2, 2)] = eleventh;
            m_Elements[GetIndex(2, 3)] = twelveth;

            m_Elements[GetIndex(3, 0)] = thirteenth;
            m_Elements[GetIndex(3, 1)] = fourteenth;
            m_Elements[GetIndex(3, 2)] = fifteenth;
            m_Elements[GetIndex(3, 3)] = sixteenth;
        }

        Mat4x4(const Math::Mat3x3<T>& mat)
        {
            Identity();

            m_Elements[GetIndex(0, 0)] = mat.m_Elements[0];
            m_Elements[GetIndex(0, 1)] = mat.m_Elements[1];
            m_Elements[GetIndex(0, 2)] = mat.m_Elements[2];

            m_Elements[GetIndex(1, 0)] = mat.m_Elements[3];
            m_Elements[GetIndex(1, 1)] = mat.m_Elements[4];
            m_Elements[GetIndex(1, 2)] = mat.m_Elements[5];

            m_Elements[GetIndex(2, 0)] = mat.m_Elements[6];
            m_Elements[GetIndex(2, 1)] = mat.m_Elements[7];
            m_Elements[GetIndex(2, 2)] = mat.m_Elements[8];
        }

        inline void Identity(float diagonal = 1.0f)
        {
            for (int i = 0; i < 16; ++i)
                m_Elements[i] = T(0);

            m_Elements[GetIndex(0, 0)] = (T)diagonal;
            m_Elements[GetIndex(1, 1)] = (T)diagonal;
            m_Elements[GetIndex(2, 2)] = (T)diagonal;
            m_Elements[GetIndex(3, 3)] = (T)diagonal;
        }

        inline T operator[](int i) const { this->m_Elements[i]; }

        // Addition

        const Mat4x4& operator+=(const float scalar)
        {
            for (int i = 0; i < 16; i++) {
                this->m_Elements[i] += scalar;
            }
            return *this;
        }

        friend Mat4x4 operator+(Mat4x4 left, const float right) { return left += right; }

        const Mat4x4& operator+=(const Mat4x4& other)
        {
            for (int i = 0; i < 16; i++) {
                this->m_Elements[i] += other.m_Elements[i];
            }
            return *this;
        }

        friend Mat4x4 operator+(Mat4x4 left, const Mat4x4& right) { return left += right; }

        // Subtraction

        const Mat4x4& operator-=(const float scalar)
        {
            for (int i = 0; i < 16; i++) {
                this->m_Elements[i] -= scalar;
            }
            return *this;
        }

        friend Mat4x4 operator-(Mat4x4 left, const float right) { return left -= right; }

        const Mat4x4& operator-=(const Mat4x4& other)
        {
            for (int i = 0; i < 16; i++) {
                this->m_Elements[i] -= other.m_Elements[i];
            }
            return *this;
        }

        friend Mat4x4 operator-(Mat4x4 left, const Mat4x4& right) { return left -= right; }

        // Multiplication

        const Mat4x4& operator*=(const float scalar)
        {
            for (int i = 0; i < 16; i++)
                m_Elements[i] *= scalar;

            return *this;
        }

        friend Mat4x4 operator*(Mat4x4 left, const float right) { return left *= right; }

        inline const Mat4x4& operator*=(const Mat4x4& other)
        {

            const Math::TVec4 row0 = Math::TVec4(m_Elements[GetIndex(0, 0)], m_Elements[GetIndex(1, 0)],
                                                 m_Elements[GetIndex(2, 0)], m_Elements[GetIndex(3, 0)]);
            const Math::TVec4 row1 = Math::TVec4(m_Elements[GetIndex(0, 1)], m_Elements[GetIndex(1, 1)],
                                                 m_Elements[GetIndex(2, 1)], m_Elements[GetIndex(3, 1)]);
            const Math::TVec4 row2 = Math::TVec4(m_Elements[GetIndex(0, 2)], m_Elements[GetIndex(1, 2)],
                                                 m_Elements[GetIndex(2, 2)], m_Elements[GetIndex(3, 2)]);
            const Math::TVec4 row3 = Math::TVec4(m_Elements[GetIndex(0, 3)], m_Elements[GetIndex(1, 3)],
                                                 m_Elements[GetIndex(2, 3)], m_Elements[GetIndex(3, 3)]);

            m_Elements[0] = other.m_Columns[0].Dot(row0);
            m_Elements[1] = other.m_Columns[0].Dot(row1);
            m_Elements[2] = other.m_Columns[0].Dot(row2);
            m_Elements[3] = other.m_Columns[0].Dot(row3);

            m_Elements[4] = other.m_Columns[1].Dot(row0);
            m_Elements[5] = other.m_Columns[1].Dot(row1);
            m_Elements[6] = other.m_Columns[1].Dot(row2);
            m_Elements[7] = other.m_Columns[1].Dot(row3);

            m_Elements[8] = other.m_Columns[2].Dot(row0);
            m_Elements[9] = other.m_Columns[2].Dot(row1);
            m_Elements[10] = other.m_Columns[2].Dot(row2);
            m_Elements[11] = other.m_Columns[2].Dot(row3);

            m_Elements[12] = other.m_Columns[3].Dot(row0);
            m_Elements[13] = other.m_Columns[3].Dot(row1);
            m_Elements[14] = other.m_Columns[3].Dot(row2);
            m_Elements[15] = other.m_Columns[3].Dot(row3);

            return *this;
        }

        friend Mat4x4 operator*(Mat4x4 left, const Mat4x4& right) { return left *= right; }

        friend Math::TVec4<T> operator*(const Mat4x4& left, const Math::TVec4<T>& right)
        {
            const Math::TVec4<T> row0(left.m_Elements[GetIndex(0, 0)], left.m_Elements[GetIndex(1, 0)],
                                      left.m_Elements[GetIndex(2, 0)], left.m_Elements[GetIndex(3, 0)]);
            const Math::TVec4<T> row1(left.m_Elements[GetIndex(0, 1)], left.m_Elements[GetIndex(1, 1)],
                                      left.m_Elements[GetIndex(2, 1)], left.m_Elements[GetIndex(3, 1)]);
            const Math::TVec4<T> row2(left.m_Elements[GetIndex(0, 2)], left.m_Elements[GetIndex(1, 2)],
                                      left.m_Elements[GetIndex(2, 2)], left.m_Elements[GetIndex(3, 2)]);
            const Math::TVec4<T> row3(left.m_Elements[GetIndex(0, 3)], left.m_Elements[GetIndex(1, 3)],
                                      left.m_Elements[GetIndex(2, 3)], left.m_Elements[GetIndex(3, 3)]);

            return Math::TVec4<T>(row0.Dot(right), row1.Dot(right), row2.Dot(right), row3.Dot(right));
        }

        // Assignment

        const Mat4x4& operator=(const Mat4x4& other)
        {
            m_Elements[GetIndex(0, 0)] = other.m_Elements[GetIndex(0, 0)];
            m_Elements[GetIndex(0, 1)] = other.m_Elements[GetIndex(0, 1)];
            m_Elements[GetIndex(0, 2)] = other.m_Elements[GetIndex(0, 2)];
            m_Elements[GetIndex(0, 3)] = other.m_Elements[GetIndex(0, 3)];

            m_Elements[GetIndex(1, 0)] = other.m_Elements[GetIndex(1, 0)];
            m_Elements[GetIndex(1, 1)] = other.m_Elements[GetIndex(1, 1)];
            m_Elements[GetIndex(1, 2)] = other.m_Elements[GetIndex(1, 2)];
            m_Elements[GetIndex(1, 3)] = other.m_Elements[GetIndex(1, 3)];

            m_Elements[GetIndex(2, 0)] = other.m_Elements[GetIndex(2, 0)];
            m_Elements[GetIndex(2, 1)] = other.m_Elements[GetIndex(2, 1)];
            m_Elements[GetIndex(2, 2)] = other.m_Elements[GetIndex(2, 2)];
            m_Elements[GetIndex(2, 3)] = other.m_Elements[GetIndex(2, 3)];

            m_Elements[GetIndex(3, 0)] = other.m_Elements[GetIndex(3, 0)];
            m_Elements[GetIndex(3, 1)] = other.m_Elements[GetIndex(3, 1)];
            m_Elements[GetIndex(3, 2)] = other.m_Elements[GetIndex(3, 2)];
            m_Elements[GetIndex(3, 3)] = other.m_Elements[GetIndex(3, 3)];
            return *this;
        }

        // Test

        const bool operator==(const Mat4x4& other) const
        {
            return (m_Elements[GetIndex(0, 0)] == other.m_Elements[GetIndex(0, 0)] &&
                    m_Elements[GetIndex(0, 1)] == other.m_Elements[GetIndex(0, 1)] &&
                    m_Elements[GetIndex(0, 2)] == other.m_Elements[GetIndex(0, 2)] &&
                    m_Elements[GetIndex(0, 3)] == other.m_Elements[GetIndex(0, 3)] &&

                    m_Elements[GetIndex(1, 0)] == other.m_Elements[GetIndex(1, 0)] &&
                    m_Elements[GetIndex(1, 1)] == other.m_Elements[GetIndex(1, 1)] &&
                    m_Elements[GetIndex(1, 2)] == other.m_Elements[GetIndex(1, 2)] &&
                    m_Elements[GetIndex(1, 3)] == other.m_Elements[GetIndex(1, 3)] &&

                    m_Elements[GetIndex(2, 0)] == other.m_Elements[GetIndex(2, 0)] &&
                    m_Elements[GetIndex(2, 1)] == other.m_Elements[GetIndex(2, 1)] &&
                    m_Elements[GetIndex(2, 2)] == other.m_Elements[GetIndex(2, 2)] &&
                    m_Elements[GetIndex(2, 3)] == other.m_Elements[GetIndex(2, 3)] &&

                    m_Elements[GetIndex(3, 0)] == other.m_Elements[GetIndex(3, 0)] &&
                    m_Elements[GetIndex(3, 1)] == other.m_Elements[GetIndex(3, 1)] &&
                    m_Elements[GetIndex(3, 2)] == other.m_Elements[GetIndex(3, 2)] &&
                    m_Elements[GetIndex(3, 3)] == other.m_Elements[GetIndex(3, 3)]);
        }

        const bool operator!=(const Mat4x4& other) const
        {
            return !(m_Elements[GetIndex(0, 0)] == other.m_Elements[GetIndex(0, 0)] &&
                     m_Elements[GetIndex(0, 1)] == other.m_Elements[GetIndex(0, 1)] &&
                     m_Elements[GetIndex(0, 2)] == other.m_Elements[GetIndex(0, 2)] &&
                     m_Elements[GetIndex(0, 3)] == other.m_Elements[GetIndex(0, 3)] &&

                     m_Elements[GetIndex(1, 0)] == other.m_Elements[GetIndex(1, 0)] &&
                     m_Elements[GetIndex(1, 1)] == other.m_Elements[GetIndex(1, 1)] &&
                     m_Elements[GetIndex(1, 2)] == other.m_Elements[GetIndex(1, 2)] &&
                     m_Elements[GetIndex(1, 3)] == other.m_Elements[GetIndex(1, 3)] &&

                     m_Elements[GetIndex(2, 0)] == other.m_Elements[GetIndex(2, 0)] &&
                     m_Elements[GetIndex(2, 1)] == other.m_Elements[GetIndex(2, 1)] &&
                     m_Elements[GetIndex(2, 2)] == other.m_Elements[GetIndex(2, 2)] &&
                     m_Elements[GetIndex(2, 3)] == other.m_Elements[GetIndex(2, 3)] &&

                     m_Elements[GetIndex(3, 0)] == other.m_Elements[GetIndex(3, 0)] &&
                     m_Elements[GetIndex(3, 1)] == other.m_Elements[GetIndex(3, 1)] &&
                     m_Elements[GetIndex(3, 2)] == other.m_Elements[GetIndex(3, 2)] &&
                     m_Elements[GetIndex(3, 3)] == other.m_Elements[GetIndex(3, 3)]);
        }

        ///////
        // Useful Matrices
        ///////

        static const Mat4x4 Transpose(const Mat4x4 mat)
        {
            return Mat4x4(
                mat.m_Elements[GetIndex(0, 0)], mat.m_Elements[GetIndex(1, 0)], mat.m_Elements[GetIndex(2, 0)],
                mat.m_Elements[GetIndex(3, 0)], mat.m_Elements[GetIndex(0, 1)], mat.m_Elements[GetIndex(1, 1)],
                mat.m_Elements[GetIndex(2, 1)], mat.m_Elements[GetIndex(3, 1)], mat.m_Elements[GetIndex(0, 2)],
                mat.m_Elements[GetIndex(1, 2)], mat.m_Elements[GetIndex(2, 2)], mat.m_Elements[GetIndex(3, 2)],
                mat.m_Elements[GetIndex(0, 3)], mat.m_Elements[GetIndex(1, 3)], mat.m_Elements[GetIndex(2, 3)],
                mat.m_Elements[GetIndex(3, 3)]);
        }

        static const Mat4x4 Inverse(const Mat4x4& mat)
        {
            const T a00 = mat.m_Elements[GetIndex(0, 0)];
            const T a01 = mat.m_Elements[GetIndex(1, 0)];
            const T a02 = mat.m_Elements[GetIndex(2, 0)];
            const T a03 = mat.m_Elements[GetIndex(3, 0)];
            const T a10 = mat.m_Elements[GetIndex(0, 1)];
            const T a11 = mat.m_Elements[GetIndex(1, 1)];
            const T a12 = mat.m_Elements[GetIndex(2, 1)];
            const T a13 = mat.m_Elements[GetIndex(3, 1)];
            const T a20 = mat.m_Elements[GetIndex(0, 2)];
            const T a21 = mat.m_Elements[GetIndex(1, 2)];
            const T a22 = mat.m_Elements[GetIndex(2, 2)];
            const T a23 = mat.m_Elements[GetIndex(3, 2)];
            const T a30 = mat.m_Elements[GetIndex(0, 3)];
            const T a31 = mat.m_Elements[GetIndex(1, 3)];
            const T a32 = mat.m_Elements[GetIndex(2, 3)];
            const T a33 = mat.m_Elements[GetIndex(3, 3)];

            const T f00 = a22 * a33 - a23 * a32;
            const T f01 = a21 * a33 - a23 * a31;
            const T f02 = a21 * a32 - a22 * a31;
            const T f03 = a20 * a33 - a23 * a30;
            const T f04 = a20 * a32 - a22 * a30;
            const T f05 = a20 * a31 - a21 * a30;
            const T f06 = a12 * a33 - a13 * a32;
            const T f07 = a11 * a33 - a13 * a31;
            const T f08 = a11 * a32 - a12 * a31;
            const T f09 = a10 * a33 - a13 * a30;
            const T f10 = a10 * a32 - a12 * a30;
            const T f11 = a10 * a31 - a11 * a30;
            const T f12 = a12 * a23 - a13 * a22;
            const T f13 = a11 * a23 - a13 * a21;
            const T f14 = a11 * a22 - a12 * a21;
            const T f15 = a10 * a23 - a13 * a20;
            const T f16 = a10 * a22 - a12 * a20;
            const T f17 = a10 * a21 - a11 * a20;

            const T c00 = (a11 * f00 - a12 * f01 + a13 * f02);
            const T c01 = -(a10 * f00 - a12 * f03 + a13 * f04);
            const T c02 = (a10 * f01 - a11 * f03 + a13 * f05);
            const T c03 = -(a10 * f02 - a11 * f04 + a12 * f05);

            const T determinant = a00 * c00 + a01 * c01 + a02 * c02 + a03 * c03;
            if (determinant == T(0)) {
                return Mat4x4(T(1));
            }

            const T invDet = T(1) / determinant;
            const T c10 = -(a01 * f00 - a02 * f01 + a03 * f02);
            const T c11 = (a00 * f00 - a02 * f03 + a03 * f04);
            const T c12 = -(a00 * f01 - a01 * f03 + a03 * f05);
            const T c13 = (a00 * f02 - a01 * f04 + a02 * f05);
            const T c20 = (a01 * f06 - a02 * f07 + a03 * f08);
            const T c21 = -(a00 * f06 - a02 * f09 + a03 * f10);
            const T c22 = (a00 * f07 - a01 * f09 + a03 * f11);
            const T c23 = -(a00 * f08 - a01 * f10 + a02 * f11);
            const T c30 = -(a01 * f12 - a02 * f13 + a03 * f14);
            const T c31 = (a00 * f12 - a02 * f15 + a03 * f16);
            const T c32 = -(a00 * f13 - a01 * f15 + a03 * f17);
            const T c33 = (a00 * f14 - a01 * f16 + a02 * f17);

            return Mat4x4(c00 * invDet, c01 * invDet, c02 * invDet, c03 * invDet, c10 * invDet, c11 * invDet,
                          c12 * invDet, c13 * invDet, c20 * invDet, c21 * invDet, c22 * invDet, c23 * invDet,
                          c30 * invDet, c31 * invDet, c32 * invDet, c33 * invDet);
        }

        static inline const Mat4x4<T> Translate(const Math::TVec3<T>& translation)
        {
            Mat4x4 result(1.0f);

            result.m_Elements[GetIndex(3, 0)] = translation.x;
            result.m_Elements[GetIndex(3, 1)] = translation.y;
            result.m_Elements[GetIndex(3, 2)] = translation.z;
            return result;
        }

        static inline const Mat4x4<T> Scale(const Math::TVec3<T>& scale)
        {
            Mat4x4 result(1.0f);

            result.m_Elements[GetIndex(0, 0)] = scale.x;
            result.m_Elements[GetIndex(1, 1)] = scale.y;
            result.m_Elements[GetIndex(2, 2)] = scale.z;
            return result;
        }

        // Recommended to normalize axis
        static inline Mat4x4<T> Rotate(const float radians, const Math::TVec3<T>& axis)
        {
            Mat4x4 result(1.0f);

            const float c = cos(radians);
            const float s = sin(radians);

            const Math::TVec3<T> temp((T(1) - c) * axis);

            result.m_Elements[GetIndex(0, 0)] = c + axis.x * temp.x;
            float aewqea = c + axis.x * temp.x;
            result.m_Elements[GetIndex(0, 1)] = s * axis.z + temp.x * axis.y;
            result.m_Elements[GetIndex(0, 2)] = -s * axis.y + temp.x * axis.z;

            result.m_Elements[GetIndex(1, 0)] = -s * axis.z + temp.y * axis.x;
            result.m_Elements[GetIndex(1, 1)] = c + axis.y * temp.y;
            result.m_Elements[GetIndex(1, 2)] = s * axis.x + temp.y * axis.z;

            result.m_Elements[GetIndex(2, 0)] = s * axis.y + temp.z * axis.x;
            result.m_Elements[GetIndex(2, 1)] = -s * axis.x + temp.z * axis.y;
            result.m_Elements[GetIndex(2, 2)] = c + axis.z * temp.z;

            return result;
        }

        // Infinite far plane
        static inline const Mat4x4<T> Orthographic(float left, float right, float bottom, float top)
        {
            return Mat4x4(2 / (right - left), 0, 0, 0, 0, 2 / (top - bottom), 0, 0, 0, 0, -1, 0,
                          -(right + left) / (right - left), -(top + bottom) / (top - bottom), 0, 1);
        }

        static inline const Mat4x4<T> Orthographic(float left, float right, float bottom, float top, float zNear,
                                                   float zFar)
        {
#if defined(DD_DEPTH_ZERO_TO_ONE)
            return Mat4x4(2 / (right - left), 0, 0, 0, 0, 2 / (top - bottom), 0, 0, 0, 0, -1 / (zFar - zNear), 0,
                          -(right + left) / (right - left), -(top + bottom) / (top - bottom), -zNear / (zFar - zNear),
                          1);
#else
            return Mat4x4(2 / (right - left), 0, 0, 0, 0, 2 / (top - bottom), 0, 0, 0, 0, -2 / (zFar - zNear), 0,
                          -(right + left) / (right - left), -(top + bottom) / (top - bottom),
                          -(zFar + zNear) / (zFar - zNear), 1);
#endif
        }

        // Use degrees
        static inline const Mat4x4<T> Perspective(float fov, float aspectratio, float zNear, float zFar)
        {
            const float tanHalfFov = tan(Math::ToRadians(fov) / 2);
#if defined(DD_DEPTH_ZERO_TO_ONE)
            return Mat4x4(1 / (aspectratio * tanHalfFov), 0, 0, 0, 0, 1 / tanHalfFov, 0, 0, 0, 0,
                          -zFar / (zFar - zNear), -1, 0, 0, -(zFar * zNear) / (zFar - zNear), 0);
#else
            return Mat4x4(1 / (aspectratio * tanHalfFov), 0, 0, 0, 0, 1 / tanHalfFov, 0, 0, 0, 0,
                          -(zFar + zNear) / (zFar - zNear), -1, 0, 0, -(2 * zFar * zNear) / (zFar - zNear), 0);
#endif
        }

        static inline const Mat4x4 LookAt(const Math::Vec3& eye, const Math::Vec3& to, const Math::Vec3& up)
        {
            Mat4x4 result(1.0f);

            const Math::Vec3 f = Normalize(to - eye);
            const Math::Vec3 r = Normalize(Cross(f, up));
            const Math::Vec3 u = Cross(r, f);

            result.m_Elements[GetIndex(0, 0)] = r.x;
            result.m_Elements[GetIndex(1, 0)] = r.y;
            result.m_Elements[GetIndex(2, 0)] = r.z;
            result.m_Elements[GetIndex(0, 1)] = u.x;
            result.m_Elements[GetIndex(1, 1)] = u.y;
            result.m_Elements[GetIndex(2, 1)] = u.z;
            result.m_Elements[GetIndex(0, 2)] = -f.x;
            result.m_Elements[GetIndex(1, 2)] = -f.y;
            result.m_Elements[GetIndex(2, 2)] = -f.z;
            result.m_Elements[GetIndex(3, 0)] = -eye.Dot(r);
            result.m_Elements[GetIndex(3, 1)] = -eye.Dot(u);
            result.m_Elements[GetIndex(3, 2)] = eye.Dot(f);
            return result;
        }

        static inline const Mat4x4 LookDir(const Math::Vec3& pos, const Math::Vec3& dir, const Math::Vec3& up)
        {
            Mat4x4 result(1.0f);

            const Math::Vec3 r = Normalize(Cross(dir, up));
            const Math::Vec3 u = Cross(r, dir);

            result.m_Elements[GetIndex(0, 0)] = r.x;
            result.m_Elements[GetIndex(1, 0)] = r.y;
            result.m_Elements[GetIndex(2, 0)] = r.z;
            result.m_Elements[GetIndex(0, 1)] = u.x;
            result.m_Elements[GetIndex(1, 1)] = u.y;
            result.m_Elements[GetIndex(2, 1)] = u.z;
            result.m_Elements[GetIndex(0, 2)] = -dir.x;
            result.m_Elements[GetIndex(1, 2)] = -dir.y;
            result.m_Elements[GetIndex(2, 2)] = -dir.z;
            result.m_Elements[GetIndex(3, 0)] = -pos.Dot(r);
            result.m_Elements[GetIndex(3, 1)] = -pos.Dot(u);
            result.m_Elements[GetIndex(3, 2)] = pos.Dot(dir);
            return result;
            return result;
        }
        /////
        // Useful static functions
        /////

        static inline const Mat4x4<T> Multiply(const Mat4x4<T>& mat) { return mat; }

        template <class... O>
        static const Mat4x4<T> Multiply(const Mat4x4<T>& mat, O... others)
        {
            Mat4x4<T> result;
            result = mat * Multiply(others...);
            return result;
        }

        static inline const Mat4x4<T> RelinquishToMat3(const Mat4x4<T>& mat)
        {
            Mat4x4<T> result = mat;
            result.m_Elements[GetIndex(0, 3)] = 0.0f;
            result.m_Elements[GetIndex(1, 3)] = 0.0f;
            result.m_Elements[GetIndex(2, 3)] = 0.0f;
            result.m_Elements[GetIndex(3, 0)] = 0.0f;
            result.m_Elements[GetIndex(3, 1)] = 0.0f;
            result.m_Elements[GetIndex(3, 2)] = 0.0f;
            result.m_Elements[GetIndex(3, 3)] = 0.0f;
            return result;
        }

      private:
        static constexpr inline int GetIndex(int column, int row) { return (column * 4) + row; }
    };
    using Mat4 = Mat4x4<float>;
#else
#error "Unsupported matrix configuration! Define DD_MATH_COLUMN_MAJOR and DD_COORDINATE_RIGHT_HANDED for a CRH matrix"
#endif
} // namespace Dodo::Math
