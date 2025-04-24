#include "Window.h"
#include <stdexcept>

Window::Window(int width, int height, const char* title)
	: m_Width(width)
    , m_Height(height)
    , m_Title(title)
{
	if (!glfwInit())
	{
		throw std::runtime_error("Failed to initialize GLFW");
	}

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_pWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
    glfwSetWindowUserPointer(m_pWindow, this);
    glfwSetFramebufferSizeCallback(m_pWindow, FramebufferResizeCallback);
}

Window::~Window()
{
	glfwDestroyWindow(m_pWindow);
	glfwTerminate();
}

