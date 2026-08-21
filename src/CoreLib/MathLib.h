/**___________________________________________________________________________/
/**___________________________________________________________________________/
@file          MathLib.h
@author        Zhang Mingyang (mingyang.zhang) 100%
@date          18/09/2025

This file defines a math library for 2D game development, providing implementations 
of `Vec2` (2D vector) and `Mat3` (3x3 matrix) structures with supporting functions. 
It includes common vector operations, matrix transformations (translation, rotation, 
scaling), and utility functions for angles and projections.

  Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#pragma once
#include <initializer_list>
////////////////////////////////////////////////////////
// MATH LIBRARY
////////////////////////////////////////////////////////
// =====================================================
// Vec2 - 2D Vector
// =====================================================

/*!
* \brief
*    Default constructor for [Vec2], initializes vector components to zero.
*
* \brief
*    [float] x - X-coordinate of vector instance.
* 
* \brief
*    [float] y - Y-coordinate of vector instance.
*/
struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    /*!
    * \brief
    *    Default constructor for [Vec2], initializes vector components to zero.
    *
    * \param
    *    [None]
    *
    * \return
    *    [Vec2] A 2D vector with x and y initialized to 0.0f.
    */
    Vec2() = default;

    /*!
    * \brief
    *    Constructs a [Vec2] with specified x and y values.
    *
    * \param
    *    [float] xVal - The initial value for the x component.
    * \param
    *    [float] yVal - The initial value for the y component.
    *
    * \return
    *    [Vec2] A 2D vector initialized with the given values.
    */
    Vec2(float xVal, float yVal) : x(xVal), y(yVal){}

    /*!
    * \brief
    *    Adds another vector to the current vector and updates its values.
    *
    * \param
    *    [const Vec2&] rhs - The vector to add to the current vector.
    *
    * \return
    *    [Vec2&] Reference to the updated vector after addition.
    */
    Vec2&  operator+=(const Vec2& rhs);

    /*!
    * \brief
    *    Subtracts another vector from the current vector and updates its values.
    *
    * \param
    *    [const Vec2&] rhs - The vector to subtract from the current vector.
    *
    * \return
    *    [Vec2&] Reference to the updated vector after subtraction.
    */
    Vec2&  operator-=(const Vec2& rhs);

    /*!
    * \brief
    *    Scales the vector by a scalar value and updates its values.
    *
    * \param
    *    [float] scalar - The value to multiply each component by.
    *
    * \return
    *    [Vec2&] Reference to the updated vector after scaling.
    */
    Vec2&  operator*=(float scalar);

    /*!
    * \brief
    *    Divides the vector by a scalar value and updates its values.
    *
    * \param
    *    [float] scalar - The value to divide each component by.
    *
    * \return
    *    [Vec2&] Reference to the updated vector after division.
    */
    Vec2&  operator/=(float scalar);

    /*!
    * \brief
    *    Returns a new vector with its components negated.
    *
    * \param
    *    [None]
    *
    * \return
    *    [Vec2] A new vector where each component is the negation of the original.
    */
    Vec2     operator-() const;

    /*!
    * \brief
    *    Calculates the magnitude (length) of the vector.
    *
    * \param
    *    [None]
    *
    * \return
    *    [float] The length of the vector.
    */
    float    Length() const;

    /*!
    * \brief
    *    Calculates the squared length of the vector (avoids square root for performance).
    *
    * \param
    *    [None]
    *
    * \return
    *    [float] The squared length of the vector.
    */
    float    LengthSquared() const;

    /*!
    * \brief
    *    Returns a normalized copy of the vector (length of 1).
    *
    * \param
    *    [None]
    *
    * \return
    *    [Vec2] A new normalized vector.
    */
    Vec2     Normalised() const;

    /*!
    * \brief
    *    Normalizes the current vector, making its length 1.
    *
    * \param
    *    [None]
    *
    * \return
    *    [void]
    */
    void     Normalise();
};

// =====================================================
// Vec2 Helper Functions
// =====================================================

/*!
* \brief
*    Linearly interpolates between two vectors.
*
* \param
*    [const Vec2&] a - Starting vector.
* \param
*    [const Vec2&] b - Ending vector.
* \param
*    [float] t - Interpolation factor (0.0 to 1.0).
*
* \return
*    [Vec2] The interpolated vector.
*/
Vec2 Lerp(const Vec2& a, const Vec2& b, float t);

/*!
* \brief
*    Adds two vectors together.
*
* \param
*    [const Vec2&] lhs - Left-hand operand.
* \param
*    [const Vec2&] rhs - Right-hand operand.
*
* \return
*    [Vec2] The result of vector addition.
*/
Vec2 operator+(const Vec2& lhs, const Vec2& rhs);

/*!
* \brief
*    Subtracts one vector from another.
*
* \param
*    [const Vec2&] lhs - Left-hand operand.
* \param
*    [const Vec2&] rhs - Right-hand operand.
*
* \return
*    [Vec2] The result of vector subtraction.
*/
Vec2 operator-(const Vec2& lhs, const Vec2& rhs);

/*!
* \brief
*    Multiplies a vector by a scalar.
*
* \param
*    [const Vec2&] lhs - The vector to multiply.
* \param
*    [float] rhs - The scalar value to multiply by.
*
* \return
*    [Vec2] The scaled vector.
*/
Vec2 operator*(const Vec2& lhs, float rhs);

/*!
* \brief
*    Multiplies a scalar by a vector.
*
* \param
*    [float] lhs - The scalar value.
* \param
*    [const Vec2&] rhs - The vector to multiply.
*
* \return
*    [Vec2] The scaled vector.
*/
Vec2 operator*(float lhs, const Vec2& rhs);  

/*!
* \brief
*    Divides a vector by a scalar.
*
* \param
*    [const Vec2&] lhs - The vector to divide.
* \param
*    [float] rhs - The scalar divisor.
*
* \return
*    [Vec2] The scaled vector.
*/
Vec2 operator/(const Vec2& lhs, float rhs);

/*!
* \brief
*    Calculates the distance between two points.
*
* \param
*    [const Vec2&] a - The first point.
* \param
*    [const Vec2&] b - The second point.
*
* \return
*    [float] The Euclidean distance between the two points.
*/
float Vec2Distance(const Vec2& a, const Vec2& b);

/*!
* \brief
*    Calculates the squared distance between two points.
*
* \param
*    [const Vec2&] a - The first point.
* \param
*    [const Vec2&] b - The second point.
*
* \return
*    [float] The squared distance between the two points.
*/
float Vec2SquareDistance(const Vec2& a, const Vec2& b);

/*!
* \brief
*    Calculates the dot product of two vectors.
*
* \param
*    [const Vec2&] a - The first vector.
* \param
*    [const Vec2&] b - The second vector.
*
* \return
*    [float] The dot product result.
*/
float Vec2DotProduct(const Vec2& a, const Vec2& b);

/*!
* \brief
*    Calculates the cross product of two vectors (scalar in 2D).
*
* \param
*    [const Vec2&] a - The first vector.
* \param
*    [const Vec2&] b - The second vector.
*
* \return
*    [float] The cross product result.
*/
float Vec2CrossProduct(const Vec2& a, const Vec2& b); 

/*!
* \brief
*    Calculates the angle between two vectors in radians.
*
* \param
*    [const Vec2&] a - The first vector.
* \param
*    [const Vec2&] b - The second vector.
*
* \return
*    [float] The angle between the vectors in radians.
*/
float Vec2Angle(const Vec2& a, const Vec2& b); 

/*!
* \brief
*    Projects vector a onto vector b.
*
* \param
*    [const Vec2&] a - The vector to project.
* \param
*    [const Vec2&] b - The vector to project onto.
*
* \return
*    [Vec2] The projected vector.
*/
Vec2  Vec2Project(const Vec2& a, const Vec2& b);
Vec2  Vec2Reflect(const Vec2& a, const Vec2& n);

// =====================================================
// Mat3 - 3x3 Matrix 
// Used for 2D transforms: translation, rotation, scaling
// =====================================================

/*!
* \brief
*    Default constructor for [Vec2], initializes vector components to zero.
*
* \brief
*    [float] m[9] - 1d array of 9 float values in the following column-major order:  [1] [4] [7]
*                                                                                    [2] [5] [8]
*                                                                                    [3] [6] [9]                       
*/
struct  Mat3
{
    float m[9]; // [ m00 m01 m02  
                //   m10 m11 m12  
                //   m20 m21 m22 ]

    /*!
    * \brief
    *    Default constructor for [Mat3], initializes to identity matrix.
    *
    * \param
    *    [None]
    *
    * \return
    *    [Mat3] An identity 3x3 matrix.
    */
    Mat3();

    /*!
    * \brief
    *    Constructs a [Mat3] using 9 individual float values (column-major order).
    *
    * \param
    *    [float] m00 - Element at column 0, row 0.
    * \param
    *    [float] m01 - Element at column 0, row 1.
    * \param
    *    [float] m02 - Element at column 0, row 2.
    * \param
    *    [float] m10 - Element at column 1, row 0.
    * \param
    *    [float] m11 - Element at column 1, row 1.
    * \param
    *    [float] m12 - Element at column 1, row 2.
    * \param
    *    [float] m20 - Element at column 2, row 0.
    * \param
    *    [float] m21 - Element at column 2, row 1.
    * \param
    *    [float] m22 - Element at column 2, row 2.
    *
    * \return
    *    [Mat3] A 3X3 matrix initialized with provided values.
    */
    Mat3(float m00, float m01, float m02,
         float m10, float m11, float m12,
         float m20, float m21, float m22);


    /*!
    * \brief
    *    Creates and returns an identity matrix.
    *
    * \param
    *    [None]
    *
    * \return
    *    [Mat3] An identity matrix.
    */
    static Mat3 Identity();

    /*!
    * \brief
    *    Creates a translation matrix for 2D transformations.
    *
    * \param
    *    [float] tx - Translation along the X-axis.
    * \param
    *    [float] ty - Translation along the Y-axis.
    *
    * \return
    *    [Mat3] A translation matrix.
    */
    static Mat3 Translation(float tx, float ty);

    /*!
    * \brief
    *    Creates a rotation matrix using radians.
    *
    * \param
    *    [float] radians - Angle of rotation in radians.
    *
    * \return
    *    [Mat3] A rotation matrix.
    */
    static Mat3 Rotation(float radians);

    /*!
    * \brief
    *    Creates a scaling matrix for 2D transformations.
    *
    * \param
    *    [float] sx - Scaling factor along the X-axis.
    * \param
    *    [float] sy - Scaling factor along the Y-axis.
    *
    * \return
    *    [Mat3] A scaling matrix.
    */
    static Mat3 Scale(float sx, float sy);

    /*!
    * \brief
    *    Transforms a 2D point using the matrix.
    *
    * \param
    *    [const Vec2&] v - The point to transform.
    *
    * \return
    *    [Vec2] The transformed point.
    */
    Vec2  TransformPoint(const Vec2& v) const;

    /*!
    * \brief
    *    Transforms a 2D vector using the matrix.
    *
    * \param
    *    [const Vec2&] v - The vector to transform.
    *
    * \return
    *    [Vec2] The transformed vector.
    */
    Vec2  TransformVector(const Vec2& v) const;

    /*!
    * \brief
    *    Returns the transposed version of the matrix.
    *
    * \param
    *    [None]
    *
    * \return
    *    [Mat3] The transposed matrix.
    */
    Mat3  Transposed() const;

    /*!
    * \brief
    *    Returns the inverse of the matrix, if invertible.
    *
    * \param
    *    [float*] outDeterminant - Pointer to store the determinant value.
    *
    * \return
    *    [Mat3] The inverse matrix, or identity if not invertible.
    */
    Mat3  Inversed(float* outDeterminant = nullptr) const; // returns identity if singular

    /*!
    * \brief
    *    Multiplies the current matrix by another matrix and updates it.
    *
    * \param
    *    [const Mat3&] rhs - The matrix to multiply by.
    *
    * \return
    *    [Mat3&] Reference to the updated matrix.
    */
    Mat3& operator*=(const Mat3& rhs);
};

// =====================================================
// Mat3 Helper Functions
// =====================================================

/*!
* \brief
*    Multiplies two 3x3 matrices together.
*
* \param
*    [const Mat3&] lhs - The left-hand matrix.
* \param
*    [const Mat3&] rhs - The right-hand matrix.
*
* \return
*    [Mat3] The result of the matrix multiplication.
*/
Mat3     operator*(const Mat3& lhs, const Mat3& rhs); 

/*!
* \brief
*    Calculates the determinant of a 3x3 matrix.
*
* \param
*    [const Mat3&] mat - The matrix to compute the determinant of.
*
* \return
*    [float] The determinant value.
*/
float    Mat3Determinant(const Mat3& mat);

/*!
* \brief
*    Sets the provided matrix to an identity matrix.
*
* \param
*    [Mat3&] out - The matrix to modify.
*
* \return
*    [void]
*/
void     Mat3Identity(Mat3& out);

/*!
* \brief
*    Creates a translation matrix and stores it in the provided output.
*
* \param
*    [Mat3&] out - The matrix to store the result.
* \param
*    [float] x - Translation along the X-axis.
* \param
*    [float] y - Translation along the Y-axis.
*
* \return
*    [void]
*/
void     Mat3Translate(Mat3& out, float x, float y);

/*!
* \brief
*    Creates a scaling matrix and stores it in the provided output.
*
* \param
*    [Mat3&] out - The matrix to store the result.
* \param
*    [float] x - Scaling factor along the X-axis.
* \param
*    [float] y - Scaling factor along the Y-axis.
*
* \return
*    [void]
*/
void     Mat3Scale(Mat3& out, float x, float y);

/*!
* \brief
*    Creates a rotation matrix from an angle in radians.
*
* \param
*    [Mat3&] out - The matrix to store the result.
* \param
*    [float] angleRadians - The rotation angle in radians.
*
* \return
*    [void]
*/
void     Mat3RotRad(Mat3& out, float angleRadians);

/*!
* \brief
*    Creates a rotation matrix from an angle in degrees.
*
* \param
*    [Mat3&] out - The matrix to store the result.
* \param
*    [float] angleDegrees - The rotation angle in degrees.
*
* \return
*    [void]
*/
void     Mat3RotDeg(Mat3& out, float angleDegrees);

/*!
* \brief
*    Computes the transpose of the input matrix and stores it in the output.
*
* \param
*    [Mat3&] out - The matrix to store the result.
* \param
*    [const Mat3&] in - The matrix to transpose.
*
* \return
*    [void]
*/
void     Mat3Transpose(Mat3& out, const Mat3& in);

/*!
* \brief
*    Computes the inverse of the input matrix and stores it in the output.
*
* \param
*    [Mat3*] output - Pointer to the matrix to store the inverse.
* \param
*    [float*] determinant - Pointer to store the determinant value.
* \param
*    [const Mat3&] input - The matrix to invert.
*
* \return
*    [void]
*/
void     Mat3Inverse(Mat3* output, float* determinant, const Mat3& input);

// =====================================================
// Utility Math
// =====================================================
constexpr float PI = 3.14159265359f;

/*!
* \brief
*    Converts degrees to radians.
*
* \param
*    [float] degrees - The angle in degrees.
*
* \return
*    [float] The angle in radians.
*/
inline float ToRad(float degrees) { return degrees * (PI / 180.0f); }

/*!
* \brief
*    Converts radians to degrees.
*
* \param
*    [float] radians - The angle in radians.
*
* \return
*    [float] The angle in degrees.
*/
inline float ToDeg(float radians) { return radians * (180.0f / PI); }


/*!
* \brief
*    Multiplies a matrix with a vector, transforming the vector.
*
* \param
*    [const Mat3&] lhs - The matrix.
* \param
*    [const Vec2&] rhs - The vector.
*
* \return
*    [Vec2] The transformed vector.
*/
Vec2 operator*(const Mat3& lhs, const Vec2& rhs);



// =====================================================
// Colour Struct
// Used for graphics
// =====================================================
struct Color{
    float r = 0.f, g = 0.f, b = 0.f, a = 0.f;

    Color() : r{ 1 }, g{ 1 }, b{ 1 }, a{ 1 } {}
    Color(float R, float G, float B, float A = 1.f) : r{ R }, g{ G }, b{ B }, a{ A } {}
};



// =====================================================
// Vec3 - 3D Vector
// Used for 3D transforms: translation, rotation, scaling
// =====================================================
struct Vec3
{
    float x = 0.f, y = 0.f, z = 0.f;

    Vec3() = default;
    Vec3(float x, float y, float z);

    Vec3  operator-() const;

    Vec3& operator+=(const Vec3& rhs);
    Vec3& operator-=(const Vec3& rhs);
    Vec3& operator*=(float s);
    Vec3& operator/=(float s);

    float Length() const;
    float LengthSquared() const;

    Vec3  Normalised() const;
    void  Normalise();
};

// =====================================================
// Vec3 helpers
// =====================================================
Vec3 operator+(const Vec3& a, const Vec3& b);
Vec3 operator-(const Vec3& a, const Vec3& b);
Vec3 operator*(const Vec3& v, float s);
Vec3 operator*(float s, const Vec3& v);
Vec3 operator/(const Vec3& v, float s);

float Vec3Dot(const Vec3& a, const Vec3& b);
Vec3  Vec3Cross(const Vec3& a, const Vec3& b);
Vec3  Vec3Lerp(const Vec3& a, const Vec3& b, float t);


// =====================================================
// Vec4 - 4D Vector
// Used for graphics (R,G,B,A)
// =====================================================
struct Vec4
{
    float x = 0.f, y = 0.f, z = 0.f, w = 0.f;

    Vec4() = default;
    Vec4(float x, float y, float z, float w);

    Vec4  operator-() const;

    Vec4& operator+=(const Vec4& rhs);
    Vec4& operator-=(const Vec4& rhs);
    Vec4& operator*=(float s);
    Vec4& operator/=(float s);
};

// =====================================================
// Vec4 helpers
// =====================================================
Vec4 operator+(const Vec4& a, const Vec4& b);
Vec4 operator-(const Vec4& a, const Vec4& b);
Vec4 operator*(const Vec4& v, float s);
Vec4 operator*(float s, const Vec4& v);
Vec4 operator/(const Vec4& v, float s);


// =====================================================
// Mat4 - 4x4 Matrix
// Used for graphics
// =====================================================
struct Mat4
{
    float m[16]{}; // column-major: m[col*4 + row]

    Mat4(); // identity
    Mat4(std::initializer_list<float> list);
    Mat4(float m00, float m01, float m02, float m03,
         float m10, float m11, float m12, float m13,
         float m20, float m21, float m22, float m23,
         float m30, float m31, float m32, float m33);

    static Mat4 Identity();
    static Mat4 Translation(float x, float y, float z);
    static Mat4 Scale(float sx, float sy, float sz);
    static Mat4 RotationZ(float radians);

    Vec3 TransformPoint(const Vec3& v) const;
    Vec3 TransformVector(const Vec3& v) const;

    Mat4 Transposed() const;
    Mat4 Inversed(float* det = nullptr) const;

    Mat4& operator*=(const Mat4& rhs);

    /*!
    * \brief
    *    Creates an orthographic projection matrix for 2D rendering.
    *
    * \param
    *    [float] left   - Left boundary of the view volume.
    * \param
    *    [float] right  - Right boundary of the view volume.
    * \param
    *    [float] bottom - Bottom boundary of the view volume.
    * \param
    *    [float] top    - Top boundary of the view volume.
    * \param
    *    [float] nearZ  - Near clipping plane.
    * \param
    *    [float] farZ   - Far clipping plane.
    *
    * \return
    *    [Mat4] The orthographic projection matrix.
    */
    static Mat4 Ortho(float left, float right, float bottom, float top, float nearZ, float farZ);

    /*!
    * \brief
    *    Constructs a 2D transform matrix from translation, rotation (Z), and scale.
    *
    * \param
    *    [const Vec3&] position - Translation in 2D (x, y). z is kept.
    * \param
    *    [float] rotationZ      - Rotation around Z-axis in radians.
    * \param
    *    [const Vec3&] scale    - Non-uniform scale.
    *
    * \return
    *    [Mat4] The TRS transform matrix.
    */
    static Mat4 TRS(const Vec3& position, float rotationZ, const Vec3& scale);

    /*!
    * \brief
    *    Decomposes a 2D affine matrix into translation, rotation (Z), and scale.
    *
    * \param
    *    [Vec2&] outPos     - Output translation in 2D.
    * \param
    *    [float&] outRotZ   - Output rotation around Z-axis (radians).
    * \param
    *    [Vec2&] outScale   - Output 2D scale.
    *
    * \return
    *    [void]
    */
    void Decompose2D(Vec2& outPos, float& outRotZ, Vec2& outScale) const;

    /*!
    * \brief
    *    Transforms a normal vector using the 2D rotation+scale block of the matrix.
    *    Ignores translation.
    *
    * \param
    *    [const Vec2&] v - Input normal.
    *
    * \return
    *    [Vec2] Transformed normal.
    */
    Vec2 TransformNormal2D(const Vec2& v) const;
};

// =====================================================
// Mat4 helpers
// =====================================================
Mat4 operator*(const Mat4& a, const Mat4& b);
Vec4 operator*(const Mat4& m, const Vec4& v);
Vec3 operator*(const Mat4& m, const Vec3& v); // treats w = 1 for point
float Mat4Determinant(const Mat4& m);


// Optional test for MathLib
void RunMathLibTests();
