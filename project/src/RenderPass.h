#pragma once
#include <vulkan/vulkan.h>

class LogicalDevice;

class RenderPass
{
public:
	RenderPass(LogicalDevice* pDevice, VkFormat swapchainImageFormat, VkFormat depthImageFormat);
	RenderPass(LogicalDevice* pDevice, VkFormat depthImageFormat);
	~RenderPass();

	VkRenderPass GetRenderPass() const { return m_RenderPass; }
	bool IsDepthOnly() const { return m_IsDepthOnly; }

private:
	LogicalDevice* m_pDevice;
	VkRenderPass m_RenderPass;
	void CreateRenderPass(VkFormat swapchainImageFormat, VkFormat depthImageFormat);
	void CreateDepthRenderPass(VkFormat depthImageFormat);
	void DestroyRenderPass();

	bool m_IsDepthOnly = false;
};