#pragma once
#include <vulkan/vulkan.h>
#include "PipelineLayout.h"
#include <vector>
#include <string>

class LogicalDevice;
class RenderPass;

class GraphicsPipeline
{
public:
	GraphicsPipeline(LogicalDevice* pDevice, RenderPass* renderPass, VkDescriptorSetLayout* pDescriptorSetLayout, const char* vertShader, const char* fragShader, bool handlesDepth);
	GraphicsPipeline(LogicalDevice* pDevice, RenderPass* renderPass, VkDescriptorSetLayout* pDescriptorSetLayout, const char* vertShader, const char* fragShader);
	GraphicsPipeline(LogicalDevice* pDevice, RenderPass* renderPass, VkDescriptorSetLayout* pDescriptorSetLayout, const char* vertShader);
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
	void CreateGraphicsPipeline(RenderPass* renderPass, bool isDepthOnly);
	void CreateGraphicsPipeline(RenderPass* renderPass);
	void CreateShaderModules(const char* vertexPath, const char* fragmentPath);
	void CreateShaderModules(const char* vertexPath);
	VkShaderModule CreateShaderModule(const std::vector<char>& code);
	static std::vector<char> ReadFile(const std::string& filename);
	void Cleanup();
};