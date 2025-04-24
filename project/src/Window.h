#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

class Window
{
public:
	Window(int width, int height, const char* title);
	~Window();

	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		app->m_FramebufferResized = true;
	}

	GLFWwindow* GetGLFWWindow() const { return m_pWindow; }
	bool IsFramebufferResized() const { return m_FramebufferResized; }
	void ResetFramebufferResized() { m_FramebufferResized = false; }

private:
	GLFWwindow* m_pWindow;
	int m_Width;
	int m_Height;
	const char* m_Title;
	bool m_FramebufferResized = false;
};