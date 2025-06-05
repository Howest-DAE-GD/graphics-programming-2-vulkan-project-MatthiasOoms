#include "Texture.h"

#include <stb_image.h>

#include "LogicalDevice.h"
#include "CommandBuffers.h"
#include "CommandPool.h"
#include "Buffer.h"
#include <stdexcept>

Texture::Texture(LogicalDevice* pDevice, CommandPool* pCommandPool, VkExtent2D swapchainExtent, VkFormat imageFormat, VkImageTiling tiling, VkImageUsageFlagBits usage, VkMemoryPropertyFlagBits properties, VkImageAspectFlagBits aspects, VkImageLayout oldLayout, VkImageLayout newLayout)
    : Image(pDevice, pCommandPool, swapchainExtent, imageFormat, tiling, usage, properties, aspects, oldLayout, newLayout)
{
}

// Do not call parent constructor
Texture::Texture(LogicalDevice* pDevice, CommandPool* pCommandPool, VkExtent2D swapchainExtent, VkFormat imageFormat, VkImageTiling tiling, VkImageUsageFlagBits usage, VkMemoryPropertyFlagBits properties, std::string texturePath)
    : Image()
{
	m_pDevice = pDevice;
	m_pCommandPool = pCommandPool;

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels)
    {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    Buffer::CreateBuffer(m_pDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_pDevice->GetVkDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_pDevice->GetVkDevice(), stagingBufferMemory);

    stbi_image_free(pixels);

    CreateImage(texWidth, texHeight, imageFormat, tiling, usage, properties, m_Image, m_ImageMemory);
    m_ImageView = CreateImageView(imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

    TransitionImageLayout(imageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyBufferToImage(stagingBuffer, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    TransitionImageLayout(imageFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(m_pDevice->GetVkDevice(), stagingBuffer, nullptr);
    vkFreeMemory(m_pDevice->GetVkDevice(), stagingBufferMemory, nullptr);
}

Texture::~Texture()
{
	vkDestroySampler(m_pDevice->GetVkDevice(), m_Sampler, nullptr);
}

void Texture::CreateSampler(VkPhysicalDevice pPhysicalDevice)
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(pPhysicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(m_pDevice->GetVkDevice(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void Texture::CopyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = CommandBuffers::BeginSingleTimeCommands(m_pDevice, m_pCommandPool);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        m_Image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    CommandBuffers::EndSingleTimeCommands(m_pDevice, m_pCommandPool, commandBuffer);
}
