#pragma once
#include <vulkan/vulkan.h>

class Instance;
class PhysicalDevice;

class LogicalDevice
{
public:
	LogicalDevice(PhysicalDevice* pPhysicalDevice, Instance* pInstance);
	~LogicalDevice();

	VkDevice GetVkDevice() const { return m_Device; }
	VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
	VkQueue GetPresentQueue() const { return m_PresentQueue; }
private:
	VkDevice m_Device;
	PhysicalDevice* m_pPhysicalDevice;

	VkQueue m_GraphicsQueue;
	VkQueue m_PresentQueue;
};