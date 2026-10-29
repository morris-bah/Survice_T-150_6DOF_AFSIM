// ****************************************************************************
// Rotorcraft 6DOF Mover - Math Utilities
//
// Quaternion, vector, and matrix operations for 6DOF dynamics.
// Uses right-handed NED (North-East-Down) coordinate convention.
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#ifndef ROTORCRAFT_6DOF_MATH_UTILS_HPP
#define ROTORCRAFT_6DOF_MATH_UTILS_HPP

#include <cmath>
#include <array>
#include <algorithm>

namespace rotorcraft_6dof
{

// ============================================================================
// Constants
// ============================================================================

constexpr double PI = 3.14159265358979323846;
constexpr double DEG_TO_RAD = PI / 180.0;
constexpr double RAD_TO_DEG = 180.0 / PI;
constexpr double GRAVITY = 9.80665;  // m/s^2
constexpr double AIR_DENSITY_SL = 1.225;  // kg/m^3 at sea level

// ============================================================================
// Vector3 - 3D vector operations
// ============================================================================

struct Vector3
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    Vector3() = default;
    Vector3(double ax, double ay, double az) : x(ax), y(ay), z(az) {}

    // Array access
    double& operator[](int i) { return (&x)[i]; }
    double operator[](int i) const { return (&x)[i]; }

    // Vector operations
    Vector3 operator+(const Vector3& v) const { return {x + v.x, y + v.y, z + v.z}; }
    Vector3 operator-(const Vector3& v) const { return {x - v.x, y - v.y, z - v.z}; }
    Vector3 operator*(double s) const { return {x * s, y * s, z * s}; }
    Vector3 operator/(double s) const { return {x / s, y / s, z / s}; }
    Vector3& operator+=(const Vector3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    Vector3& operator-=(const Vector3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    Vector3& operator*=(double s) { x *= s; y *= s; z *= s; return *this; }

    // Magnitude
    double Magnitude() const { return std::sqrt(x*x + y*y + z*z); }
    double MagnitudeSquared() const { return x*x + y*y + z*z; }

    // Normalize
    Vector3 Normalized() const
    {
        double mag = Magnitude();
        if (mag > 1e-10) return *this / mag;
        return {0, 0, 0};
    }

    // Dot product
    double Dot(const Vector3& v) const { return x*v.x + y*v.y + z*v.z; }

    // Cross product
    Vector3 Cross(const Vector3& v) const
    {
        return {
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        };
    }

    // Zero vector
    static Vector3 Zero() { return {0, 0, 0}; }

    // Unit vectors
    static Vector3 UnitX() { return {1, 0, 0}; }
    static Vector3 UnitY() { return {0, 1, 0}; }
    static Vector3 UnitZ() { return {0, 0, 1}; }
};

// Scalar * Vector
inline Vector3 operator*(double s, const Vector3& v) { return v * s; }

// ============================================================================
// Quaternion - Unit quaternion for rotation representation
// Convention: q = [w, x, y, z] where w is the scalar part
// ============================================================================

struct Quaternion
{
    double w = 1.0;  // Scalar part
    double x = 0.0;  // Vector part
    double y = 0.0;
    double z = 0.0;

    Quaternion() = default;
    Quaternion(double aw, double ax, double ay, double az) : w(aw), x(ax), y(ay), z(az) {}

    // Identity quaternion (no rotation)
    static Quaternion Identity() { return {1, 0, 0, 0}; }

    // Create from Euler angles (roll, pitch, yaw in radians)
    // Convention: ZYX rotation order (yaw-pitch-roll)
    static Quaternion FromEuler(double roll, double pitch, double yaw)
    {
        double cr = std::cos(roll * 0.5);
        double sr = std::sin(roll * 0.5);
        double cp = std::cos(pitch * 0.5);
        double sp = std::sin(pitch * 0.5);
        double cy = std::cos(yaw * 0.5);
        double sy = std::sin(yaw * 0.5);

        return {
            cr * cp * cy + sr * sp * sy,  // w
            sr * cp * cy - cr * sp * sy,  // x
            cr * sp * cy + sr * cp * sy,  // y
            cr * cp * sy - sr * sp * cy   // z
        };
    }

    // Convert to Euler angles (roll, pitch, yaw in radians)
    void ToEuler(double& roll, double& pitch, double& yaw) const
    {
        // Roll (x-axis rotation)
        double sinr_cosp = 2.0 * (w * x + y * z);
        double cosr_cosp = 1.0 - 2.0 * (x * x + y * y);
        roll = std::atan2(sinr_cosp, cosr_cosp);

        // Pitch (y-axis rotation)
        double sinp = 2.0 * (w * y - z * x);
        if (std::abs(sinp) >= 1.0)
            pitch = std::copysign(PI / 2.0, sinp);  // Gimbal lock
        else
            pitch = std::asin(sinp);

        // Yaw (z-axis rotation)
        double siny_cosp = 2.0 * (w * z + x * y);
        double cosy_cosp = 1.0 - 2.0 * (y * y + z * z);
        yaw = std::atan2(siny_cosp, cosy_cosp);
    }

    // Quaternion multiplication (rotation composition)
    Quaternion operator*(const Quaternion& q) const
    {
        return {
            w * q.w - x * q.x - y * q.y - z * q.z,
            w * q.x + x * q.w + y * q.z - z * q.y,
            w * q.y - x * q.z + y * q.w + z * q.x,
            w * q.z + x * q.y - y * q.x + z * q.w
        };
    }

    // Quaternion-vector multiplication (for derivative calculation)
    // Returns quaternion representing q * [0, v.x, v.y, v.z]
    Quaternion operator*(const Vector3& v) const
    {
        return {
            -x * v.x - y * v.y - z * v.z,
             w * v.x + y * v.z - z * v.y,
             w * v.y - x * v.z + z * v.x,
             w * v.z + x * v.y - y * v.x
        };
    }

    // Scalar multiplication
    Quaternion operator*(double s) const { return {w * s, x * s, y * s, z * s}; }
    Quaternion operator+(const Quaternion& q) const { return {w + q.w, x + q.x, y + q.y, z + q.z}; }

    // Conjugate (inverse for unit quaternion)
    Quaternion Conjugate() const { return {w, -x, -y, -z}; }

    // Magnitude
    double Magnitude() const { return std::sqrt(w*w + x*x + y*y + z*z); }

    // Normalize to unit quaternion
    Quaternion Normalized() const
    {
        double mag = Magnitude();
        if (mag > 1e-10)
            return {w / mag, x / mag, y / mag, z / mag};
        return Identity();
    }

    // Rotate a vector by this quaternion
    // v' = q * v * q^(-1)
    Vector3 RotateVector(const Vector3& v) const
    {
        // Optimized rotation formula
        Vector3 qv(x, y, z);
        Vector3 uv = qv.Cross(v);
        Vector3 uuv = qv.Cross(uv);
        return v + (uv * w + uuv) * 2.0;
    }

    // Inverse rotation (rotate by conjugate)
    Vector3 RotateVectorInverse(const Vector3& v) const
    {
        return Conjugate().RotateVector(v);
    }
};

inline Quaternion operator*(double s, const Quaternion& q) { return q * s; }

// ============================================================================
// Matrix3x3 - 3x3 matrix for inertia tensor and rotations
// ============================================================================

struct Matrix3x3
{
    std::array<std::array<double, 3>, 3> m = {{{1,0,0}, {0,1,0}, {0,0,1}}};

    Matrix3x3() = default;

    // Access
    double& operator()(int row, int col) { return m[row][col]; }
    double operator()(int row, int col) const { return m[row][col]; }

    // Identity matrix
    static Matrix3x3 Identity()
    {
        Matrix3x3 result;
        result.m = {{{1,0,0}, {0,1,0}, {0,0,1}}};
        return result;
    }

    // Diagonal matrix (for principal inertia tensor)
    static Matrix3x3 Diagonal(double xx, double yy, double zz)
    {
        Matrix3x3 result;
        result.m = {{{xx,0,0}, {0,yy,0}, {0,0,zz}}};
        return result;
    }

    // Matrix-vector multiplication
    Vector3 operator*(const Vector3& v) const
    {
        return {
            m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
            m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
            m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z
        };
    }

    // Inverse (for diagonal matrix - simplified)
    Matrix3x3 InverseDiagonal() const
    {
        Matrix3x3 result;
        result.m[0][0] = (std::abs(m[0][0]) > 1e-10) ? 1.0 / m[0][0] : 0.0;
        result.m[1][1] = (std::abs(m[1][1]) > 1e-10) ? 1.0 / m[1][1] : 0.0;
        result.m[2][2] = (std::abs(m[2][2]) > 1e-10) ? 1.0 / m[2][2] : 0.0;
        return result;
    }
};

// ============================================================================
// Utility functions
// ============================================================================

// Clamp value to range
template<typename T>
inline T Clamp(T value, T minVal, T maxVal)
{
    return std::max(minVal, std::min(value, maxVal));
}

// Linear interpolation
inline double Lerp(double a, double b, double t)
{
    return a + t * (b - a);
}

// Sign function
inline double Sign(double x)
{
    if (x > 0) return 1.0;
    if (x < 0) return -1.0;
    return 0.0;
}

// Air density as function of altitude (ISA model)
inline double AirDensity(double altitudeMSL)
{
    // Simple ISA model up to 11km (troposphere)
    constexpr double T0 = 288.15;      // Sea level temp (K)
    constexpr double L = 0.0065;       // Lapse rate (K/m)
    constexpr double R = 287.05;       // Gas constant
    constexpr double g = 9.80665;
    constexpr double M = 0.0289644;    // Molar mass

    if (altitudeMSL < 0) altitudeMSL = 0;
    if (altitudeMSL > 11000) altitudeMSL = 11000;  // Troposphere limit

    double T = T0 - L * altitudeMSL;
    double exponent = g / (R * L);
    double rho = AIR_DENSITY_SL * std::pow(T / T0, exponent - 1.0);

    return rho;
}

// ============================================================================
// Catmull-Rom Spline Functions
// ============================================================================
// Catmull-Rom splines provide smooth C1-continuous curves through control points.
// Given 4 control points P0, P1, P2, P3, the spline interpolates from P1 to P2
// as parameter t goes from 0 to 1.
// ============================================================================

//! Catmull-Rom spline interpolation for a single dimension
//! @param p0, p1, p2, p3 Control point values
//! @param t Parameter [0,1] - interpolates from p1 to p2
//! @param tension Spline tension [0,1], 0.5 is standard Catmull-Rom
//! @return Interpolated value
inline double CatmullRomValue(double p0, double p1, double p2, double p3,
                               double t, double tension = 0.5)
{
    double t2 = t * t;
    double t3 = t2 * t;

    // Tangent scale factor
    double s = (1.0 - tension) / 2.0;

    // Hermite basis functions
    double h1 = 2.0 * t3 - 3.0 * t2 + 1.0;   // H00
    double h2 = -2.0 * t3 + 3.0 * t2;         // H01
    double h3 = t3 - 2.0 * t2 + t;            // H10
    double h4 = t3 - t2;                       // H11

    // Tangents at P1 and P2
    double m1 = s * (p2 - p0);  // Tangent at P1
    double m2 = s * (p3 - p1);  // Tangent at P2

    return h1 * p1 + h2 * p2 + h3 * m1 + h4 * m2;
}

//! Catmull-Rom spline interpolation for 3D vector
//! @param p0, p1, p2, p3 Control points
//! @param t Parameter [0,1] - interpolates from p1 to p2
//! @param tension Spline tension [0,1]
//! @return Interpolated position
inline Vector3 CatmullRomPoint(const Vector3& p0, const Vector3& p1,
                                const Vector3& p2, const Vector3& p3,
                                double t, double tension = 0.5)
{
    return Vector3(
        CatmullRomValue(p0.x, p1.x, p2.x, p3.x, t, tension),
        CatmullRomValue(p0.y, p1.y, p2.y, p3.y, t, tension),
        CatmullRomValue(p0.z, p1.z, p2.z, p3.z, t, tension)
    );
}

//! Catmull-Rom spline tangent (derivative) for a single dimension
//! @param p0, p1, p2, p3 Control point values
//! @param t Parameter [0,1]
//! @param tension Spline tension [0,1]
//! @return Derivative at t
inline double CatmullRomDerivative(double p0, double p1, double p2, double p3,
                                    double t, double tension = 0.5)
{
    double t2 = t * t;

    // Tangent scale factor
    double s = (1.0 - tension) / 2.0;

    // Derivatives of Hermite basis functions
    double dh1 = 6.0 * t2 - 6.0 * t;          // dH00/dt
    double dh2 = -6.0 * t2 + 6.0 * t;         // dH01/dt
    double dh3 = 3.0 * t2 - 4.0 * t + 1.0;    // dH10/dt
    double dh4 = 3.0 * t2 - 2.0 * t;          // dH11/dt

    // Tangents at P1 and P2
    double m1 = s * (p2 - p0);
    double m2 = s * (p3 - p1);

    return dh1 * p1 + dh2 * p2 + dh3 * m1 + dh4 * m2;
}

//! Catmull-Rom spline tangent for 3D vector
//! @param p0, p1, p2, p3 Control points
//! @param t Parameter [0,1]
//! @param tension Spline tension [0,1]
//! @return Tangent vector (unnormalized)
inline Vector3 CatmullRomTangent(const Vector3& p0, const Vector3& p1,
                                  const Vector3& p2, const Vector3& p3,
                                  double t, double tension = 0.5)
{
    return Vector3(
        CatmullRomDerivative(p0.x, p1.x, p2.x, p3.x, t, tension),
        CatmullRomDerivative(p0.y, p1.y, p2.y, p3.y, t, tension),
        CatmullRomDerivative(p0.z, p1.z, p2.z, p3.z, t, tension)
    );
}

//! Calculate approximate arc length of a Catmull-Rom spline segment
//! Uses adaptive Simpson's rule for accuracy
//! @param p0, p1, p2, p3 Control points
//! @param tension Spline tension [0,1]
//! @param numSegments Number of line segments for approximation (default 32)
//! @return Arc length
inline double CatmullRomArcLength(const Vector3& p0, const Vector3& p1,
                                   const Vector3& p2, const Vector3& p3,
                                   double tension = 0.5, int numSegments = 32)
{
    double length = 0.0;
    double dt = 1.0 / numSegments;

    Vector3 prevPoint = p1;  // t=0 -> p1
    for (int i = 1; i <= numSegments; ++i)
    {
        double t = i * dt;
        Vector3 currentPoint = CatmullRomPoint(p0, p1, p2, p3, t, tension);
        length += (currentPoint - prevPoint).Magnitude();
        prevPoint = currentPoint;
    }

    return length;
}

//! Convert arc distance to spline parameter using binary search
//! @param p0, p1, p2, p3 Control points
//! @param targetDistance Target arc distance from P1
//! @param totalLength Total arc length of segment
//! @param tension Spline tension
//! @param tolerance Convergence tolerance
//! @return Parameter t such that arc length from 0 to t equals targetDistance
inline double ArcDistanceToSplineParameter(const Vector3& p0, const Vector3& p1,
                                            const Vector3& p2, const Vector3& p3,
                                            double targetDistance, double totalLength,
                                            double tension = 0.5, double tolerance = 0.1)
{
    if (targetDistance <= 0.0) return 0.0;
    if (targetDistance >= totalLength) return 1.0;

    // Initial guess using linear approximation
    double t = targetDistance / totalLength;

    // Binary search refinement
    double tLow = 0.0;
    double tHigh = 1.0;

    for (int iter = 0; iter < 20; ++iter)
    {
        // Calculate arc length up to t
        double currentLength = CatmullRomArcLength(p0, p1, p2, p3, tension, 16) * t;

        // More accurate: sum up segments to t
        double actualLength = 0.0;
        int numSegs = 16;
        double dt = t / numSegs;
        Vector3 prevPoint = p1;
        for (int i = 1; i <= numSegs; ++i)
        {
            double ti = i * dt;
            Vector3 point = CatmullRomPoint(p0, p1, p2, p3, ti, tension);
            actualLength += (point - prevPoint).Magnitude();
            prevPoint = point;
        }

        double error = actualLength - targetDistance;

        if (std::abs(error) < tolerance)
            return t;

        if (error > 0)
            tHigh = t;
        else
            tLow = t;

        t = (tLow + tHigh) * 0.5;
    }

    return t;
}

//! Calculate arc distance from spline start (P1) to parameter t
inline double SplineParameterToArcDistance(const Vector3& p0, const Vector3& p1,
                                            const Vector3& p2, const Vector3& p3,
                                            double t, double tension = 0.5)
{
    if (t <= 0.0) return 0.0;
    if (t >= 1.0) return CatmullRomArcLength(p0, p1, p2, p3, tension, 32);

    // Sum arc lengths of segments up to t
    int numSegs = static_cast<int>(t * 32) + 1;
    double dt = t / numSegs;
    double length = 0.0;

    Vector3 prevPoint = p1;
    for (int i = 1; i <= numSegs; ++i)
    {
        double ti = i * dt;
        Vector3 point = CatmullRomPoint(p0, p1, p2, p3, ti, tension);
        length += (point - prevPoint).Magnitude();
        prevPoint = point;
    }

    return length;
}

} // namespace rotorcraft_6dof

#endif // ROTORCRAFT_6DOF_MATH_UTILS_HPP
