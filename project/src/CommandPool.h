#pragma once
#include <vulkan/vulkan.h>

class LogicalDevice;

class CommandPool
{
public:
	CommandPool(LogicalDevice* pDevice);
	~CommandPool();
	VkCommandPool GetCommandPool() const { return m_CommandPool; }

private:
	LogicalDevice* m_pDevice;
	VkCommandPool m_CommandPool;
};