#pragma once
#include <glm/glm.hpp>

struct Matrix
{
	Matrix() = default;
	Matrix(
		const glm::vec3& xAxis,
		const glm::vec3& yAxis,
		const glm::vec3& zAxis,
		const glm::vec3& t);

	Matrix(
		const glm::vec4& xAxis,
		const glm::vec4& yAxis,
		const glm::vec4& zAxis,
		const glm::vec4& t);

	Matrix(const Matrix& m);

	glm::mat4 GetMat4() const
	{
		return glm::mat4{
			data[0].x, data[1].x, data[2].x, data[3].x,
			data[0].y, data[1].y, data[2].y, data[3].y,
			data[0].z, data[1].z, data[2].z, data[3].z,
			data[0].w, data[1].w, data[2].w, data[3].w
		};
	}

	glm::vec3 TransformVector(const glm::vec3& v) const;
	glm::vec3 TransformVector(float x, float y, float z) const;
	glm::vec3 TransformPoint(const glm::vec3& p) const;
	glm::vec3 TransformPoint(float x, float y, float z) const;

	glm::vec4 TransformPoint(const glm::vec4& p) const;
	glm::vec4 TransformPoint(float x, float y, float z, float w) const;

	const Matrix& Transpose();
	const Matrix& Inverse();

	glm::vec3 GetAxisX() const;
	glm::vec3 GetAxisY() const;
	glm::vec3 GetAxisZ() const;
	glm::vec3 GetTranslation() const;

	static Matrix CreateTranslation(float x, float y, float z);
	static Matrix CreateTranslation(const glm::vec3& t);
	static Matrix CreateRotationX(float pitch);
	static Matrix CreateRotationY(float yaw);
	static Matrix CreateRotationZ(float roll);
	static Matrix CreateRotation(float pitch, float yaw, float roll);
	static Matrix CreateRotation(const glm::vec3& r);
	static Matrix CreateScale(float sx, float sy, float sz);
	static Matrix CreateScale(const glm::vec3& s);
	static Matrix Transpose(const Matrix& m);
	static Matrix Inverse(const Matrix& m);

	static Matrix CreateLookAtLH(const glm::vec3& origin, const glm::vec3& forward, const glm::vec3& up);
	static Matrix CreatePerspectiveFovLH(float fovy, float aspect, float zn, float zf);

	static glm::vec3 Normalize(const glm::vec3& vec);
	static const float Magnitude(const glm::vec3& vec);
	static glm::vec3 Cross(const glm::vec3& v1, const glm::vec3& v2);
	static float Dot(const glm::vec3& v1, const glm::vec3& v2);
	static float Dot(const glm::vec4& v1, const glm::vec4& v2);

	glm::vec4& operator[](int index);
	glm::vec4 operator[](int index) const;
	Matrix operator*(const Matrix& m) const;
	const Matrix& operator*=(const Matrix& m);

private:

	//Row-Major Matrix
	glm::vec4 data[4]
	{
		{1,0,0,0}, //xAxis
		{0,1,0,0}, //yAxis
		{0,0,1,0}, //zAxis
		{0,0,0,1}  //T
	};

	// v0x v0y v0z v0w
	// v1x v1y v1z v1w
	// v2x v2y v2z v2w
	// v3x v3y v3z v3w
};
