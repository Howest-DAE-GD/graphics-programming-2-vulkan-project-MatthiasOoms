#pragma once
#include <vulkan/vulkan.h>

class LogicalDevice;

class DescriptorPool
{
public:
	DescriptorPool(int maxFramesInFlight, int modelCount, LogicalDevice* pDevice);
	~DescriptorPool();
	VkDescriptorPool* GetDescriptorPool() { return &m_DescriptorPool; }

private:
	VkDescriptorPool m_DescriptorPool;
	LogicalDevice* m_pDevice;
};