#pragma once
#include "../SDK/Vector.h"
#include "../SDK/VMatrix.h"
#include "../Utils/SPoint.h"
#include <array>
#define PI 3.14159265358979323846f

class Math
{
public:
	static Vector VectorTransform(Vector vector, matrix3x4_t matrix);
	static void VectorTransform_2(const Vector & in1, const matrix3x4_t & in2, Vector & out);

	void RotateTriangle(std::array<Vector2D, 3>& points, float rotation);

	void VectorAngles_2(const Vector & forward, Vector & angles);

	void AngleVectors(const Vector& angles, Vector* forward = nullptr, Vector* right = nullptr, Vector* up = nullptr);

	float DistanceTo(Vector vecDst, Vector vecSrc);

	Vector NormalizeAngle(Vector angle);

	Vector CalcAngle(Vector src, Vector dst);

	void CalcAngle(Vector src, Vector dst, Vector & angles);

	static constexpr float Math::rad_2_deg(float radian)
	{
		return radian * (180.f / PI);
	}

	static constexpr float Math::deg_2_rad(float degree)
	{
		return degree * (PI / 180.f);
	}
};

extern Math g_Math;
