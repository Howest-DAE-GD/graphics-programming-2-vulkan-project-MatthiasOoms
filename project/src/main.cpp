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
    RenderPass* m_pDepthRenderPass;
    RenderPass* m_pDeferredRenderPass;
    RenderPass* m_pCombineRenderPass;
	GraphicsPipeline* m_pGraphicsPipeline;
	GraphicsPipeline* m_pDepthGraphicsPipeline;
	GraphicsPipeline* m_pDeferredGraphicsPipeline;
    GraphicsPipeline* m_pTransparentGraphicsPipeline;
    GraphicsPipeline* m_pCombineGraphicsPipeline;
    
	DescriptorSetLayout* m_pDescriptorSetLayout;

	CommandPool* m_pCommandPool;

	CommandBuffers* m_pCommandBuffers;

    std::vector<VkSemaphore> m_ImageAvailableSemaphores;
    std::vector<VkSemaphore> m_RenderFinishedSemaphores;
    std::vector<VkFence> m_InFlightFences;
    uint32_t m_CurrentFrame = 0;

	std::vector<Model*> m_pOpaqueModels;
	std::vector<Model*> m_pTransparentModels;

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
        CreateCommandPool();
        CreateSwapChain();
        CreateImageViews();
        CreateRenderPass();
        CreateDescriptorSetLayout();
        CreateGraphicsPipeline();
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
		m_pSwapchain = new Swapchain(m_pPhysicalDevice, m_pDevice, m_pInstance, m_pCommandPool);
    }

    void CreateImageViews()
    {
		m_pSwapchain->CreateImageViews();
    }

    void CreateRenderPass()
    {
        auto& albedoImage = m_pSwapchain->GetGBufferAlbedoImages();
		auto& normalImage = m_pSwapchain->GetGBufferNormalImages();
		auto& metalRoughImage = m_pSwapchain->GetGBufferMetalRoughImages();

		m_pCombineRenderPass = new RenderPass(m_pDevice, m_pSwapchain->GetSwapChainImageFormat(), FindDepthFormat(), false);
		m_pDeferredRenderPass = new RenderPass(m_pDevice, *albedoImage[0]->GetImageFormat(), *normalImage[0]->GetImageFormat(), *metalRoughImage[0]->GetImageFormat(), FindDepthFormat());
		m_pDepthRenderPass = new RenderPass(m_pDevice, FindDepthFormat());
		m_pRenderPass = new RenderPass(m_pDevice, m_pSwapchain->GetSwapChainImageFormat(), FindDepthFormat(), true);
    }

    void CreateDescriptorSetLayout()
    {
		m_pDescriptorSetLayout = new DescriptorSetLayout(m_pDevice);
    }

    void CreateGraphicsPipeline()
    {
		m_pCombineGraphicsPipeline = new GraphicsPipeline(m_pDevice, m_pCombineRenderPass, m_pDescriptorSetLayout->GetDescriptorSetLayout(), "resources/shaders/combineVert.spv", "resources/shaders/combineFrag.spv");
		m_pTransparentGraphicsPipeline = new GraphicsPipeline(m_pDevice, m_pRenderPass, m_pDescriptorSetLayout->GetDescriptorSetLayout(), "resources/shaders/vert.spv", "resources/shaders/frag.spv", true);
		m_pDeferredGraphicsPipeline = new GraphicsPipeline(m_pDevice, m_pDeferredRenderPass, m_pDescriptorSetLayout->GetDescriptorSetLayout(), "resources/shaders/deferredVert.spv", "resources/shaders/deferredFrag.spv");
		m_pDepthGraphicsPipeline = new GraphicsPipeline(m_pDevice, m_pDepthRenderPass, m_pDescriptorSetLayout->GetDescriptorSetLayout(), "resources/shaders/depth.spv");
		m_pGraphicsPipeline = new GraphicsPipeline(m_pDevice, m_pRenderPass, m_pDescriptorSetLayout->GetDescriptorSetLayout(), "resources/shaders/vert.spv", "resources/shaders/frag.spv");
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
		VkImageAspectFlagBits aspects = VK_IMAGE_ASPECT_DEPTH_BIT;
		VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageLayout newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		m_pDepthImage = new Image(m_pDevice, m_pCommandPool, swapchainExtent, FindDepthFormat(), tiling, usage, properties, aspects, oldLayout, newLayout);
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
        m_pSwapchain->CreateDepthFramebuffers(m_pDepthRenderPass->GetRenderPass(), *m_pDepthImage->GetImageView());
		m_pSwapchain->CreateDeferredFramebuffers(m_pDeferredRenderPass->GetRenderPass(), *m_pDepthImage->GetImageView());
	}

    void CreateTextureImage()
    {
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		VkImageUsageFlagBits usage = static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		VkMemoryPropertyFlagBits properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        
        // Loop over all meshes and create a texture for each one
		for (Model* pModel : m_pOpaqueModels)
		{
			// Diffuse texture
			if (pModel->GetDiffuseTexturePath().empty())
			{
                // Create a default texture for the model
                pModel->SetDiffuseTexture(new Texture(m_pDevice, m_pCommandPool, m_pSwapchain->GetSwapchainExtent(), m_pSwapchain->GetSwapChainImageFormat(), tiling, usage, properties, "resources/models/white.png"));
                pModel->GetDiffuseTexture()->CreateSampler(m_pPhysicalDevice->GetVkPhysicalDevice());
			}
            else
            {
                // Create a texture for the model
                pModel->SetDiffuseTexture(new Texture(m_pDevice, m_pCommandPool, m_pSwapchain->GetSwapchainExtent(), m_pSwapchain->GetSwapChainImageFormat(), tiling, usage, properties, pModel->GetDiffuseTexturePath()));
                pModel->GetDiffuseTexture()->CreateSampler(m_pPhysicalDevice->GetVkPhysicalDevice());
            }

			// Normal texture
            if (pModel->GetNormalTexturePath().empty())
            {
                // Create a default normal for the model
                pModel->SetNormalTexture(new Texture(m_pDevice, m_pCommandPool, m_pSwapchain->GetSwapchainExtent(), VK_FORMAT_R8G8B8A8_UNORM, tiling, usage, properties, "resources/models/white.png"));
                pModel->GetNormalTexture()->CreateSampler(m_pPhysicalDevice->GetVkPhysicalDevice());
            }
            else
            {
                // Create a normal for the model
                pModel->SetNormalTexture(new Texture(m_pDevice, m_pCommandPool, m_pSwapchain->GetSwapchainExtent(), VK_FORMAT_R8G8B8A8_UNORM, tiling, usage, properties, pModel->GetNormalTexturePath()));
                pModel->GetNormalTexture()->CreateSampler(m_pPhysicalDevice->GetVkPhysicalDevice());
            }

			// MetalRough texture
            if (pModel->GetMetalRoughTexturePath().empty())
            {
                // Create a default texture for the model
                pModel->SetMetalRoughTexture(new Texture(m_pDevice, m_pCommandPool, m_pSwapchain->GetSwapchainExtent(), m_pSwapchain->GetSwapChainImageFormat(), tiling, usage, properties, "resources/models/white.png"));
                pModel->GetMetalRoughTexture()->CreateSampler(m_pPhysicalDevice->GetVkPhysicalDevice());
            }
            else
            {
                // Create a texture for the model
                pModel->SetMetalRoughTexture(new Texture(m_pDevice, m_pCommandPool, m_pSwapchain->GetSwapchainExtent(), m_pSwapchain->GetSwapChainImageFormat(), tiling, usage, properties, pModel->GetMetalRoughTexturePath()));
                pModel->GetMetalRoughTexture()->CreateSampler(m_pPhysicalDevice->GetVkPhysicalDevice());
            }
        }

        // Loop over all meshes and create a texture for each one
        for (Model* pModel : m_pTransparentModels)
        {
            // Diffuse texture
            if (pModel->GetDiffuseTexturePath().empty())
            {
                // Create a default texture for the model
                pModel->SetDiffuseTexture(new Texture(m_pDevice, m_pCommandPool, m_pSwapchain->GetSwapchainExtent(), m_pSwapchain->GetSwapChainImageFormat(), tiling, usage, properties, "resources/models/white.png"));
                pModel->GetDiffuseTexture()->CreateSampler(m_pPhysicalDevice->GetVkPhysicalDevice());
            }
            else
            {
                // Create a texture for the model
                pModel->SetDiffuseTexture(new Texture(m_pDevice, m_pCommandPool, m_pSwapchain->GetSwapchainExtent(), m_pSwapchain->GetSwapChainImageFormat(), tiling, usage, properties, pModel->GetDiffuseTexturePath()));
                pModel->GetDiffuseTexture()->CreateSampler(m_pPhysicalDevice->GetVkPhysicalDevice());
            }

            // Normal texture
            if (pModel->GetNormalTexturePath().empty())
            {
                // Create a default normal for the model
                pModel->SetNormalTexture(new Texture(m_pDevice, m_pCommandPool, m_pSwapchain->GetSwapchainExtent(), VK_FORMAT_R8G8B8A8_UNORM, tiling, usage, properties, "resources/models/white.png"));
                pModel->GetNormalTexture()->CreateSampler(m_pPhysicalDevice->GetVkPhysicalDevice());
            }
            else
            {
                // Create a normal for the model
                pModel->SetNormalTexture(new Texture(m_pDevice, m_pCommandPool, m_pSwapchain->GetSwapchainExtent(), VK_FORMAT_R8G8B8A8_UNORM, tiling, usage, properties, pModel->GetNormalTexturePath()));
                pModel->GetNormalTexture()->CreateSampler(m_pPhysicalDevice->GetVkPhysicalDevice());
            }

            // MetalRough texture
            if (pModel->GetMetalRoughTexturePath().empty())
            {
                // Create a default texture for the model
                pModel->SetMetalRoughTexture(new Texture(m_pDevice, m_pCommandPool, m_pSwapchain->GetSwapchainExtent(), m_pSwapchain->GetSwapChainImageFormat(), tiling, usage, properties, "resources/models/white.png"));
                pModel->GetMetalRoughTexture()->CreateSampler(m_pPhysicalDevice->GetVkPhysicalDevice());
            }
            else
            {
                // Create a texture for the model
                pModel->SetMetalRoughTexture(new Texture(m_pDevice, m_pCommandPool, m_pSwapchain->GetSwapchainExtent(), m_pSwapchain->GetSwapChainImageFormat(), tiling, usage, properties, pModel->GetMetalRoughTexturePath()));
                pModel->GetMetalRoughTexture()->CreateSampler(m_pPhysicalDevice->GetVkPhysicalDevice());
            }
        }
    }

    void LoadModels()
    {
		ModelLoader modelLoader{};

        for (Model* pModel : modelLoader.LoadModel(g_MODEL_PATH))
        {
            if (!pModel->IsTransparent())
            {
                m_pOpaqueModels.push_back(pModel);
            }
            else
            {
                m_pTransparentModels.push_back(pModel);
            }
        }
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

		for (Model* pModel : m_pOpaqueModels)
		{
            pModel->SetVertexOffset(vertexOffset);
            pModel->SetFirstIndex(indexOffset);

            vertexOffset += static_cast<uint32_t>(pModel->GetVertices().size());
            indexOffset += static_cast<uint32_t>(pModel->GetIndices().size());

			bufferSize += sizeof(pModel->GetVertices()[0]) * pModel->GetVertices().size();
			
			for (const Vertex& vertex : pModel->GetVertices())
			{
				vertices.push_back(vertex);
			}
		}

        for (Model* pModel : m_pTransparentModels)
        {
            pModel->SetVertexOffset(vertexOffset);
            pModel->SetFirstIndex(indexOffset);

            vertexOffset += static_cast<uint32_t>(pModel->GetVertices().size());
            indexOffset += static_cast<uint32_t>(pModel->GetIndices().size());

            bufferSize += sizeof(pModel->GetVertices()[0]) * pModel->GetVertices().size();

            for (const Vertex& vertex : pModel->GetVertices())
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

        // Add the size of the vertices of each model to the total buffer size
        VkDeviceSize bufferSize = 0;
        std::vector<uint32_t> indices;
        for (Model* pModel : m_pOpaqueModels)
        {
            bufferSize += sizeof(pModel->GetIndices()[0]) * pModel->GetIndices().size();

            for (const uint32_t index : pModel->GetIndices())
            {
                indices.push_back(index);
            }
        }

        for (Model* pModel : m_pTransparentModels)
        {
            bufferSize += sizeof(pModel->GetIndices()[0]) * pModel->GetIndices().size();

            for (const uint32_t index : pModel->GetIndices())
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
		m_pDescriptorPool = new DescriptorPool(g_MAX_FRAMES_IN_FLIGHT, m_pOpaqueModels.size() + m_pTransparentModels.size(), m_pDevice);
    }

    void CreateDescriptorSets()
    {
		// Create descriptor sets for each model
		for (Model* pModel : m_pOpaqueModels)
		{
            pModel->SetDescriptorSets(new DescriptorSets(g_MAX_FRAMES_IN_FLIGHT, m_pDevice, m_pDescriptorSetLayout->GetDescriptorSetLayout(), m_pDescriptorPool->GetDescriptorPool(), m_UniformBuffers, pModel, m_pSwapchain->GetGBufferAlbedoImages()[0], m_pSwapchain->GetGBufferNormalImages()[0], m_pSwapchain->GetGBufferMetalRoughImages()[0]));
		}

        for (Model* pModel : m_pTransparentModels)
        {
            pModel->SetDescriptorSets(new DescriptorSets(g_MAX_FRAMES_IN_FLIGHT, m_pDevice, m_pDescriptorSetLayout->GetDescriptorSetLayout(), m_pDescriptorPool->GetDescriptorPool(), m_UniformBuffers, pModel, m_pSwapchain->GetGBufferAlbedoImages()[0], m_pSwapchain->GetGBufferNormalImages()[0], m_pSwapchain->GetGBufferMetalRoughImages()[0]));
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

		CreateFrameBuffers();

		// Update descriptor sets with new image views
		for (Model* pModel : m_pOpaqueModels)
		{
			pModel->GetDescriptorSets()->UpdateDescriptorSets(m_pSwapchain->GetGBufferAlbedoImages()[0], m_pSwapchain->GetGBufferNormalImages()[0], m_pSwapchain->GetGBufferMetalRoughImages()[0]);
		}
		for (Model* pModel : m_pTransparentModels)
		{
			pModel->GetDescriptorSets()->UpdateDescriptorSets(m_pSwapchain->GetGBufferAlbedoImages()[0], m_pSwapchain->GetGBufferNormalImages()[0], m_pSwapchain->GetGBufferMetalRoughImages()[0]);
		}
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

        auto swapChainExtent = m_pSwapchain->GetSwapchainExtent();

        VkRenderPassBeginInfo depthPrePassInfo{};
        depthPrePassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        depthPrePassInfo.renderPass = m_pDepthRenderPass->GetRenderPass();
        depthPrePassInfo.framebuffer = m_pSwapchain->GetSwapChainDepthFramebuffers()[imageIndex];

        depthPrePassInfo.renderArea.offset = { 0, 0 };
        depthPrePassInfo.renderArea.extent = swapChainExtent;

        VkClearValue depthClear{};
        depthClear.depthStencil = { 1.0f, 0 }; // Clear depth to farthest value (1.0)

        depthPrePassInfo.clearValueCount = 1;
        depthPrePassInfo.pClearValues = &depthClear;

        vkCmdBeginRenderPass(commandBuffer, &depthPrePassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pDepthGraphicsPipeline->GetGraphicsPipeline());

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

            for (Model* pModel : m_pOpaqueModels)
            {
                uint32_t indexCount = static_cast<uint32_t>(pModel->GetIndices().size());
                uint32_t firstIndex = pModel->GetFirstIndex();
                int32_t vertexOffset = static_cast<int32_t>(pModel->GetVertexOffset());

                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    m_pDepthGraphicsPipeline->GetPipelineLayout()->GetPipelineLayout(), 0, 1,
                    &pModel->GetDescriptorSets()->GetDescriptorSets()[m_CurrentFrame], 0, nullptr);

                vkCmdDrawIndexed(commandBuffer, indexCount, 1, firstIndex, vertexOffset, 0);
            }

        vkCmdEndRenderPass(commandBuffer);

        VkRenderPassBeginInfo deferredRenderPassInfo{};
        deferredRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        deferredRenderPassInfo.renderPass = m_pDeferredRenderPass->GetRenderPass();
        deferredRenderPassInfo.framebuffer = m_pSwapchain->GetSwapChainDeferredFramebuffers()[imageIndex];

        deferredRenderPassInfo.renderArea.offset = { 0, 0 };
        deferredRenderPassInfo.renderArea.extent = swapChainExtent;

        std::vector<VkClearValue> deferredClearValues{};
        deferredClearValues.resize(m_pDeferredRenderPass->GetAttachmentCount(), { 0.0f, 0.0f, 0.0f, 1.0f });

        deferredRenderPassInfo.clearValueCount = static_cast<uint32_t>(deferredClearValues.size());
        deferredRenderPassInfo.pClearValues = deferredClearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &deferredRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pDeferredGraphicsPipeline->GetGraphicsPipeline());

            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffer, m_pIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

            // Draw all models
            for (Model* pModel : m_pOpaqueModels)
            {
                uint32_t indexCount = static_cast<uint32_t>(pModel->GetIndices().size());
                uint32_t firstIndex = pModel->GetFirstIndex();
                int32_t vertexOffset = static_cast<int32_t>(pModel->GetVertexOffset());

                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pDeferredGraphicsPipeline->GetPipelineLayout()->GetPipelineLayout(), 0, 1, &pModel->GetDescriptorSets()->GetDescriptorSets()[m_CurrentFrame], 0, nullptr);

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

		// Transition images from color attachment to shader read only
        auto& albedoImage = m_pSwapchain->GetGBufferAlbedoImages();
        auto& normalImage = m_pSwapchain->GetGBufferNormalImages();
        auto& metalRoughImage = m_pSwapchain->GetGBufferMetalRoughImages();

        // Transition G-buffer images to SHADER_READ_ONLY_OPTIMAL
        for (size_t i = 0; i < albedoImage.size(); ++i)
        {
            albedoImage[i]->TransitionImageLayout(commandBuffer, *albedoImage[i]->GetImageFormat(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			normalImage[i]->TransitionImageLayout(commandBuffer, *normalImage[i]->GetImageFormat(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			metalRoughImage[i]->TransitionImageLayout(commandBuffer, *metalRoughImage[i]->GetImageFormat(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }

        VkRenderPassBeginInfo combineRenderPassInfo{};
        combineRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        combineRenderPassInfo.renderPass = m_pCombineRenderPass->GetRenderPass();
        combineRenderPassInfo.framebuffer = m_pSwapchain->GetSwapChainFramebuffers()[imageIndex];

        combineRenderPassInfo.renderArea.offset = { 0, 0 };
        combineRenderPassInfo.renderArea.extent = swapChainExtent;

        std::vector<VkClearValue> combinedClearValues{};
        combinedClearValues.resize(m_pCombineRenderPass->GetAttachmentCount(), { 0.0f, 0.0f, 0.0f, 1.0f });

        combineRenderPassInfo.clearValueCount = static_cast<uint32_t>(combinedClearValues.size());
        combineRenderPassInfo.pClearValues = combinedClearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &combineRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pCombineGraphicsPipeline->GetGraphicsPipeline());

            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffer, m_pIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

            PushConstants pc = { glm::vec4(swapChainExtent.width, swapChainExtent.height, 0, 0), glm::vec4(m_pCamera->forward, 0) };

            vkCmdPushConstants(
                commandBuffer,
                m_pCombineGraphicsPipeline->GetPipelineLayout()->GetPipelineLayout(),
                VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(PushConstants),
                &pc
            );

            // Draw all models
            for (Model* pModel : m_pOpaqueModels)
            {
                uint32_t indexCount = static_cast<uint32_t>(pModel->GetIndices().size());
                uint32_t firstIndex = pModel->GetFirstIndex();
                int32_t vertexOffset = static_cast<int32_t>(pModel->GetVertexOffset());

                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pCombineGraphicsPipeline->GetPipelineLayout()->GetPipelineLayout(), 0, 1, &pModel->GetDescriptorSets()->GetDescriptorSets()[m_CurrentFrame], 0, nullptr);

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

        VkRenderPassBeginInfo transparentRenderPassInfo{};
        transparentRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        transparentRenderPassInfo.renderPass = m_pRenderPass->GetRenderPass();
        transparentRenderPassInfo.framebuffer = m_pSwapchain->GetSwapChainFramebuffers()[imageIndex];

        transparentRenderPassInfo.renderArea.offset = { 0, 0 };
        transparentRenderPassInfo.renderArea.extent = swapChainExtent;

        std::vector<VkClearValue> transparentClearValues{};
        transparentClearValues.resize(m_pRenderPass->GetAttachmentCount(), { 0.0f, 0.0f, 0.0f, 1.0f });

        transparentRenderPassInfo.clearValueCount = static_cast<uint32_t>(transparentClearValues.size());
        transparentRenderPassInfo.pClearValues = transparentClearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &transparentRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffer, m_pIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pTransparentGraphicsPipeline->GetGraphicsPipeline());

            // TODO: Sort transparent models by distance from camera before drawing

            // Draw all models
            for (Model* pModel : m_pTransparentModels)
            {
                uint32_t indexCount = static_cast<uint32_t>(pModel->GetIndices().size());
                uint32_t firstIndex = pModel->GetFirstIndex();
                int32_t vertexOffset = static_cast<int32_t>(pModel->GetVertexOffset());

                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pTransparentGraphicsPipeline->GetPipelineLayout()->GetPipelineLayout(), 0, 1, &pModel->GetDescriptorSets()->GetDescriptorSets()[m_CurrentFrame], 0, nullptr);

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

		// Transition G-buffer images to COLOR_ATTACHMENT_OPTIMAL
        for (size_t i = 0; i < albedoImage.size(); ++i)
        {
			albedoImage[i]->TransitionImageLayout(commandBuffer, *albedoImage[i]->GetImageFormat(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

			normalImage[i]->TransitionImageLayout(commandBuffer, *normalImage[i]->GetImageFormat(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

			metalRoughImage[i]->TransitionImageLayout(commandBuffer, *metalRoughImage[i]->GetImageFormat(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        }

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

        VkSwapchainKHR swapChains[] = { m_pSwapchain->GetSwapchain() };
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
		for (Model* pModel : m_pOpaqueModels)
		{
			delete pModel;
            pModel = nullptr;
		}

		for (Model* pModel : m_pTransparentModels)
		{
			delete pModel;
            pModel = nullptr;
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

		delete m_pCombineGraphicsPipeline;
        delete m_pTransparentGraphicsPipeline;
		delete m_pDeferredGraphicsPipeline;
		delete m_pDepthGraphicsPipeline;
		delete m_pGraphicsPipeline;

		delete m_pCombineRenderPass;
		delete m_pDeferredRenderPass;
		delete m_pDepthRenderPass;
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
