#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "Texture.h"

class LogicalDevice;
class PhysicalDevice;
class CommandPool;
class Instance;

class Swapchain
{
public:
	Swapchain(PhysicalDevice* pPhysicalDevice, LogicalDevice* pDevice, Instance* pInstance, CommandPool* pCommandPool);
	~Swapchain();
	void CleanupSwapChain(Image* pImage);

	void CreateImageViews();
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	void CreateFramebuffers(VkRenderPass renderPass, VkImageView depthImageView);
	void CreateDepthFramebuffers(VkRenderPass renderPass, VkImageView depthImageView);
	void CreateDeferredFramebuffers(VkRenderPass renderPass, VkImageView depthImageView);

	VkFormat GetSwapChainImageFormat() const { return m_SwapchainImageFormat; }
	std::vector<VkFramebuffer> GetSwapChainFramebuffers() { return m_SwapchainFramebuffers; }
	std::vector<VkFramebuffer> GetSwapChainDepthFramebuffers() { return m_SwapchainDepthFramebuffers; }
	std::vector<VkFramebuffer> GetSwapChainDeferredFramebuffers() { return m_SwapchainDeferredFramebuffers; }
	VkExtent2D GetSwapchainExtent() const { return m_SwapchainExtent; }
	VkSwapchainKHR GetSwapchain() const { return m_Swapchain; }

	std::vector<Texture*>& GetGBufferAlbedoImages() { return m_pGBufferAlbedoImages; }
	std::vector<Texture*>& GetGBufferNormalImages() { return m_pGBufferNormalImages; }
	std::vector<Texture*>& GetGBufferMetalRoughImages() { return m_pGBufferMetalRoughImages; }

private:
	void CreateImages(CommandPool* pCommandPool);

	VkSwapchainKHR m_Swapchain;
	VkFormat m_SwapchainImageFormat;
	VkExtent2D m_SwapchainExtent;
	std::vector<VkImage> m_SwapchainImages;
	std::vector<VkImageView> m_SwapchainImageViews;
	std::vector<VkFramebuffer> m_SwapchainFramebuffers;
	std::vector<VkFramebuffer> m_SwapchainDepthFramebuffers;
	std::vector<VkFramebuffer> m_SwapchainDeferredFramebuffers;

	// G-buffer attachments
	std::vector<Texture*> m_pGBufferAlbedoImages;
	std::vector<Texture*> m_pGBufferNormalImages;
	std::vector<Texture*> m_pGBufferMetalRoughImages;

	LogicalDevice* m_pDevice;
	PhysicalDevice* m_pPhysicalDevice;
	Instance* m_pInstance;
};