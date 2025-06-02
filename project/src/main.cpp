#define NOMINMAX // ::max and ::min are defined in windows.h

#define VK_USE_PLATFORM_WIN32_KHR
#include "Structs.h"
#include "Window.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "Swapchain.h"
#include "RenderPass.h"
#include "DescriptorSetLayout.h"
#include "PipelineLayout.h"
#include "GraphicsPipeline.h"
#include "CommandPool.h"
#include "CommandBuffers.h"
#include "Model.h"
#include "Buffer.h"
#include "DescriptorPool.h"
#include "DescriptorSets.h"
#include "Image.h"
#include "Texture.h"
#include "Camera.h"
#include "Timer.h"
#include "ModelLoader.h"

#include <unordered_map> // unordered_map
#include <stdexcept> // runtime_error
#include <algorithm> // clamp
#include <optional> // optional
#include <iostream> // cerr
#include <fstream> // ifstream
#include <cstdlib> // EXIT
#include <cstdint> // uint32_t
#include <limits> // numeric_limits
#include <vector> // vector
#include <chrono> // duration
#include <array> // array
#include <set> // set

const uint32_t g_WIDTH = 800;
const uint32_t g_HEIGHT = 600;

const std::string g_MODEL_PATH = "resources/models/Sponza.gltf";
//const std::string g_MODEL_PATH = "resources/models/Box.gltf";
//const std::string g_MODEL_PATH = "resources/models/viking_room.obj";
//const std::string g_MODEL_PATH = "resources/models/cubes.gltf";

const std::string g_TEXTURE_PATH = "resources/textures/viking_room.png";

const int g_MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> g_ValidationLayers = 
{
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> g_DeviceExtensions = 
{ 
    VK_KHR_SWAPCHAIN_EXTENSION_NAME 
};

#ifdef NDEBUG
const bool g_EnableValidationLayers = false;
#else
const bool g_EnableValidationLayers = true;
#endif

class HelloTriangleApplication 
{
public:
    void Run()
    {
        LoadModels();
        InitWindow();
        InitVulkan();
        InitCamera();
        MainLoop();
        Cleanup();
    }

private:
	// Member variables
    Window* m_pWindow;

    Instance* m_pInstance;

	PhysicalDevice* m_pPhysicalDevice;
	LogicalDevice* m_pDevice;

	Swapchain* m_pSwapchain;

    RenderPass* m_pRenderPass;
	DescriptorSetLayout* m_pDescriptorSetLayout;
	GraphicsPipeline* m_pGraphicsPipeline;

	CommandPool* m_pCommandPool;

	CommandBuffers* m_pCommandBuffers;

    std::vector<VkSemaphore> m_ImageAvailableSemaphores;
    std::vector<VkSemaphore> m_RenderFinishedSemaphores;
    std::vector<VkFence> m_InFlightFences;
    uint32_t m_CurrentFrame = 0;

	std::vector<Model*> m_pModels;

    Camera* m_pCamera;
    Timer m_Timer;

	Buffer* m_pVertexBuffer;
	Buffer* m_pIndexBuffer;

    std::vector<Buffer*> m_UniformBuffers;
    std::vector<void*> m_UniformBuffersMapped;

	DescriptorPool* m_pDescriptorPool;

    Image* m_pDepthImage;

    void InitWindow()
    {
		m_pWindow = new Window(g_WIDTH, g_HEIGHT, "Vulkan");
    }

    void InitCamera()
    {
		m_pCamera = new Camera();
		auto swapchainExtent = m_pSwapchain->GetSwapchainExtent();
        m_pCamera->Initialize(m_pWindow, 90.f, { 0, 0, 2 });
    }

    void InitVulkan()
    {
        CreateInstance();
        CreatePhysicalDevice();
        CreateLogicalDevice();
        CreateSwapChain();
        CreateImageViews();
        CreateRenderPass();
        CreateDescriptorSetLayout();
        CreateGraphicsPipeline();
        CreateCommandPool();
        CreateDepthImage();
        CreateFrameBuffers();
        CreateTextureImage();
        CreateVertexBuffer();
        CreateIndexBuffer();
        CreateUniformBuffers();
        CreateDescriptorPool();
        CreateDescriptorSets();
        CreateCommandBuffers();
        CreateSyncObjects();
    }

    void CreateInstance()
    {
		m_pInstance = new Instance(g_WIDTH, g_HEIGHT, g_ValidationLayers, g_DeviceExtensions, g_EnableValidationLayers);
		m_pInstance->CreateSurface(m_pWindow->GetGLFWWindow());
    }

    void CreatePhysicalDevice()
    {
		m_pPhysicalDevice = new PhysicalDevice(m_pInstance, m_pWindow);
    }

    void CreateLogicalDevice()
    {
		m_pDevice = new LogicalDevice(m_pPhysicalDevice, m_pInstance);
    }

    void CreateSwapChain()
    {
		m_pSwapchain = new Swapchain(m_pPhysicalDevice, m_pDevice, m_pInstance);
    }

    void CreateImageViews()
    {
		m_pSwapchain->CreateImageViews();
    }

    void CreateRenderPass()
    {
		m_pRenderPass = new RenderPass(m_pDevice, m_pSwapchain->GetSwapchainImageFormat(), FindDepthFormat());
    }

    void CreateDescriptorSetLayout()
    {
		m_pDescriptorSetLayout = new DescriptorSetLayout(m_pDevice);
    }

    void CreateGraphicsPipeline()
    {
		m_pGraphicsPipeline = new GraphicsPipeline(m_pDevice, m_pRenderPass->GetRenderPass(), m_pDescriptorSetLayout->GetDescriptorSetLayout());
    }

    void CreateCommandPool()
    {
		m_pCommandPool = new CommandPool(m_pDevice);
    }

    void CreateDepthImage()
    {
		auto swapchainExtent = m_pSwapchain->GetSwapchainExtent();
        VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		VkImageUsageFlagBits usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		VkMemoryPropertyFlagBits properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		m_pDepthImage = new Image(m_pDevice, m_pCommandPool, swapchainExtent, FindDepthFormat(), tiling, usage, properties);
    }

    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_pPhysicalDevice->GetVkPhysicalDevice(), format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }

    VkFormat FindDepthFormat()
    {
        return FindSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

	void CreateFrameBuffers()
	{
        m_pSwapchain->CreateFramebuffers(m_pRenderPass->GetRenderPass(), *m_pDepthImage->GetImageView());
	}

    void CreateTextureImage()
    {
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		VkImageUsageFlagBits usage = static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		VkMemoryPropertyFlagBits properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        
        // Loop over all meshes and create a texture for each one
		for (Model* model : m_pModels)
		{
			if (model->GetDiffuseTexture().empty())
			{
				continue; // Skip models without a texture path
			}
			// Create a texture for the model
			model->SetTexture(new Texture(m_pDevice, m_pCommandPool, m_pSwapchain->GetSwapchainExtent(), m_pSwapchain->GetSwapchainImageFormat(), tiling, usage, properties, model->GetDiffuseTexture()));
            model->GetTexture()->CreateSampler(m_pPhysicalDevice->GetVkPhysicalDevice());
        }
    }

    void LoadModels()
    {
		ModelLoader modelLoader{};

        for (Model* model : modelLoader.LoadModel(g_MODEL_PATH))
        {
			m_pModels.push_back(model);
        }

        m_pModels;
    }

    void CreateVertexBuffer()
    {
        VkBufferUsageFlags bufferFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		// Add the size of the vertices of each model to the total buffer size
		std::vector<Vertex> vertices;
		VkDeviceSize bufferSize = 0;

        uint32_t vertexOffset = 0;
        uint32_t indexOffset = 0;

		for (Model* model : m_pModels)
		{
            model->SetVertexOffset(vertexOffset);
            model->SetFirstIndex(indexOffset);

            vertexOffset += static_cast<uint32_t>(model->GetVertices().size());
            indexOffset += static_cast<uint32_t>(model->GetIndices().size());

			bufferSize += sizeof(model->GetVertices()[0]) * model->GetVertices().size();
			
			for (const Vertex& vertex : model->GetVertices())
			{
				vertices.push_back(vertex);
			}
		}

        m_pVertexBuffer = new Buffer(bufferSize, bufferFlags, propertyFlags, vertices.data(), m_pDevice, m_pCommandPool);
    }

    void CreateIndexBuffer()
    {
        VkBufferUsageFlags bufferFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        //VkDeviceSize bufferSize = sizeof(m_pModel->GetIndices()[0]) * m_pModel->GetIndices().size();

        // Add the size of the vertices of each model to the total buffer size
        VkDeviceSize bufferSize = 0;
        std::vector<uint32_t> indices;
        for (Model* model : m_pModels)
        {
            bufferSize += sizeof(model->GetIndices()[0]) * model->GetIndices().size();

            for (const uint32_t index : model->GetIndices())
            {
                indices.push_back(index);
            }
        }

		m_pIndexBuffer = new Buffer(bufferSize, bufferFlags, propertyFlags, indices.data(), m_pDevice, m_pCommandPool);
    }

    void CreateUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        m_UniformBuffers.resize(g_MAX_FRAMES_IN_FLIGHT);
        m_UniformBuffersMapped.resize(g_MAX_FRAMES_IN_FLIGHT);

        for (size_t i{}; i < g_MAX_FRAMES_IN_FLIGHT; ++i)
        {
			m_UniformBuffers[i] = new Buffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_pDevice, m_pCommandPool, &m_UniformBuffersMapped[i]);
        }
    }

    void CreateDescriptorPool()
    {
		m_pDescriptorPool = new DescriptorPool(g_MAX_FRAMES_IN_FLIGHT, m_pModels.size(), m_pDevice);
    }

    void CreateDescriptorSets()
    {
		// Create descriptor sets for each model
		for (Model* model : m_pModels)
		{
            model->SetDescriptorSets(new DescriptorSets(g_MAX_FRAMES_IN_FLIGHT, m_pDevice, m_pDescriptorSetLayout->GetDescriptorSetLayout(), m_pDescriptorPool->GetDescriptorPool(), m_UniformBuffers, *model->GetTexture()->GetImageView(), *model->GetTexture()->GetSampler()));
		}
    }

    void CreateCommandBuffers()
    {
        m_pCommandBuffers = new CommandBuffers(m_pDevice, m_pCommandPool, g_MAX_FRAMES_IN_FLIGHT);
    }

    void CreateSyncObjects()
    {
        m_ImageAvailableSemaphores.resize(g_MAX_FRAMES_IN_FLIGHT);
        m_RenderFinishedSemaphores.resize(g_MAX_FRAMES_IN_FLIGHT);
        m_InFlightFences.resize(g_MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i{}; i < g_MAX_FRAMES_IN_FLIGHT; ++i)
        {
            if (vkCreateSemaphore(m_pDevice->GetVkDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_pDevice->GetVkDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_pDevice->GetVkDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    void RecreateSwapChain()
    {
        int width = 0;
        int height = 0;
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(m_pWindow->GetGLFWWindow(), &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(m_pDevice->GetVkDevice());

        CleanupSwapChain();

        CreateSwapChain();
        CreateImageViews();
		CreateDepthImage();
        m_pSwapchain->CreateFramebuffers(m_pRenderPass->GetRenderPass(), *m_pDepthImage->GetImageView());
    }

    void CleanupSwapChain()
    {
		m_pSwapchain->CleanupSwapChain(m_pDepthImage);
    }

    void MainLoop()
    {
		m_Timer.Start();
        while (!glfwWindowShouldClose(m_pWindow->GetGLFWWindow()))
        {
            glfwPollEvents();
            DrawFrame();
            m_Timer.Update();
			m_pCamera->Update(m_Timer.GetElapsed());
        }
		m_Timer.Stop();

        vkDeviceWaitIdle(m_pDevice->GetVkDevice());
    }

    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_pRenderPass->GetRenderPass();
        renderPassInfo.framebuffer = m_pSwapchain->GetSwapChainFramebuffers()[imageIndex];

        renderPassInfo.renderArea.offset = { 0, 0 };
		auto swapChainExtent = m_pSwapchain->GetSwapchainExtent();
        renderPassInfo.renderArea.extent = swapChainExtent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pGraphicsPipeline->GetGraphicsPipeline());

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(swapChainExtent.width);
            viewport.height = static_cast<float>(swapChainExtent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = { 0, 0 };
            scissor.extent = swapChainExtent;
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            VkBuffer vertexBuffers[] = { m_pVertexBuffer->GetBuffer() };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffer, m_pIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
            
            // Draw all models
            for (Model* model : m_pModels)
            {
                uint32_t indexCount = static_cast<uint32_t>(model->GetIndices().size());
                uint32_t firstIndex = model->GetFirstIndex();
                int32_t vertexOffset = static_cast<int32_t>(model->GetVertexOffset());

                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pGraphicsPipeline->GetPipelineLayout()->GetPipelineLayout(), 0, 1, &model->GetDescriptorSets()->GetDescriptorSets()[m_CurrentFrame], 0, nullptr);

                vkCmdDrawIndexed(
                    commandBuffer,
                    indexCount,
                    1,
                    firstIndex,
                    vertexOffset,
                    0
                );
            }

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void DrawFrame()
    {
        vkWaitForFences(m_pDevice->GetVkDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_pDevice->GetVkDevice(), m_pSwapchain->GetSwapchain(), UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            RecreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        UpdateUniformBuffer(m_CurrentFrame);

        vkResetFences(m_pDevice->GetVkDevice(), 1, &m_InFlightFences[m_CurrentFrame]);

        vkResetCommandBuffer(m_pCommandBuffers->GetCommandBuffers()[m_CurrentFrame], 0);
		RecordCommandBuffer(m_pCommandBuffers->GetCommandBuffers()[m_CurrentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_pCommandBuffers->GetCommandBuffers()[m_CurrentFrame];

        VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(m_pDevice->GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { m_pSwapchain->GetSwapchain()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

        result = vkQueuePresentKHR(m_pDevice->GetPresentQueue(), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_pWindow->IsFramebufferResized())
        {
            m_pWindow->ResetFramebufferResized();
            RecreateSwapChain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        m_CurrentFrame = (m_CurrentFrame + 1) % g_MAX_FRAMES_IN_FLIGHT;
    }

    void UpdateUniformBuffer(uint32_t currentImage)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::mat4(1.0f);
		//ubo.view = m_pCamera->viewMatrix.GetMat4();
        //ubo.view = glm::lookAt(m_pCamera->GetOrigin(), m_pCamera->GetOrigin() + m_pCamera->forward, m_pCamera->up);
		ubo.view = m_pCamera->viewMatrix;
        //ubo.proj = m_pCamera->projectionMatrix.GetMat4();
        //ubo.proj = glm::perspective(m_pCamera->GetFov(), m_pCamera->GetAspectRatio(), 0.1f, 25.0f);
		ubo.proj = m_pCamera->projectionMatrix;
        ubo.proj[1][1] *= -1;

        memcpy(m_UniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

    void Cleanup()
    {
		for (Model* model : m_pModels)
		{
			delete model;
            model = nullptr;
		}

		m_pSwapchain->CleanupSwapChain(m_pDepthImage);
		delete m_pSwapchain;

        for (size_t i{}; i < g_MAX_FRAMES_IN_FLIGHT; ++i)
        {
			delete m_UniformBuffers[i];
        }

        delete m_pDescriptorPool;

        delete m_pDescriptorSetLayout;

        delete m_pIndexBuffer;
		delete m_pVertexBuffer;

		delete m_pGraphicsPipeline;

		delete m_pRenderPass;

        for (size_t i{}; i < g_MAX_FRAMES_IN_FLIGHT; ++i)
        {
            vkDestroySemaphore(m_pDevice->GetVkDevice(), m_RenderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(m_pDevice->GetVkDevice(), m_ImageAvailableSemaphores[i], nullptr);
            vkDestroyFence(m_pDevice->GetVkDevice(), m_InFlightFences[i], nullptr);
        }

        delete m_pCommandPool;

        delete m_pDevice;

        delete m_pInstance;

        delete m_pWindow;
    }
};

int main()
{
    HelloTriangleApplication app;

    try 
    {
        app.Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
