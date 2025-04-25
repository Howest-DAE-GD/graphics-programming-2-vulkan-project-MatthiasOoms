#pragma once
#define NOMINMAX
#include <vulkan/vulkan.h>
#include <vector>
#include "Structs.h"

class Window;
class Instance;

class PhysicalDevice
{
public:
	PhysicalDevice(Instance* pInstance, Window* pWindow);
	~PhysicalDevice();

	VkPhysicalDevice GetVkPhysicalDevice() const { return m_PhysicalDevice; }
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	// Public methods that use m_PhysicalDevice
	QueueFamilyIndices FindQueueFamilies();
	SwapChainSupportDetails QuerySwapChainSupport();

private:
	Instance* m_pInstance;
	Window* m_pWindow;
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;

	void PickPhysicalDevice();
	// Private methods that use given device
	bool IsDeviceSuitable(VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
};