#pragma once
#include <vulkan/vulkan.h>

class LogicalDevice;

class DescriptorSetLayout
{
public:
	DescriptorSetLayout(LogicalDevice* pDevice);
	~DescriptorSetLayout();
	const VkDescriptorSetLayout* GetDescriptorSetLayout() const { return &m_DescriptorSetLayout; }

private:
	VkDescriptorSetLayout m_DescriptorSetLayout;
	LogicalDevice* m_pDevice;
	void CreateDescriptorSetLayout();
	void DestroyDescriptorSetLayout();
};