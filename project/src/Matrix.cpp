#include "Matrix.h"

#include <cassert>

#include <cmath>

Matrix::Matrix(const glm::vec3& xAxis, const glm::vec3& yAxis, const glm::vec3& zAxis, const glm::vec3& t) :
	Matrix({ xAxis, 0 }, { yAxis, 0 }, { zAxis, 0 }, { t, 1 })
{
}

Matrix::Matrix(const glm::vec4& xAxis, const glm::vec4& yAxis, const glm::vec4& zAxis, const glm::vec4& t)
{
	data[0] = xAxis;
	data[1] = yAxis;
	data[2] = zAxis;
	data[3] = t;
}

Matrix::Matrix(const Matrix& m)
{
	data[0] = m[0];
	data[1] = m[1];
	data[2] = m[2];
	data[3] = m[3];
}

glm::vec3 Matrix::TransformVector(const glm::vec3& v) const
{
	return TransformVector(v.x, v.y, v.z);
}

glm::vec3 Matrix::TransformVector(float x, float y, float z) const
{
	return glm::vec3{
		data[0].x * x + data[1].x * y + data[2].x * z,
		data[0].y * x + data[1].y * y + data[2].y * z,
		data[0].z * x + data[1].z * y + data[2].z * z
	};
}

glm::vec3 Matrix::TransformPoint(const glm::vec3& p) const
{
	return TransformPoint(p.x, p.y, p.z);
}

glm::vec3 Matrix::TransformPoint(float x, float y, float z) const
{
	return glm::vec3{
		data[0].x * x + data[1].x * y + data[2].x * z + data[3].x,
		data[0].y * x + data[1].y * y + data[2].y * z + data[3].y,
		data[0].z * x + data[1].z * y + data[2].z * z + data[3].z,
	};
}

glm::vec4 Matrix::TransformPoint(const glm::vec4& p) const
{
	return TransformPoint(p.x, p.y, p.z, p.w);
}

glm::vec4 Matrix::TransformPoint(float x, float y, float z, float w) const
{
	return glm::vec4{
		data[0].x * x + data[1].x * y + data[2].x * z + data[3].x,
		data[0].y * x + data[1].y * y + data[2].y * z + data[3].y,
		data[0].z * x + data[1].z * y + data[2].z * z + data[3].z,
		data[0].w * x + data[1].w * y + data[2].w * z + data[3].w
	};
}

const Matrix& Matrix::Transpose()
{
	Matrix result{};
	for (int r{ 0 }; r < 4; ++r)
	{
		for (int c{ 0 }; c < 4; ++c)
		{
			result[r][c] = data[c][r];
		}
	}

	data[0] = result[0];
	data[1] = result[1];
	data[2] = result[2];
	data[3] = result[3];

	return *this;
}

const Matrix& Matrix::Inverse()
{
	//Optimized Inverse as explained in FGED1 - used widely in other libraries too.
	const glm::vec3& a = data[0];
	const glm::vec3& b = data[1];
	const glm::vec3& c = data[2];
	const glm::vec3& d = data[3];

	const float x = data[0][3];
	const float y = data[1][3];
	const float z = data[2][3];
	const float w = data[3][3];

	glm::vec3 s = Cross(a, b);
	glm::vec3 t = Cross(c, d);
	glm::vec3 u = a * y - b * x;
	glm::vec3 v = c * w - d * z;

	const float det = Dot(s, v) + Dot(t, u);
	const float invDet = 1.f / det;

	s *= invDet; t *= invDet; u *= invDet; v *= invDet;

	const glm::vec3 r0 = Cross(b, v) + t * y;
	const glm::vec3 r1 = Cross(v, a) - t * x;
	const glm::vec3 r2 = Cross(d, u) + s * w;
	//glm::vec3 r3 = glm::vec3::Cross(u, c) - s * z;

	data[0] = glm::vec4{ r0.x, r1.x, r2.x, 0.f };
	data[1] = glm::vec4{ r0.y, r1.y, r2.y, 0.f };
	data[2] = glm::vec4{ r0.z, r1.z, r2.z, 0.f };
	data[3] = { -Dot(b, t), Dot(a, t), -Dot(d, s), Dot(c, s) };

	return *this;
}

Matrix Matrix::Transpose(const Matrix& m)
{
	Matrix out{ m };
	out.Transpose();

	return out;
}

Matrix Matrix::Inverse(const Matrix& m)
{
	Matrix out{ m };
	out.Inverse();

	return out;
}

Matrix Matrix::CreateLookAtLH(const glm::vec3& origin, const glm::vec3& forward, const glm::vec3& up)
{
	const glm::vec3& right = Normalize(Cross(up, forward));

	Matrix result =
	{
			glm::vec4{right, 0},
			glm::vec4{up, 0},
			glm::vec4{forward, 0},
			glm::vec4{origin, 1}
	};

	return result;
}

Matrix Matrix::CreatePerspectiveFovLH(float fov, float aspect, float zn, float zf)
{
	const float A{ zf / (zf - zn) };
	const float B{ -(zf * zn) / (zf - zn) };

	return
	{
		glm::vec4{ 1 / (aspect * fov), 0, 0, 0 },
		glm::vec4{ 0, 1 / fov, 0, 0 },
		glm::vec4{ 0, 0, A, 1 },
		glm::vec4{ 0, 0, B, 0}
	};
}

glm::vec3 Matrix::GetAxisX() const
{
	return data[0];
}

glm::vec3 Matrix::GetAxisY() const
{
	return data[1];
}

glm::vec3 Matrix::GetAxisZ() const
{
	return data[2];
}

glm::vec3 Matrix::GetTranslation() const
{
	return data[3];
}

Matrix Matrix::CreateTranslation(float x, float y, float z)
{
	return CreateTranslation({ x, y, z });
}

Matrix Matrix::CreateTranslation(const glm::vec3& t)
{
	return { glm::vec3{ 1, 0, 0 }, glm::vec3{ 0, 1, 0 }, glm::vec3{ 0, 0, 1 }, t };
}

Matrix Matrix::CreateRotationX(float pitch)
{
	return {
		{1, 0, 0, 0},
		{0, cos(pitch), -sin(pitch), 0},
		{0, sin(pitch), cos(pitch), 0},
		{0, 0, 0, 1}
	};
}

Matrix Matrix::CreateRotationY(float yaw)
{
	return {
		{cos(yaw), 0, -sin(yaw), 0},
		{0, 1, 0, 0},
		{sin(yaw), 0, cos(yaw), 0},
		{0, 0, 0, 1}
	};
}

Matrix Matrix::CreateRotationZ(float roll)
{
	return {
		{cos(roll), sin(roll), 0, 0},
		{-sin(roll), cos(roll), 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 1}
	};
}

Matrix Matrix::CreateRotation(float pitch, float yaw, float roll)
{
	return CreateRotation({ pitch, yaw, roll });
}

Matrix Matrix::CreateRotation(const glm::vec3& r)
{
	return CreateRotationX(r[0]) * CreateRotationY(r[1]) * CreateRotationZ(r[2]);
}

Matrix Matrix::CreateScale(float sx, float sy, float sz)
{
	return { {sx, 0, 0}, {0, sy, 0}, {0, 0, sz}, glm::vec3{} };
}

Matrix Matrix::CreateScale(const glm::vec3& s)
{
	return CreateScale(s[0], s[1], s[2]);
}

glm::vec3 Matrix::Normalize(const glm::vec3& vec)
{
	const float m = Magnitude(vec);
	return { vec.x / m, vec.y / m, vec.z / m };
}

const float Matrix::Magnitude(const glm::vec3& vec)
{
	return sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

float Matrix::Dot(const glm::vec3& v1, const glm::vec3& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

float Matrix::Dot(const glm::vec4& v1, const glm::vec4& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

glm::vec3 Matrix::Cross(const glm::vec3& v1, const glm::vec3& v2)
{
	return glm::vec3{
		v1.y * v2.z - v1.z * v2.y,
		v1.z * v2.x - v1.x * v2.z,
		v1.x * v2.y - v1.y * v2.x
	};
}

#pragma region Operator Overloads
glm::vec4& Matrix::operator[](int index)
{
	assert(index <= 3 && index >= 0);
	return data[index];
}

glm::vec4 Matrix::operator[](int index) const
{
	assert(index <= 3 && index >= 0);
	return data[index];
}

Matrix Matrix::operator*(const Matrix& m) const
{
	Matrix result{};
	Matrix m_transposed = Transpose(m);

	for (int r{ 0 }; r < 4; ++r)
	{
		for (int c{ 0 }; c < 4; ++c)
		{
			result[r][c] = Dot(data[r], m_transposed[c]);
		}
	}

	return result;
}

const Matrix& Matrix::operator*=(const Matrix& m)
{
	Matrix copy{ *this };
	Matrix m_transposed = Transpose(m);

	for (int r{ 0 }; r < 4; ++r)
	{
		for (int c{ 0 }; c < 4; ++c)
		{
			data[r][c] = Dot(copy[r], m_transposed[c]);
		}
	}

	return *this;
}
#pragma endregion