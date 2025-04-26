#include "Buffer.h"
#include "LogicalDevice.h"
#include "PhysicalDevice.h"
#include "CommandBuffers.h"
#include "Model.h"
#include <stdexcept>

Buffer::Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, const uint32_t* pData, LogicalDevice* pDevice, CommandPool* pCommandPool, Model* pModel)
	: m_Size(size)
	, m_Usage(usage)
	, m_Properties(properties)
	, m_Buffer(VK_NULL_HANDLE)
	, m_Memory(VK_NULL_HANDLE)
	, m_pDevice(pDevice)
	, m_pCommandPool(pCommandPool)
{
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(m_pDevice, m_Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(m_pDevice->GetVkDevice(), stagingBufferMemory, 0, m_Size, 0, &data);
	memcpy(data, pData, (size_t)m_Size);
	vkUnmapMemory(m_pDevice->GetVkDevice(), stagingBufferMemory);

	CreateBuffer(m_pDevice, m_Size, m_Usage, m_Properties, m_Buffer, m_Memory);

	CopyBuffer(stagingBuffer, m_Size);

	vkDestroyBuffer(m_pDevice->GetVkDevice(), stagingBuffer, nullptr);
	vkFreeMemory(m_pDevice->GetVkDevice(), stagingBufferMemory, nullptr);
}

Buffer::Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, const Vertex* pData, LogicalDevice* pDevice, CommandPool* pCommandPool, Model* pModel)
	: m_Size(size)
	, m_Usage(usage)
	, m_Properties(properties)
	, m_Buffer(VK_NULL_HANDLE)
	, m_Memory(VK_NULL_HANDLE)
	, m_pDevice(pDevice)
	, m_pCommandPool(pCommandPool)
{
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(m_pDevice, m_Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(m_pDevice->GetVkDevice(), stagingBufferMemory, 0, m_Size, 0, &data);
	memcpy(data, pData, (size_t)m_Size);
	vkUnmapMemory(m_pDevice->GetVkDevice(), stagingBufferMemory);

	CreateBuffer(m_pDevice, m_Size, m_Usage, m_Properties, m_Buffer, m_Memory);

	CopyBuffer(stagingBuffer, m_Size);

	vkDestroyBuffer(m_pDevice->GetVkDevice(), stagingBuffer, nullptr);
	vkFreeMemory(m_pDevice->GetVkDevice(), stagingBufferMemory, nullptr);
}

Buffer::Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, LogicalDevice* pDevice, CommandPool* pCommandPool, void** uniformBufferMapped)
	: m_Size(size)
	, m_Usage(usage)
	, m_Properties(properties)
	, m_Buffer(VK_NULL_HANDLE)
	, m_Memory(VK_NULL_HANDLE)
	, m_pDevice(pDevice)
	, m_pCommandPool(pCommandPool)
{
	Buffer::CreateBuffer(m_pDevice, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_Buffer, m_Memory);

	vkMapMemory(m_pDevice->GetVkDevice(), m_Memory, 0, size, 0, uniformBufferMapped);
}

Buffer::~Buffer()
{
	if (m_Buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(m_pDevice->GetVkDevice(), m_Buffer, nullptr);
	}
	if (m_Memory != VK_NULL_HANDLE)
	{
		vkFreeMemory(m_pDevice->GetVkDevice(), m_Memory, nullptr);
	}
}

void Buffer::CopyBuffer(VkBuffer srcBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = CommandBuffers::BeginSingleTimeCommands(m_pDevice, m_pCommandPool);

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, m_Buffer, 1, &copyRegion);

	CommandBuffers::EndSingleTimeCommands(m_pDevice, m_pCommandPool, commandBuffer);
}

void Buffer::CreateBuffer(LogicalDevice* pDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(pDevice->GetVkDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create vertex buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(pDevice->GetVkDevice(), buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(pDevice, memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(pDevice->GetVkDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(pDevice->GetVkDevice(), buffer, bufferMemory, 0);
}

uint32_t Buffer::FindMemoryType(LogicalDevice* pDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(pDevice->GetPhysicalDevice()->GetVkPhysicalDevice(), &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}