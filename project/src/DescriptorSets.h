#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class LogicalDevice;
class Buffer;
class Model;

class DescriptorSets
{
public:
	DescriptorSets(int maxFramesInFlight, LogicalDevice* pDevice, VkDescriptorSetLayout* descriptorSetLayout, VkDescriptorPool* descriptorPool, std::vector<Buffer*> uniformBuffers, Model* model);
	~DescriptorSets();
	std::vector<VkDescriptorSet> GetDescriptorSets() const { return m_DescriptorSets; }

private:
	LogicalDevice* m_pDevice;
	std::vector<VkDescriptorSet> m_DescriptorSets;
};