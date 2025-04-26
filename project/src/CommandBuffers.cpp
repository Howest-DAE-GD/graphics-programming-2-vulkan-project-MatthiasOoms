#include "CommandBuffers.h"
#include "LogicalDevice.h"
#include "CommandPool.h"
#include <stdexcept>

CommandBuffers::CommandBuffers(LogicalDevice* pDevice, CommandPool* pCommandPool, int maxFrames)
	: m_pDevice{ pDevice }
	, m_pCommandPool { pCommandPool }
{
	m_CommandBuffers.resize(maxFrames);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_pCommandPool->GetCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

	if (vkAllocateCommandBuffers(m_pDevice->GetVkDevice(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

CommandBuffers::~CommandBuffers()
{
	for (auto& commandBuffer : m_CommandBuffers)
	{
		if (commandBuffer != VK_NULL_HANDLE)
		{
			vkFreeCommandBuffers(m_pDevice->GetVkDevice(), m_pCommandPool->GetCommandPool(), 1, &commandBuffer);
			commandBuffer = VK_NULL_HANDLE;
		}
	}
}
