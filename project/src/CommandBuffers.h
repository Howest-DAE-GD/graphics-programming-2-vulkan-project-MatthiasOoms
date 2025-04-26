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

	static VkCommandBuffer BeginSingleTimeCommands(LogicalDevice* pDevice, CommandPool* pCommandPool);
	static void EndSingleTimeCommands(LogicalDevice* pDevice, CommandPool* pCommandPool, VkCommandBuffer commandBuffer);

private:
	LogicalDevice* m_pDevice;
	CommandPool* m_pCommandPool;
	std::vector<VkCommandBuffer> m_CommandBuffers;
};