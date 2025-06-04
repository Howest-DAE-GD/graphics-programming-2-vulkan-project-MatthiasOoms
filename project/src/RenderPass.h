#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class LogicalDevice;

class RenderPass
{
public:
	RenderPass(LogicalDevice* pDevice, VkFormat swapchainImageFormat, VkFormat depthImageFormat);
	RenderPass(LogicalDevice* pDevice, VkFormat depthImageFormat);
	RenderPass(LogicalDevice* pDevice, VkFormat albedoImageFormat, VkFormat normalImageFormat, VkFormat positionImageFormat);
	~RenderPass();

	VkRenderPass GetRenderPass() const { return m_RenderPass; }
	bool IsDepthOnly() const { return m_IsDepthOnly; }
	uint32_t GetAttachmentCount() const { return m_AttachmentCount; }

private:
	LogicalDevice* m_pDevice;
	VkRenderPass m_RenderPass;
	void CreateRenderPass(VkFormat swapchainImageFormat, VkFormat depthImageFormat);
	void CreateDepthRenderPass(VkFormat depthImageFormat);
	void CreateDeferredRenderPass(VkFormat albedoImageFormat, VkFormat normalImageFormat, VkFormat positionImageFormat);
	void DestroyRenderPass();

	bool m_IsDepthOnly = false;
	uint32_t m_AttachmentCount = 0;
};