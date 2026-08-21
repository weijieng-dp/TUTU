/*!
@file       MathLib.cpp
@author     Zhang Mingyang (mingyang.zhang) 100%
@date       18/09/2025
@brief		Implements a math library providing 2D vector (Vec2) and 3x3 matrix 
            (Mat3) operations, including arithmetic, transformations, interpolation, 
            and geometric calculations for use in physics and graphics systems.


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*______________________________________________________________________*/
#include "pch.h"
#include "MathLib.h"

// =====================================================
// Vec2 - 2D Vector
// =====================================================
Vec2& Vec2::operator+=(const Vec2& rhs){ x += rhs.x; y += rhs.y; return *this; }
Vec2& Vec2::operator-=(const Vec2& rhs){ x -= rhs.x; y -= rhs.y; return *this; }
Vec2& Vec2::operator*=(float scalar){ x *= scalar; y *= scalar; return *this; }
Vec2& Vec2::operator/=(float scalar){ x /= scalar; y /= scalar; return *this; }

// Unary -
Vec2 Vec2::operator-() const { return Vec2(-x,-y); }

// Measures distance from origin (0,0) to the vector's point
float Vec2::LengthSquared() const { return x*x + y*y; }
float Vec2::Length() const { return std::sqrt(LengthSquared()); }

// Normalise functions - one for copy of value, one to modify variable itself
Vec2 Vec2::Normalised() const {
    float len = Length();
    return (len > 0.0f) ? Vec2{x/len, y/len} : Vec2{0.0f, 0.0f};
}
void Vec2::Normalise() {
    float len = Length();
    if (len > 0.0f) { 
        x /= len; 
        y /= len; 
    }
}

// Lerp for interpolation purposes
Vec2 Lerp(const Vec2& a, const Vec2& b, float t){ return a + (b - a)*t; }

// Binary operators
Vec2 operator+(const Vec2& lhs, const Vec2& rhs) { return Vec2{lhs.x + rhs.x , lhs.y + rhs.y};}
Vec2 operator-(const Vec2& lhs, const Vec2& rhs) { return Vec2{lhs.x - rhs.x , lhs.y - rhs.y};}
Vec2 operator*(const Vec2& lhs, float rhs) { return Vec2{lhs.x * rhs , lhs.y * rhs};}
Vec2 operator*(float lhs, const Vec2& rhs) { return Vec2{lhs * rhs.x , lhs * rhs.y};}  
Vec2 operator/(const Vec2& lhs, float rhs) { return Vec2{lhs.x / rhs , lhs.y / rhs};}

// Measures distance between two points
float Vec2SquareDistance(const Vec2& a, const Vec2& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return dy*dy + dx*dx;
}
float Vec2Distance(const Vec2& a, const Vec2& b){ return std::sqrtf(Vec2SquareDistance(a,b));}

// Dot and Cross Products for 2D Vectors
float Vec2DotProduct(const Vec2& a, const Vec2& b){ return a.x*b.x + a.y*b.y; }
float Vec2CrossProduct(const Vec2& a, const Vec2& b){ return a.x*b.y - a.y*b.x; }

// Angle between Vectors
float Vec2Angle(const Vec2& a, const Vec2& b) {
    float dot = Vec2DotProduct(a.Normalised(), b.Normalised());
    return std::acos(std::clamp(dot, -1.0f, 1.0f));
}

// Projection of Vector A onto B
Vec2  Vec2Project(const Vec2& a, const Vec2& b) {
    float denom = Vec2DotProduct(b, b);
    if (denom < 1e-8f) return Vec2(0, 0);
    return b * (Vec2DotProduct(a, b) / Vec2DotProduct(b, b));
}

Vec2 Vec2Reflect(const Vec2& v, const Vec2& normal) {
    return v - 2 * Vec2DotProduct(v, normal.Normalised()) * normal.Normalised();
}

// =====================================================
// Mat3 - 3x3 Matrix 
// =====================================================
// layout indices (column-major):
// [0]=m00 [3]=m01 [6]=m02
// [1]=m10 [4]=m11 [7]=m12
// [2]=m20 [5]=m21 [8]=m22


Mat3::Mat3() {
    m[0] = 1; m[1] = 0; m[2] = 0;
    m[3] = 0; m[4] = 1; m[5] = 0;
    m[6] = 0; m[7] = 0; m[8] = 1;
}

// Per-element constructor (column-major)
Mat3::Mat3(float m00, float m01, float m02,
           float m10, float m11, float m12,
           float m20, float m21, float m22){

    m[0] = m00; m[1] = m01; m[2] = m02;
    m[3] = m10; m[4] = m11; m[5] = m12;
    m[6] = m20; m[7] = m21; m[8] = m22;
}

Mat3 Mat3::Identity() {
    return Mat3();
}


Mat3 Mat3::Translation(float tx, float ty) {
    Mat3 T = Mat3::Identity();
    T.m[6] = tx;  // last column, X
    T.m[7] = ty;  // last column, Y
    return T;
}

// Always expects RADIANS, NOT DEGREES!!!
Mat3 Mat3::Rotation(float r) {
    Mat3 R = Mat3::Identity();
    const float c = std::cos(r), s = std::sin(r);
    R.m[0] = c;  R.m[3] = -s;  // first column
    R.m[1] = s;  R.m[4] = c;  // second column
    return R;
}

Mat3 Mat3::Scale(float sx, float sy) {
    Mat3 S = Mat3::Identity();
    S.m[0] = sx;
    S.m[4] = sy;
    return S;
}

// Apply final transformation matrix to a Vec2 Point
Vec2 Mat3::TransformPoint(const Vec2& v) const {
    // Column-vector convention: result = M * [x, y, 1]
    return {
        m[0] * v.x + m[3] * v.y + m[6],
        m[1] * v.x + m[4] * v.y + m[7]
    };
}

Vec2 Mat3::TransformVector(const Vec2& v) const
{
    // Column-vector convention: result = M * [x, y, 1]
    return {
        m[0] * v.x + m[3] * v.y,
        m[1] * v.x + m[4] * v.y
    };
}

// Returns a transposed copy
Mat3 Mat3::Transposed() const {
    Mat3 t;
    t.m[0] = m[0]; t.m[1] = m[3]; t.m[2] = m[6];
    t.m[3] = m[1]; t.m[4] = m[4]; t.m[5] = m[7];
    t.m[6] = m[2]; t.m[7] = m[5]; t.m[8] = m[8];
    return t;
}

// Determinant 
float Mat3Determinant(const Mat3& mat) {
    return mat.m[0] * (mat.m[4] * mat.m[8] - mat.m[5] * mat.m[7]) -
           mat.m[3] * (mat.m[1] * mat.m[8] - mat.m[2] * mat.m[7]) +
           mat.m[6] * (mat.m[1] * mat.m[5] - mat.m[2] * mat.m[4]);
}

// Returns a inversed copy
Mat3 Mat3::Inversed(float* outDet) const {
    float c00 =  (m[4] * m[8] - m[5] * m[7]);
    float c01 = -(m[1] * m[8] - m[2] * m[7]);
    float c02 =  (m[1] * m[5] - m[2] * m[4]);

    float c10 = -(m[3] * m[8] - m[5] * m[6]);
    float c11 =  (m[0] * m[8] - m[2] * m[6]);
    float c12 = -(m[0] * m[5] - m[2] * m[3]);

    float c20 =  (m[3] * m[7] - m[4] * m[6]);
    float c21 = -(m[0] * m[7] - m[1] * m[6]);
    float c22 =  (m[0] * m[4] - m[1] * m[3]);

    float det = Mat3Determinant(*this);
    if (outDet) *outDet = det;

    if (std::fabs(det) < 1e-8f)
        return Mat3::Identity();

    float invDet = 1.0f / det;
    Mat3 inv;

    // Adjugate transpose
    inv.m[0] = c00 * invDet; inv.m[3] = c10 * invDet; inv.m[6] = c20 * invDet;
    inv.m[1] = c01 * invDet; inv.m[4] = c11 * invDet; inv.m[7] = c21 * invDet;
    inv.m[2] = c02 * invDet; inv.m[5] = c12 * invDet; inv.m[8] = c22 * invDet;

    return inv;
}

Mat3& Mat3::operator*=(const Mat3& r){ *this = (*this)*r; return *this; }

// Binary Matrix multiplicaiton
Mat3 operator*(const Mat3& lhs, const Mat3& rhs) {
    Mat3 out;
    // Column-major multiplication
    for (int col = 0; col < 3; ++col) {
        for (int row = 0; row < 3; ++row) {
            out.m[col * 3 + row] =
                lhs.m[0 * 3 + row] * rhs.m[col * 3 + 0] +
                lhs.m[1 * 3 + row] * rhs.m[col * 3 + 1] +
                lhs.m[2 * 3 + row] * rhs.m[col * 3 + 2];
        }
    }
    return out;
}

// Non-member helper functions
void Mat3Identity(Mat3& out){ out = Mat3::Identity(); }
void Mat3Translate(Mat3& out, float x, float y){ out = Mat3::Translation(x, y); }
void Mat3Scale(Mat3& out, float x, float y){ out = Mat3::Scale(x, y); }
void Mat3RotRad(Mat3& out, float angleRad){ out = Mat3::Rotation(angleRad); }
void Mat3RotDeg(Mat3& out, float angleDeg){ out = Mat3::Rotation(ToRad(angleDeg)); }
void Mat3Transpose(Mat3& out, const Mat3& in){ out = in.Transposed(); }
void Mat3Inverse(Mat3* output, float* determinant, const Mat3& input){
    if (!output){ 
        if (determinant){ 
            *determinant = 0.0f; 
        } 
        return; 
    }
    *output = input.Inversed(determinant);
}

// Matrix multiplication with Vec2
Vec2 operator*(const Mat3& lhs, const Vec2& rhs) {
    return lhs.TransformPoint(rhs);
}




// =====================================================
// Vec3
// =====================================================

Vec3::Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_){}

Vec3 Vec3::operator-() const{ return Vec3{ -x, -y, -z };}

Vec3& Vec3::operator+=(const Vec3& rhs){
    x += rhs.x; 
    y += rhs.y; 
    z += rhs.z; 
    return *this; 
}

Vec3& Vec3::operator-=(const Vec3& rhs){ 
    x -= rhs.x; 
    y -= rhs.y; 
    z -= rhs.z; 
    return *this; 
}

Vec3& Vec3::operator*=(float s){ 
    x *= s; 
    y *= s; 
    z *= s; 
    return *this; 
}

Vec3& Vec3::operator/=(float s){
    x /= s; 
    y /= s; 
    z /= s; 
    return *this; 
}

float Vec3::LengthSquared() const{ return x * x + y * y + z * z; }
float Vec3::Length() const{ return std::sqrt(LengthSquared()); }

Vec3 Vec3::Normalised() const{
    float len = Length();
    if (len <= 1e-8f) return Vec3{};
    return Vec3{ x / len, y / len, z / len };
}

void Vec3::Normalise(){
    float len = Length();
    if (len > 1e-8f) { x /= len; y /= len; z /= len;}
}

// Helpers
Vec3 operator+(const Vec3& a, const Vec3& b) { return Vec3{ a.x + b.x, a.y + b.y, a.z + b.z }; }
Vec3 operator-(const Vec3& a, const Vec3& b) { return Vec3{ a.x - b.x, a.y - b.y, a.z - b.z }; }
Vec3 operator*(const Vec3& v, float s) { return Vec3{ v.x * s, v.y * s, v.z * s }; }
Vec3 operator*(float s, const Vec3& v) { return Vec3{ s * v.x, s * v.y, s * v.z }; }
Vec3 operator/(const Vec3& v, float s) { return Vec3{ v.x / s, v.y / s, v.z / s }; }

float Vec3Dot(const Vec3& a, const Vec3& b){
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 Vec3Cross(const Vec3& a, const Vec3& b){
    return Vec3{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

Vec3 Vec3Lerp(const Vec3& a, const Vec3& b, float t){
    return a + (b - a) * t;
}



// =====================================================
// Vec4
// =====================================================

Vec4::Vec4(float x_, float y_, float z_, float w_): x(x_), y(y_), z(z_), w(w_){}

Vec4 Vec4::operator-() const{ return Vec4{ -x, -y, -z, -w };}

Vec4& Vec4::operator+=(const Vec4& rhs){ 
    x += rhs.x; 
    y += rhs.y; 
    z += rhs.z; 
    w += rhs.w; 
    return *this; 
}

Vec4& Vec4::operator-=(const Vec4& rhs){ 
    x -= rhs.x; 
    y -= rhs.y; 
    z -= rhs.z; 
    w -= rhs.w; 
    return *this; 
}

Vec4& Vec4::operator*=(float s){ 
    x *= s; 
    y *= s; 
    z *= s; 
    w *= s; 
    return *this; 
}

Vec4& Vec4::operator/=(float s){ 
    x /= s; 
    y /= s;
    z /= s; 
    w /= s; 
    return *this; 
}

// Helpers
Vec4 operator+(const Vec4& a, const Vec4& b) { return Vec4{ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w }; }
Vec4 operator-(const Vec4& a, const Vec4& b) { return Vec4{ a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w }; }
Vec4 operator*(const Vec4& v, float s) { return Vec4{ v.x * s, v.y * s, v.z * s, v.w * s }; }
Vec4 operator*(float s, const Vec4& v) { return Vec4{ s * v.x, s * v.y, s * v.z, s * v.w }; }
Vec4 operator/(const Vec4& v, float s) { return Vec4{ v.x / s, v.y / s, v.z / s, v.w / s }; }



// =====================================================
// Mat4
// =====================================================

Mat4::Mat4(){  // identity
    for (int i = 0; i < 16; ++i) m[i] = 0.f;
    m[0] = m[5] = m[10] = m[15] = 1.f;
}

Mat4::Mat4(std::initializer_list<float> list){
    int i = 0;
    for (float v : list){
        if (i < 16) m[i++] = v;
    }
}

Mat4::Mat4(float m00, float m01, float m02, float m03,
           float m10, float m11, float m12, float m13,
           float m20, float m21, float m22, float m23,
           float m30, float m31, float m32, float m33){

    m[0] = m00; m[1] = m01; m[2] = m02; m[3] = m03;
    m[4] = m10; m[5] = m11; m[6] = m12; m[7] = m13;
    m[8] = m20; m[9] = m21; m[10] = m22; m[11] = m23;
    m[12] = m30; m[13] = m31; m[14] = m32; m[15] = m33;
}

Mat4 Mat4::Identity(){ return Mat4();}

Mat4 Mat4::Translation(float x, float y, float z){
    Mat4 T = Mat4::Identity();
    T.m[12] = x;  // last column
    T.m[13] = y;
    T.m[14] = z;
    return T;
}

Mat4 Mat4::Scale(float sx, float sy, float sz){
    Mat4 S = Mat4::Identity();
    S.m[0] = sx;
    S.m[5] = sy;
    S.m[10] = sz;
    return S;
}

// only this needed since we're on 2D
Mat4 Mat4::RotationZ(float r){
    float c = std::cos(r), s = std::sin(r);
    return Mat4{
        c,-s, 0, 0,
        s, c, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
}

Vec3 Mat4::TransformPoint(const Vec3& v) const{
    return{
        m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12],
        m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13],
        m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14]
    };
}

Vec3 Mat4::TransformVector(const Vec3& v) const{
    return{
        m[0] * v.x + m[4] * v.y + m[8] * v.z,
        m[1] * v.x + m[5] * v.y + m[9] * v.z,
        m[2] * v.x + m[6] * v.y + m[10] * v.z
    };
}

Mat4 Mat4::Transposed() const{
    Mat4 t;
    for (int col = 0; col < 4; ++col)
        for (int row = 0; row < 4; ++row)
            t.m[col * 4 + row] = m[row * 4 + col];
    return t;
}

Mat4 Mat4::Inversed(float* outDet) const{

    // Extract 2X2 block (rotation+scale)
    float a = m[0], b = m[4];
    float c = m[1], d = m[5];

    // Compute determinant
    float det = a * d - b * c;

    if (outDet)
        *outDet = det;

    // If singular, return identity
    if (fabs(det) < 1e-8f)
        return Mat4::Identity();

    float invDet = 1.0f / det;

    Mat4 inv; // default constructor makes identity

    // Invert the 2X2 block
    inv.m[0] = d * invDet;
    inv.m[4] = -b * invDet;
    inv.m[1] = -c * invDet;
    inv.m[5] = a * invDet;

    // Translation terms
    float tx = m[12];
    float ty = m[13];

    inv.m[12] = -(inv.m[0] * tx + inv.m[4] * ty);
    inv.m[13] = -(inv.m[1] * tx + inv.m[5] * ty);

    // Preserve Z row/column as identity for 2D engine
    inv.m[10] = 1.0f / m[10];
    inv.m[14] = -m[14] * inv.m[10];
    inv.m[15] = 1.0f;

    return inv;
}


Mat4& Mat4::operator*=(const Mat4& rhs){
    *this = (*this) * rhs;
    return *this;
}

Mat4 Mat4::Ortho(float left, float right, float bottom, float top, float nearZ, float farZ){

    Mat4 M;
    float rl = right - left;
    float tb = top - bottom;
    float fn = farZ - nearZ;

    M.m[0] = 2.0f / rl;
    M.m[5] = 2.0f / tb;
    M.m[10] = -2.0f / fn;

    M.m[12] = -(right + left) / rl;
    M.m[13] = -(top + bottom) / tb;
    M.m[14] = -(farZ + nearZ) / fn;

    M.m[15] = 1.0f;

    return M;
}

Mat4 Mat4::TRS(const Vec3& position, float rotationZ, const Vec3& scale){

    float c = std::cos(rotationZ);
    float s = std::sin(rotationZ);
    Mat4 M;

    // Rotation * Scale (2X2 block)
    M.m[0] = scale.x * c;
    M.m[1] = scale.x * s;
    M.m[2] = 0.0f;
    M.m[3] = 0.0f;

    M.m[4] = scale.y * -s;
    M.m[5] = scale.y * c;
    M.m[6] = 0.0f;
    M.m[7] = 0.0f;

    // Z scale
    M.m[8] = 0.0f;
    M.m[9] = 0.0f;
    M.m[10] = scale.z;
    M.m[11] = 0.0f;

    // Translation
    M.m[12] = position.x;
    M.m[13] = position.y;
    M.m[14] = position.z;
    M.m[15] = 1.0f;

    return M;
}

void Mat4::Decompose2D(Vec2& outPos, float& outRotZ, Vec2& outScale) const{
    // Translation from last column
    outPos.x = m[12];
    outPos.y = m[13];

    // Scale = length of basis vectors
    outScale.x = std::sqrt(m[0] * m[0] + m[1] * m[1]);
    outScale.y = std::sqrt(m[4] * m[4] + m[5] * m[5]);

    // Rotation = atan2 of normalized basis
    float rot = std::atan2(m[1] / outScale.x, m[0] / outScale.x);
    outRotZ = rot;
}

Vec2 Mat4::TransformNormal2D(const Vec2& v) const{
    float a = m[0], b = m[4];
    float c = m[1], d = m[5];

    return Vec2{
        a * v.x + b * v.y,
        c * v.x + d * v.y
    };
}

Mat4 operator*(const Mat4& a, const Mat4& b){
    Mat4 r;
    for (int col = 0; col < 4; ++col){
        for (int row = 0; row < 4; ++row){
            r.m[col * 4 + row] =
                a.m[0 * 4 + row] * b.m[col * 4 + 0] +
                a.m[1 * 4 + row] * b.m[col * 4 + 1] +
                a.m[2 * 4 + row] * b.m[col * 4 + 2] +
                a.m[3 * 4 + row] * b.m[col * 4 + 3];
        }
    }
    return r;
}

Vec4 operator*(const Mat4& m, const Vec4& v){
    return {
        m.m[0] * v.x + m.m[4] * v.y + m.m[8] * v.z + m.m[12] * v.w,
        m.m[1] * v.x + m.m[5] * v.y + m.m[9] * v.z + m.m[13] * v.w,
        m.m[2] * v.x + m.m[6] * v.y + m.m[10] * v.z + m.m[14] * v.w,
        m.m[3] * v.x + m.m[7] * v.y + m.m[11] * v.z + m.m[15] * v.w
    };
}

Vec3 operator*(const Mat4& m, const Vec3& v){
    Vec4 r = m * Vec4{ v.x, v.y, v.z, 1.f };
    return { r.x, r.y, r.z };
}

float Mat4Determinant(const Mat4& m){
    // 2X2 determinant from rotation+scale block:
    // | a  b |
    // | c  d |
    //
    // Stored in:
    // a = m[0], b = m[4]
    // c = m[1], d = m[5]

    return m.m[0] * m.m[5] - m.m[1] * m.m[4];
}




// Optional test for MathLib
void RunMathLibTests() {

    std::cout << "============================\n";
    std::cout << "==== Vec2 Tests ============\n";
    std::cout << "============================\n";

    Vec2 a(3.0f, 4.0f);
    Vec2 b(1.0f, 2.0f);

    std::cout << "a = (3,4), b = (1,2)\n";
    std::cout << "Length(a) = " << a.Length() << " (Expected: 5)\n";

    Vec2 normA = a.Normalised();
    std::cout << "Normalised a = (" << normA.x << ", " << normA.y
        << ") (Expected: (0.6, 0.8))\n";

    std::cout << "a + b = (" << (a + b).x << ", " << (a + b).y
        << ") (Expected: (4, 6))\n";

    std::cout << "a - b = (" << (a - b).x << ", " << (a - b).y
        << ") (Expected: (2, 2))\n";

    std::cout << "a * 2 = (" << (a * 2.f).x << ", " << (a * 2.f).y
        << ") (Expected: (6, 8))\n";

    std::cout << "a / 2 = (" << (a / 2.f).x << ", " << (a / 2.f).y
        << ") (Expected: (1.5, 2))\n";

    float dist = Vec2Distance(a, b);
    std::cout << "Distance(a,b) = " << dist
        << " (Expected: ~2.82843)\n";

    std::cout << "Dot(a,b) = " << Vec2DotProduct(a, b)
        << " (Expected: 11)\n";

    std::cout << "Cross(a,b) = " << Vec2CrossProduct(a, b)
        << " (Expected: 2)\n";

    Vec2 c(0, 1), d(1, 0);
    std::cout << "Angle(c,d) = " << ToDeg(Vec2Angle(c, d))
        << " deg (Expected: 90)\n";

    Vec2 proj = Vec2Project(a, b);
    std::cout << "Project(a onto b) = (" << proj.x << ", " << proj.y
        << ") (Expected: (2.2, 4.4))\n";



    std::cout << "\n============================\n";
    std::cout << "==== Mat3 Tests ============\n";
    std::cout << "============================\n";

    Mat3 I = Mat3::Identity();
    std::cout << "Identity det = " << Mat3Determinant(I)
        << " (Expected: 1)\n";

    Mat3 T = Mat3::Translation(5, 10);
    Vec2 translated = T * Vec2(1, 1);
    std::cout << "Translate (1,1) -> (" << translated.x << ", " << translated.y
        << ") (Expected: (6, 11))\n";

    Mat3 S = Mat3::Scale(2, 3);
    Vec2 scaled = S * Vec2(1, 1);
    std::cout << "Scale (1,1) -> (" << scaled.x << ", " << scaled.y
        << ") (Expected: (2, 3))\n";

    Mat3 R = Mat3::Rotation(ToRad(90));
    Vec2 rotated = R * Vec2(1, 0);
    std::cout << "Rotate (1,0) 90deg -> (" << rotated.x << ", " << rotated.y
        << ") (Expected: (0, 1))\n";

    Mat3 combo = S * R * T;
    Vec2 comboRes = combo * Vec2(1, 1);

    float det3;
    Mat3 inv3 = combo.Inversed(&det3);
    Vec2 comboBack = inv3 * comboRes;
    std::cout << "Inverse(combo) recovers -> (" << comboBack.x << ", " << comboBack.y
        << ") (Expected ~ (1,1))\n";



    std::cout << "\n============================\n";
    std::cout << "==== Vec3 Tests ============\n";
    std::cout << "============================\n";

    Vec3 v3a(3, 4, 0);
    Vec3 v3b(1, 2, 3);

    std::cout << "Length(v3a) = " << v3a.Length()
        << " (Expected: 5)\n";

    Vec3 v3norm = v3a.Normalised();
    std::cout << "Normalised v3a = (" << v3norm.x << ", "
        << v3norm.y << ", " << v3norm.z
        << ") (Expected: (0.6, 0.8, 0))\n";

    Vec3 v3add = v3a + v3b;
    std::cout << "v3a + v3b = (" << v3add.x << ", " << v3add.y << ", " << v3add.z
        << ") (Expected: (4, 6, 3))\n";

    std::cout << "Dot(v3a,v3b) = " << Vec3Dot(v3a, v3b)
        << " (Expected: 11)\n";

    Vec3 v3cross = Vec3Cross(v3a, v3b);
    std::cout << "Cross(v3a,v3b) = ("
        << v3cross.x << ", " << v3cross.y << ", " << v3cross.z
        << ") (Expected: (12, -9, 2))\n";



    std::cout << "\n============================\n";
    std::cout << "==== Vec4 Tests ============\n";
    std::cout << "============================\n";

    Vec4 v4a(1, 2, 3, 4);
    Vec4 v4b(4, 3, 2, 1);

    Vec4 v4add = v4a + v4b;
    std::cout << "v4a + v4b = (" << v4add.x << ", " << v4add.y << ", "
        << v4add.z << ", " << v4add.w
        << ") (Expected: (5, 5, 5, 5))\n";

    Vec4 v4scaled = v4a * 2.f;
    std::cout << "v4a * 2 = (" << v4scaled.x << ", " << v4scaled.y << ", "
        << v4scaled.z << ", " << v4scaled.w
        << ") (Expected: (2, 4, 6, 8))\n";



    std::cout << "\n============================\n";
    std::cout << "==== Mat4 Tests ============\n";
    std::cout << "============================\n";

    Mat4 M4I = Mat4::Identity();
    std::cout << "Mat4 Identity diag = "
        << M4I.m[0] << ", " << M4I.m[5] << ", "
        << M4I.m[10] << ", " << M4I.m[15]
        << " (Expected: 1,1,1,1)\n";

    Mat4 M4T = Mat4::Translation(5, 10, 0);
    Vec3 resultT = M4T.TransformPoint(Vec3(1, 1, 0));
    std::cout << "Translate (1,1,0) -> (" << resultT.x << ", " << resultT.y
        << ", " << resultT.z << ") (Expected: (6, 11, 0))\n";

    Mat4 M4S = Mat4::Scale(2, 3, 1);
    Vec3 resultS = M4S.TransformPoint(Vec3(1, 1, 0));
    std::cout << "Scale (1,1,0) -> (" << resultS.x << ", " << resultS.y
        << ", " << resultS.z << ") (Expected: (2, 3, 0))\n";

    Mat4 M4R = Mat4::RotationZ(ToRad(90));
    Vec3 resultR = M4R.TransformPoint(Vec3(1, 0, 0));
    std::cout << "RotZ(90deg) (1,0) -> (" << resultR.x << ", " << resultR.y
        << ") (Expected: (0, 1))\n";

    // TRS
    Mat4 M4trs = Mat4::TRS(Vec3(5, 10, 0), ToRad(90), Vec3(2, 2, 1));
    Vec3 resTRS = M4trs.TransformPoint(Vec3(1, 0, 0));
    std::cout << "TRS pos=(5,10), rot=90 deg, scale=(2,2), point=(1,0)\n";
    std::cout << "Result -> (" << resTRS.x << ", " << resTRS.y << ", " << resTRS.z
        << ") (Expected: (5, 12, 0))\n";

    // Ortho
    Mat4 M4ortho = Mat4::Ortho(-10, 10, -10, 10, -1, 1);
    std::cout << "Ortho m[0],m[5] = (" << M4ortho.m[0] << ", " << M4ortho.m[5]
        << ") (Expected: (0.1, 0.1))\n";

    // Mat4 * Mat4
    Mat4 M4combo = M4R * M4T;
    Vec3 comboTest = M4combo.TransformPoint(Vec3(1, 1, 0));
    std::cout << "(RotZ * Translate)(1,1) -> (" << comboTest.x << ", " << comboTest.y
        << ") (Expected: (-11, 6))\n";

    // Inverse(TRS)
    float det4 = 0.f;
    Mat4 M4inv = M4trs.Inversed(&det4);
    Vec3 recovered4 = M4inv.TransformPoint(resTRS);
    std::cout << "Inverse(TRS) recovers -> (" << recovered4.x << ", " << recovered4.y
        << ", " << recovered4.z << ") (Expected: (1, 0, 0))\n";

    // Decompose2D
    Vec2 dpos, dscale;
    float drot;
    M4trs.Decompose2D(dpos, drot, dscale);

    std::cout << "Decompose2D pos = (" << dpos.x << ", " << dpos.y
        << ") (Expected: (5, 10))\n";

    std::cout << "Decompose2D rotDeg = " << ToDeg(drot)
        << " (Expected: 90)\n";

    std::cout << "Decompose2D scale = (" << dscale.x << ", " << dscale.y
        << ") (Expected: (2, 2))\n";


    std::cout << "\n==== End of MathLib Tests ====\n\n";
}


