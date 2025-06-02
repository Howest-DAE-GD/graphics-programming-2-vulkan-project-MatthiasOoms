#pragma once
#include <vulkan/vulkan.h>

class LogicalDevice;
class CommandPool;

class Image
{
public:
	Image(LogicalDevice* pDevice, CommandPool* pCommandPool, VkExtent2D swapchainExtent, VkFormat imageFormat, VkImageTiling tiling, VkImageUsageFlagBits usage, VkMemoryPropertyFlagBits properties);
	Image() = default;
	virtual ~Image();

	virtual VkImage* GetImage() { return &m_Image; }
	virtual VkDeviceMemory* GetImageMemory() { return &m_ImageMemory; }
	virtual VkImageView* GetImageView() { return &m_ImageView; }

protected:
	LogicalDevice* m_pDevice;
	CommandPool* m_pCommandPool;
	VkImage m_Image;
	VkDeviceMemory m_ImageMemory;
	VkImageView m_ImageView;

	virtual void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	virtual VkImageView CreateImageView(VkFormat format, VkImageAspectFlags aspectFlags);
	virtual void TransitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	virtual bool HasStencilComponent(VkFormat format);
};