#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class LogicalDevice;
class PhysicalDevice;
class Instance;

class Swapchain
{
public:
	Swapchain(PhysicalDevice* pPhysicalDevice, LogicalDevice* pDevice, Instance* pInstance);
	~Swapchain();
	void CleanupSwapChain(VkImageView depthImageView, VkImage depthImage, VkDeviceMemory depthImageMemory);

	void CreateImageViews();
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	void CreateFramebuffers(VkRenderPass renderPass, VkImageView depthImageView);

	VkFormat GetSwapchainImageFormat() const { return m_SwapchainImageFormat; }
	std::vector<VkFramebuffer> GetSwapChainFramebuffers() { return m_SwapchainFramebuffers; }
	VkExtent2D GetSwapchainExtent() const { return m_SwapchainExtent; }
	VkSwapchainKHR GetSwapchain() const { return m_Swapchain; }

private:
	VkSwapchainKHR m_Swapchain;
	VkFormat m_SwapchainImageFormat;
	VkExtent2D m_SwapchainExtent;
	std::vector<VkImage> m_SwapchainImages;
	std::vector<VkImageView> m_SwapchainImageViews;
	std::vector<VkFramebuffer> m_SwapchainFramebuffers;

	LogicalDevice* m_pDevice;
	PhysicalDevice* m_pPhysicalDevice;
	Instance* m_pInstance;
};