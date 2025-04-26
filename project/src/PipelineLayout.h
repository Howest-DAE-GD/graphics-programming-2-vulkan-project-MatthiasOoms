#pragma once
#include <vulkan/vulkan.h>

class LogicalDevice;

class PipelineLayout
{
public:
	PipelineLayout(LogicalDevice* pDevice, const VkDescriptorSetLayout* pDescriptorSetLayout);
	~PipelineLayout();
	VkPipelineLayout GetPipelineLayout() { return m_PipelineLayout; }
private:
	LogicalDevice* m_pDevice;
	VkPipelineLayout m_PipelineLayout;
};