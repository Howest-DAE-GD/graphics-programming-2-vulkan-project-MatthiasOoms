#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class LogicalDevice;
class CommandPool;

class CommandBuffers
{
public:
	CommandBuffers(LogicalDevice* pDevice, CommandPool* pCommandPool, int maxFrames);
	~CommandBuffers();
	CommandPool* GetCommandPool() const { return m_pCommandPool; }
	std::vector<VkCommandBuffer>& GetCommandBuffers() { return m_CommandBuffers; }

private:
	LogicalDevice* m_pDevice;
	CommandPool* m_pCommandPool;
	std::vector<VkCommandBuffer> m_CommandBuffers;
};