#pragma once
#include <vulkan/vulkan.h>
#include "PipelineLayout.h"
#include <vector>
#include <string>

class LogicalDevice;

class GraphicsPipeline
{
public:
	GraphicsPipeline(LogicalDevice* pDevice, VkRenderPass renderPass, VkDescriptorSetLayout* pDescriptorSetLayout, bool isDepthOnly);
	~GraphicsPipeline();
	VkPipeline* GetGraphicsPipeline() { return &m_GraphicsPipeline; }
	PipelineLayout* GetPipelineLayout() { return m_pPipelineLayout; }

private:
	LogicalDevice* m_pDevice;
	PipelineLayout* m_pPipelineLayout;
	VkPipeline m_GraphicsPipeline;
	VkShaderModule m_VertexShaderModule;
	VkShaderModule m_FragmentShaderModule;

	void CreatePipelineLayout(VkDescriptorSetLayout* pDescriptorSetLayout);
	void CreateGraphicsPipeline(VkRenderPass renderPass, bool isDepthOnly);
	void CreateShaderModules(const char* vertexPath, const char* fragmentPath);
	VkShaderModule CreateShaderModule(const std::vector<char>& code);
	static std::vector<char> ReadFile(const std::string& filename);
	void Cleanup();
};