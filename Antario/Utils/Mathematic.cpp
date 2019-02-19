#include "Mathematic.h"

Vector Math::VectorTransform(Vector vector, matrix3x4_t matrix)
{
	return Vector(vector.Dot2(matrix[0]) + matrix[0][3], vector.Dot2(matrix[1]) + matrix[1][3], vector.Dot2(matrix[2]) + matrix[2][3]);
}

void Math::VectorTransform_2(const Vector& in1, const matrix3x4_t& in2, Vector& out)
{
	auto dot_product = [](const Vector& a, const Vector& b)
	{
		return (a.x * b.x + a.y * b.y + a.z * b.z);
	};

	out[0] = dot_product(in1, Vector(in2[0][0], in2[0][1], in2[0][2])) + in2[0][3];
	out[1] = dot_product(in1, Vector(in2[1][0], in2[1][1], in2[1][2])) + in2[1][3];
	out[2] = dot_product(in1, Vector(in2[2][0], in2[2][1], in2[2][2])) + in2[2][3];
}

void Math::RotateTriangle(std::array<Vector2D, 3>& points, float rotation)
{
	const auto points_center = (points.at(0) + points.at(1) + points.at(2)) / 3;
	for (auto& point : points)
	{
		point -= points_center;

		const auto temp_x = point.x;
		const auto temp_y = point.y;

		const auto theta = deg_2_rad(rotation);
		const auto c = cosf(theta);
		const auto s = sinf(theta);

		point.x = temp_x * c - temp_y * s;
		point.y = temp_x * s + temp_y * c;

		point += points_center;
	}
}

void Math::VectorAngles_2(const Vector& forward, Vector& angles)
{
	if (forward.x == 0.f && forward.y == 0.f)
	{
		angles.x = forward.z > 0.f ? -90.f : 90.f;
		angles.y = 0.f;
	}
	else
	{
		angles.x = rad_2_deg(atan2(-forward.z, forward.Length2D()));
		angles.y = rad_2_deg(atan2(forward.y, forward.x));
	}

	angles.z = 0.f;
}

void inline SinCos(float radians, float* sine, float* cosine)
{
	*sine = sin(radians);
	*cosine = cos(radians);
}

void Math::AngleVectors(const Vector& angles, Vector* forward, Vector* right, Vector* up)
{
	float sr, sp, sy, cr, cp, cy;
	SinCos(DEG2RAD(angles[1]), &sy, &cy);
	SinCos(DEG2RAD(angles[0]), &sp, &cp);
	SinCos(DEG2RAD(angles[2]), &sr, &cr);

	if (forward)
	{
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}
	if (right)
	{
		right->x = (-1 * sr * sp * cy + -1 * cr * -sy);
		right->y = (-1 * sr * sp * sy + -1 * cr * cy);
		right->z = -1 * sr * cp;
	}
	if (up)
	{
		up->x = (cr * sp * cy + -sr * -sy);
		up->y = (cr * sp * sy + -sr * cy);
		up->z = cr * cp;
	}
}

float Math::DistanceTo(Vector vecDst, Vector vecSrc)
{
	Vector vDelta = vecDst - vecSrc;

	float fDistance = ::sqrtf((vDelta.Length()));

	//if (fDistance < 1.0f)
	//	return 1.0f;

	return fDistance;
}

Vector Math::NormalizeAngle(Vector angle) {
	while (angle.x > 89.f) {
		angle.x -= 180.f;
	}
	while (angle.x < -89.f) {
		angle.x += 180.f;
	}
	if (angle.y > 180) {
		angle.y -= (round(angle.y / 360) * 360.f);
	}
	else if (angle.y < -180) {
		angle.y += (round(angle.y / 360) * -360.f);
	}
	if ((angle.z > 50) || (angle.z < 50)) {
		angle.z = 0;
	}
	return angle;
}

Vector Math::CalcAngle(Vector src, Vector dst) {
	Vector angles;
	Vector delta = src - dst;
	g_Math.VectorAngles_2(delta, angles);
	delta.Normalize();
	return angles;
}
void Math::CalcAngle(Vector src, Vector dst, Vector &angles) {
	Vector delta = src - dst;
	double hyp = delta.Length2D();
	angles.y = (atan(delta.y / delta.x) * 57.295779513082f);
	angles.x = (vec_t)(atan(delta.z / hyp) * 57.295779513082f);
	angles[2] = 0.00;

	if (delta.x >= 0.0)
		angles.y += 180.0f;
}


Math g_Math;