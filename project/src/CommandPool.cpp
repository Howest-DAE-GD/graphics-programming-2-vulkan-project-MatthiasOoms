#include "CommandPool.h"
#include "LogicalDevice.h"
#include "PhysicalDevice.h"
#include "Structs.h"
#include <stdexcept>

CommandPool::CommandPool(LogicalDevice* pDevice)
	: m_pDevice{ pDevice }
{
    QueueFamilyIndices queueFamilyIndices = m_pDevice->GetPhysicalDevice()->FindQueueFamilies();

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(m_pDevice->GetVkDevice(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

CommandPool::~CommandPool()
{
	vkDestroyCommandPool(m_pDevice->GetVkDevice(), m_CommandPool, nullptr);
}
