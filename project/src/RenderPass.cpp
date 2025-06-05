#include "RenderPass.h"
#include "LogicalDevice.h"
#include <array>
#include <stdexcept>

RenderPass::RenderPass(LogicalDevice* pDevice, VkFormat swapchainImageFormat, VkFormat depthImageFormat)
	: m_pDevice(pDevice)
	, m_RenderPass(VK_NULL_HANDLE)
	, m_DepthWrite(false)
{
	CreateRenderPass(swapchainImageFormat, depthImageFormat);
}

RenderPass::RenderPass(LogicalDevice* pDevice, VkFormat swapchainImageFormat, VkFormat depthImageFormat, bool writeDepth)
    : m_pDevice(pDevice)
    , m_RenderPass(VK_NULL_HANDLE)
    , m_DepthWrite(writeDepth)
{
    CreateRenderPass(swapchainImageFormat, depthImageFormat);
}

RenderPass::RenderPass(LogicalDevice* pDevice, VkFormat depthImageFormat)
    : m_pDevice(pDevice)
    , m_RenderPass(VK_NULL_HANDLE)
	, m_DepthWrite(true)
{
    CreateDepthRenderPass(depthImageFormat);
}

RenderPass::RenderPass(LogicalDevice* pDevice, VkFormat albedoImageFormat, VkFormat normalImageFormat, VkFormat positionImageFormat, VkFormat depthImageFormat)
	: m_pDevice(pDevice)
	, m_RenderPass(VK_NULL_HANDLE)
	, m_DepthWrite(false)
{
	CreateDeferredRenderPass(depthImageFormat, albedoImageFormat, normalImageFormat, positionImageFormat);
}

RenderPass::~RenderPass()
{
	DestroyRenderPass();
}

void RenderPass::CreateRenderPass(VkFormat swapchainImageFormat, VkFormat depthImageFormat)
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = depthImageFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	if (m_DepthWrite)
	{
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	}
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    m_AttachmentCount = subpass.colorAttachmentCount;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_pDevice->GetVkDevice(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

void RenderPass::CreateDepthRenderPass(VkFormat depthImageFormat)
{
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = depthImageFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 0;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 0;
    m_AttachmentCount = subpass.colorAttachmentCount;
    subpass.pColorAttachments = nullptr;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 1> attachments = { depthAttachment };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_pDevice->GetVkDevice(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

void RenderPass::CreateDeferredRenderPass(VkFormat depthImageFormat, VkFormat albedoImageFormat, VkFormat normalImageFormat, VkFormat positionImageFormat)
{
    VkAttachmentDescription albedoAttachment{};
    albedoAttachment.format = albedoImageFormat;
    albedoAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    albedoAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    albedoAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    albedoAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    albedoAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    albedoAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    albedoAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference albedoAttachmentRef{};
    albedoAttachmentRef.attachment = 0;
    albedoAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription normalAttachment{};
    normalAttachment.format = normalImageFormat;
    normalAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    normalAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    normalAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    normalAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    normalAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    normalAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    normalAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference normalAttachmentRef{};
    normalAttachmentRef.attachment = 1;
    normalAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription positionAttachment{};
    positionAttachment.format = positionImageFormat;
    positionAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    positionAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    positionAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    positionAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    positionAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    positionAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    positionAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference positionAttachmentRef{};
	positionAttachmentRef.attachment = 2;
    positionAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = depthImageFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 3;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    std::array<VkAttachmentReference, 3> colorAttachmentRefs = { albedoAttachmentRef, normalAttachmentRef, positionAttachmentRef };

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
	m_AttachmentCount = subpass.colorAttachmentCount;
    subpass.pColorAttachments = colorAttachmentRefs.data();

	subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

    std::array<VkAttachmentDescription, 4> attachments = { albedoAttachment, normalAttachment, positionAttachment, depthAttachment };

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_pDevice->GetVkDevice(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

void RenderPass::DestroyRenderPass()
{
	if (m_RenderPass != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(m_pDevice->GetVkDevice(), m_RenderPass, nullptr);
		m_RenderPass = VK_NULL_HANDLE;
	}
}
