#include "DescriptorSetLayout.h"
#include "LogicalDevice.h"
#include <array>
#include <stdexcept>

DescriptorSetLayout::DescriptorSetLayout(LogicalDevice* pDevice)
	: m_DescriptorSetLayout(VK_NULL_HANDLE)
	, m_pDevice(pDevice)
{
	CreateDescriptorSetLayout();
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	DestroyDescriptorSetLayout();
}

void DescriptorSetLayout::CreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding normalSamplerLayoutBinding{};
    normalSamplerLayoutBinding.binding = 2;
    normalSamplerLayoutBinding.descriptorCount = 1;
    normalSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    normalSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding metalRoughSamplerLayoutBinding{};
	metalRoughSamplerLayoutBinding.binding = 3;
	metalRoughSamplerLayoutBinding.descriptorCount = 1;
	metalRoughSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	metalRoughSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding albedoBinding{};
    albedoBinding.binding = 4;
    albedoBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    albedoBinding.descriptorCount = 1;
    albedoBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding normalBinding{};
    normalBinding.binding = 5;
    normalBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    normalBinding.descriptorCount = 1;
    normalBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding metalRoughBinding{};
    metalRoughBinding.binding = 6;
    metalRoughBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    metalRoughBinding.descriptorCount = 1;
    metalRoughBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 7> bindings = { uboLayoutBinding, samplerLayoutBinding, normalSamplerLayoutBinding, metalRoughSamplerLayoutBinding, albedoBinding, normalBinding, metalRoughBinding };
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(m_pDevice->GetVkDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void DescriptorSetLayout::DestroyDescriptorSetLayout()
{
    vkDestroyDescriptorSetLayout(m_pDevice->GetVkDevice(), m_DescriptorSetLayout, nullptr);
}
