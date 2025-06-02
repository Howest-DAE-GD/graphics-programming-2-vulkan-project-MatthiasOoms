#include "Swapchain.h"
#include "LogicalDevice.h"
#include "PhysicalDevice.h"
#include "Instance.h"
#include "Image.h"
#include <stdexcept>
#include <array>

Swapchain::Swapchain(PhysicalDevice* pPhysicalDevice, LogicalDevice* pDevice, Instance* pInstance)
	: m_pDevice(pDevice)
	, m_pInstance(pInstance)
	, m_pPhysicalDevice(pPhysicalDevice)
{
    SwapChainSupportDetails swapChainSupport = m_pPhysicalDevice->QuerySwapChainSupport();

    VkSurfaceFormatKHR surfaceFormat = m_pPhysicalDevice->ChooseSwapSurfaceFormat(swapChainSupport.formats);

    m_SwapchainImageFormat = surfaceFormat.format;

    VkPresentModeKHR presentMode = m_pPhysicalDevice->ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = m_pPhysicalDevice->ChooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = *m_pInstance->GetSurface();

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;

    m_SwapchainExtent = extent;

    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = m_pPhysicalDevice->FindQueueFamilies();
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_pDevice->GetVkDevice(), &createInfo, nullptr, &m_Swapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(m_pDevice->GetVkDevice(), m_Swapchain, &imageCount, nullptr);
    m_SwapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_pDevice->GetVkDevice(), m_Swapchain, &imageCount, m_SwapchainImages.data());
}

Swapchain::~Swapchain()
{
}

void Swapchain::CleanupSwapChain(Image* pImage)
{
    VkDevice device = m_pDevice->GetVkDevice();
    vkDestroyImageView(device, *pImage->GetImageView(), nullptr);
    vkDestroyImage(device, *pImage->GetImage(), nullptr);
    vkFreeMemory(device, *pImage->GetImageMemory(), nullptr);

    for (size_t i{}; i < m_SwapchainFramebuffers.size(); ++i)
    {
        vkDestroyFramebuffer(m_pDevice->GetVkDevice(), m_SwapchainFramebuffers[i], nullptr);
    }

    for (size_t i{}; i < m_SwapchainImageViews.size(); ++i)
    {
        vkDestroyImageView(m_pDevice->GetVkDevice(), m_SwapchainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(m_pDevice->GetVkDevice(), m_Swapchain, nullptr);
}

void Swapchain::CreateImageViews()
{
    m_SwapchainImageViews.resize(m_SwapchainImages.size());

    for (uint32_t i{}; i < m_SwapchainImages.size(); ++i)
    {
        m_SwapchainImageViews[i] = CreateImageView(m_SwapchainImages[i], m_SwapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

VkImageView Swapchain::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(m_pDevice->GetVkDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

void Swapchain::CreateFramebuffers(VkRenderPass renderPass, VkImageView depthImageView)
{
    m_SwapchainFramebuffers.resize(m_SwapchainImageViews.size());
    for (size_t i{}; i < m_SwapchainImageViews.size(); ++i)
    {
        std::array<VkImageView, 2> attachments = { m_SwapchainImageViews[i], depthImageView };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_SwapchainExtent.width;
        framebufferInfo.height = m_SwapchainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_pDevice->GetVkDevice(), &framebufferInfo, nullptr, &m_SwapchainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void Swapchain::CreateFramebuffers(VkRenderPass renderPass, VkImageView depthImageView, bool isDepthOnly)
{
    if (!isDepthOnly)
    {
        CreateFramebuffers(renderPass, depthImageView);
    }
    else
    {
        m_SwapchainFramebuffers.resize(m_SwapchainImageViews.size());
        for (size_t i{}; i < m_SwapchainImageViews.size(); ++i)
        {
            std::array<VkImageView, 1> attachments = { depthImageView };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = m_SwapchainExtent.width;
            framebufferInfo.height = m_SwapchainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(m_pDevice->GetVkDevice(), &framebufferInfo, nullptr, &m_SwapchainFramebuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }
}
