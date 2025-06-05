#include "PipelineLayout.h"
#include "LogicalDevice.h"
#include "Structs.h"
#include <stdexcept>

PipelineLayout::PipelineLayout(LogicalDevice* pDevice, const VkDescriptorSetLayout* pDescriptorSetLayout)
	: m_pDevice{ pDevice }
    , m_PipelineLayout{ VK_NULL_HANDLE }
{
    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstants);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1; // Optional
    pipelineLayoutInfo.pSetLayouts = pDescriptorSetLayout; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 1; // Optional
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange; // Optional

    if (vkCreatePipelineLayout(m_pDevice->GetVkDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

PipelineLayout::~PipelineLayout()
{
	vkDestroyPipelineLayout(m_pDevice->GetVkDevice(), m_PipelineLayout, nullptr);
}
