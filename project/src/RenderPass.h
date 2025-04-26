#pragma once
#include <vulkan/vulkan.h>

class LogicalDevice;

class RenderPass
{
public:
	RenderPass(LogicalDevice* pDevice, VkFormat swapchainImageFormat, VkFormat depthImageFormat);
	~RenderPass();
	VkRenderPass GetRenderPass() const { return m_RenderPass; }

private:
	LogicalDevice* m_pDevice;
	VkRenderPass m_RenderPass;
	void CreateRenderPass(VkFormat swapchainImageFormat, VkFormat depthImageFormat);
	void DestroyRenderPass();
};