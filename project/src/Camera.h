#pragma once
#include <cassert>
#include <glm/glm.hpp>

#include "Matrix.h"
#include <GLFW/glfw3.h>
#include "Window.h"

constexpr float PI = 3.14159265358979323846f;
constexpr float TO_RADIANS(PI / 180.0f);

struct Camera
{
private:
	float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };
	float aspectRatio{};

public:
	Camera() = default;

	Camera(const glm::vec3& _origin, float _fovAngle) :
		origin{ _origin },
		fovAngle{ _fovAngle }
	{
	}

	glm::vec3 origin{};
	float fovAngle{ 90.f };

	glm::vec3 forward{ glm::vec3{ 0, 0, -1 } };
	glm::vec3 up{ glm::vec3{ 0, 1, 0 } };
	glm::vec3 right{ glm::vec3{ 1, 0, 0 } };

	float totalPitch{ -90.f };
	float totalYaw{};

	const float baseSpeed{ 3.f };
	const float rotSpeed{ 0.25f };

	const float fNear{ 0.1f };
	const float fFar{ 100.f };

	Matrix viewMatrix{};
	Matrix invViewMatrix{};
	Matrix projectionMatrix{};

	GLFWwindow* window{ nullptr };

	double lastMouseX = 0.0;
	double lastMouseY = 0.0;
	bool isFirstFrame = true;

	float GetFov() const { return fov; }
	void SetFov(float value)
	{
		fov = value;
		CalculateProjectionMatrix();
	}

	float GetAspectRatio() const { return aspectRatio; }
	void SetAspectRatio(float value)
	{
		aspectRatio = value;
		CalculateProjectionMatrix();
	}

	glm::vec3 GetOrigin() const { return origin; }

	void Initialize(Window* _window, float _fovAngle = 90.f, glm::vec3 _origin = { 0.f,0.f,0.f })
	{
		window = _window->GetGLFWWindow();
		aspectRatio = _window->GetAspectRatio();
		fovAngle = _fovAngle;
		fov = tanf((fovAngle * TO_RADIANS) / 2.f);

		origin = _origin;
		CalculateProjectionMatrix();
	}

	void CalculateViewMatrix()
	{
		//ONB => invViewMatrix
		//Inverse(ONB) => ViewMatrix
		invViewMatrix = Matrix::CreateLookAtLH(origin, forward, up);
		viewMatrix = invViewMatrix.Inverse();
		//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
	}

	void CalculateProjectionMatrix()
	{
		projectionMatrix = Matrix::CreatePerspectiveFovLH(fov, aspectRatio, fNear, fFar);
		//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
	}

	void Update(float deltaTime)
	{
		HandleKeyboardInput(deltaTime);
		HandleMouseInput(deltaTime);

		//Update Matrices
		CalculateViewMatrix();
	}

	void HandleKeyboardInput(float deltaTime)
	{
		// GLFW example: querying keys instead of SDL
		float moveSpeed = baseSpeed;
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		{
			moveSpeed *= 2.0f;
		}

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		{
			origin += forward * moveSpeed * deltaTime;
		}
		else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			origin -= forward * moveSpeed * deltaTime;
		}

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		{
			origin -= right * moveSpeed * deltaTime;
		}
		else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			origin += right * moveSpeed * deltaTime;
		}
	}

	void HandleMouseInput(float deltaTime)
	{
		// Get current mouse position
		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		// Used to calculate relative mouse movement
		if (isFirstFrame)
		{
			lastMouseX = mouseX;
			lastMouseY = mouseY;
			isFirstFrame = false;
		}

		float deltaX = static_cast<float>(mouseX - lastMouseX);
		float deltaY = static_cast<float>(mouseY - lastMouseY);

		lastMouseX = mouseX;
		lastMouseY = mouseY;

		// Check mouse button states
		bool rightHeld = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
		bool leftHeld = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

		if (rightHeld)
		{
			if (!leftHeld)
			{
				// Only right is held
				totalYaw += deltaX * rotSpeed;
				totalPitch += deltaY * rotSpeed;

				// Clamp pitch
				totalPitch = glm::clamp(totalPitch, -180.f, 0.f);

				// Create rotation matrix
				Matrix rot = Matrix::CreateRotationX(totalPitch * TO_RADIANS) * Matrix::CreateRotationZ(-totalYaw * TO_RADIANS);

				forward = Matrix::Normalize(rot.TransformVector(glm::vec3{ 0, 0, -1 }));
				right = Matrix::Normalize(rot.TransformVector(glm::vec3{ 1, 0, 0 }));
				up = Matrix::Normalize(rot.TransformVector(glm::vec3{ 0, 1, 0 }));
			}
			else
			{
				// Both left and right are held
				origin += (up * deltaY * deltaTime * baseSpeed);
				origin += (right * -deltaX * deltaTime * baseSpeed);
			}
		}
		else if (leftHeld)
		{
			// Only left is held
			origin += (forward * -deltaY * deltaTime * baseSpeed);

			totalYaw += deltaX * rotSpeed;

			Matrix rot = Matrix::CreateRotationX(totalPitch * TO_RADIANS) * Matrix::CreateRotationZ(-totalYaw * TO_RADIANS);

			forward = Matrix::Normalize(rot.TransformVector(glm::vec3{ 0, 0, -1 }));
			right = Matrix::Normalize(rot.TransformVector(glm::vec3{ 1, 0, 0 }));
			up = Matrix::Normalize(rot.TransformVector(glm::vec3{ 0, 1, 0 }));
		}
	}
};