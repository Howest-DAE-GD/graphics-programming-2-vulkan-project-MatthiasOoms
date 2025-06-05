#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <vulkan/vulkan_core.h>

class LogicalDevice;

class RenderPass
{
public:
	RenderPass(LogicalDevice* pDevice, VkFormat depthImageFormat);
	RenderPass(LogicalDevice* pDevice, VkFormat swapchainImageFormat, VkFormat depthImageFormat, bool loadValues = false);
	RenderPass(LogicalDevice* pDevice, VkFormat albedoImageFormat, VkFormat normalImageFormat, VkFormat positionImageFormat, VkFormat depthImageFormat);
	~RenderPass();

	VkRenderPass GetRenderPass() const { return m_RenderPass; }
	bool DoesWriteDepth() const { return m_DepthWrite; }
	uint32_t GetAttachmentCount() const { return m_AttachmentCount; }

private:
	LogicalDevice* m_pDevice;
	VkRenderPass m_RenderPass;
	void CreateRenderPass(VkFormat swapchainImageFormat, VkFormat depthImageFormat, bool loadValues);
	void CreateDepthRenderPass(VkFormat depthImageFormat);
	void CreateDeferredRenderPass(VkFormat depthImageFormat, VkFormat albedoImageFormat, VkFormat normalImageFormat, VkFormat positionImageFormat);
	void DestroyRenderPass();

	bool m_DepthWrite = false;
	uint32_t m_AttachmentCount = 0;
};