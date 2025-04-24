#pragma once
#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>

#include <GLFW/glfw3.h>

class Instance
{
public:
	Instance(int width, int height, std::vector<const char*> validationLayers, std::vector<const char*> deviceExtensions, bool enableValidationLayers = false);
	~Instance();
	VkInstance GetVkInstance() const { return m_Instance; }
	VkDebugUtilsMessengerEXT GetDebugMessenger() const { return m_DebugMessenger; }
	VkSurfaceKHR* GetSurface() { return &m_Surface; }

private:
	VkInstance m_Instance;
	VkDebugUtilsMessengerEXT m_DebugMessenger;
	VkSurfaceKHR m_Surface;

	bool m_EnableValidationLayers;
	std::vector<const char*> m_ValidationLayers;
	std::vector<const char*> m_DeviceExtensions;

	std::vector<const char*> GetRequiredExtensions();
	bool CheckValidationLayerSupport();
	void SetupDebugMessenger();
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
};