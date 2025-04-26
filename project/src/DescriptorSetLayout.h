#pragma once
#include <vulkan/vulkan.h>

class LogicalDevice;

class DescriptorSetLayout
{
public:
	DescriptorSetLayout(LogicalDevice* pDevice);
	~DescriptorSetLayout();
	VkDescriptorSetLayout* GetDescriptorSetLayout() { return &m_DescriptorSetLayout; }

private:
	VkDescriptorSetLayout m_DescriptorSetLayout;
	LogicalDevice* m_pDevice;
	void CreateDescriptorSetLayout();
	void DestroyDescriptorSetLayout();
};