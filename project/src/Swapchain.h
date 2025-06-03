#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class LogicalDevice;
class PhysicalDevice;
class Instance;
class Image;

class Swapchain
{
public:
	Swapchain(PhysicalDevice* pPhysicalDevice, LogicalDevice* pDevice, Instance* pInstance);
	~Swapchain();
	void CleanupSwapChain(Image* pImage);

	void CreateImageViews();
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	void CreateFramebuffers(VkRenderPass renderPass, VkImageView depthImageView);
	void CreateDepthFramebuffers(VkRenderPass renderPass, VkImageView depthImageView);

	VkFormat GetSwapChainImageFormat() const { return m_SwapchainImageFormat; }
	std::vector<VkFramebuffer> GetSwapChainFramebuffers() { return m_SwapchainFramebuffers; }
	std::vector<VkFramebuffer> GetSwapChainDepthFramebuffers() { return m_SwapchainDepthFramebuffers; }
	VkExtent2D GetSwapchainExtent() const { return m_SwapchainExtent; }
	VkSwapchainKHR GetSwapchain() const { return m_Swapchain; }

private:
	VkSwapchainKHR m_Swapchain;
	VkFormat m_SwapchainImageFormat;
	VkExtent2D m_SwapchainExtent;
	std::vector<VkImage> m_SwapchainImages;
	std::vector<VkImageView> m_SwapchainImageViews;
	std::vector<VkFramebuffer> m_SwapchainFramebuffers;
	std::vector<VkFramebuffer> m_SwapchainDepthFramebuffers;

	LogicalDevice* m_pDevice;
	PhysicalDevice* m_pPhysicalDevice;
	Instance* m_pInstance;
};