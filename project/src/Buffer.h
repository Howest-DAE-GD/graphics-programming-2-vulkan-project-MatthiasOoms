#pragma once
#include <vulkan/vulkan.h>
#include "Structs.h"

class LogicalDevice;
class CommandPool;
class Model;

class Buffer
{
public:
	Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, const uint32_t* data, LogicalDevice* pDevice, CommandPool* pCommandPool, Model* pModel);
	Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, const Vertex* data, LogicalDevice* pDevice, CommandPool* pCommandPool, Model* pModel);
	~Buffer();
	VkBuffer GetBuffer() { return m_Buffer; }
	VkDeviceMemory GetMemory() { return m_Memory; }
	void CopyBuffer(VkBuffer srcBuffer, VkDeviceSize size);

	static void CreateBuffer(LogicalDevice* pDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	static uint32_t FindMemoryType(LogicalDevice* pDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

private:
	LogicalDevice* m_pDevice;
	CommandPool* m_pCommandPool;
	VkBuffer m_Buffer;
	VkDeviceMemory m_Memory;
	VkDeviceSize m_Size;
	VkBufferUsageFlags m_Usage;
	VkMemoryPropertyFlags m_Properties;

};