#pragma once
#include <vulkan/vulkan.h>
#include "Image.h"
#include <string>

class LogicalDevice;
class CommandPool;

class Texture final : public Image
{
public:
	Texture(LogicalDevice* pDevice, CommandPool* pCommandPool, VkExtent2D swapchainExtent, VkFormat imageFormat, VkImageTiling tiling, VkImageUsageFlagBits usage, VkMemoryPropertyFlagBits properties, const std::string texturePath);
	~Texture();
	void CreateSampler(VkPhysicalDevice pPhysicalDevice);

	VkSampler* GetSampler() { return &m_Sampler; }

private:
	VkSampler m_Sampler;
	void CopyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height);
};