#pragma once
#include <vulkan/vulkan.h>

class LogicalDevice;

class DescriptorPool
{
public:
	DescriptorPool(int maxFramesInFlight, LogicalDevice* pDevice);
	~DescriptorPool();
	VkDescriptorPool GetDescriptorPool() const { return m_DescriptorPool; }

private:
	VkDescriptorPool m_DescriptorPool;
	LogicalDevice* m_pDevice;
};