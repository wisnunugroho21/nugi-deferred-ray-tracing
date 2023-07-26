#include "texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <cmath>

#include "../buffer/buffer.hpp"
#include "../command/command_buffer.hpp"

namespace nugiEngine {
  EngineTexture::EngineTexture(EngineDevice &appDevice, const char* textureFileName, VkFilter filterMode, VkSamplerAddressMode addressMode, 
    VkBool32 anistropyEnable, VkBorderColor borderColor, VkCompareOp compareOp, VkSamplerMipmapMode mipmapMode) : appDevice{appDevice} 
  {
    this->createTextureImage(textureFileName);
    this->createTextureSampler(filterMode, addressMode, anistropyEnable, borderColor, compareOp, mipmapMode);
  }

  EngineTexture::EngineTexture(EngineDevice &appDevice, std::shared_ptr<EngineImage> image, VkFilter filterMode, VkSamplerAddressMode addressMode, 
    VkBool32 anistropyEnable, VkBorderColor borderColor, VkCompareOp compareOp, VkSamplerMipmapMode mipmapMode) : appDevice{appDevice}, image{image} 
  {
    this->createTextureSampler(filterMode, addressMode, anistropyEnable, borderColor, compareOp, mipmapMode);
  }

  EngineTexture::~EngineTexture() {
    vkDestroySampler(this->appDevice.getLogicalDevice(), this->sampler, nullptr);
  }

  void EngineTexture::createTextureImage(const char* textureFileName) {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(textureFileName, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
      throw std::runtime_error("failed to load texture image!");
    }

    this->mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    unsigned long pixelSize = 4;
    uint32_t pixelCount = texWidth * texHeight;

    EngineBuffer stagingBuffer {
			this->appDevice,
			pixelSize,
			pixelCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_AUTO,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
		};

    stagingBuffer.map();
    stagingBuffer.writeToBuffer(pixels);
    stagingBuffer.unmap();

    stbi_image_free(pixels);

    this->image = std::make_shared<EngineImage>(this->appDevice, texWidth, texHeight, this->mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, 
      VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
      VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, 
      VK_IMAGE_ASPECT_COLOR_BIT);

    this->image->transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 
      0, VK_ACCESS_TRANSFER_WRITE_BIT);
      
    stagingBuffer.copyBufferToImage(this->image->getImage(), static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);
    this->image->generateMipMap();
    // this->image->transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  }

  void EngineTexture::createTextureSampler(VkFilter filterMode, VkSamplerAddressMode addressMode, VkBool32 anistropyEnable, 
    VkBorderColor borderColor, VkCompareOp compareOp, VkSamplerMipmapMode mipmapMode) 
  {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = filterMode;
    samplerInfo.minFilter = filterMode;

    samplerInfo.addressModeU = addressMode;
    samplerInfo.addressModeV = addressMode;
    samplerInfo.addressModeW = addressMode;

    samplerInfo.anisotropyEnable = anistropyEnable;
    samplerInfo.maxAnisotropy = this->appDevice.getProperties().limits.maxSamplerAnisotropy;

    samplerInfo.borderColor = borderColor;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = compareOp;

    samplerInfo.mipmapMode = mipmapMode;
    samplerInfo.minLod = 0.0f; // Optional
    samplerInfo.maxLod = static_cast<float>(this->mipLevels);
    samplerInfo.mipLodBias = 0.0f; // Optional

    if (vkCreateSampler(this->appDevice.getLogicalDevice(), &samplerInfo, nullptr, &this->sampler) != VK_SUCCESS) {
      throw std::runtime_error("failed to create texture sampler!");
    }
  }

  VkDescriptorImageInfo EngineTexture::getDescriptorInfo(VkImageLayout desiredImageLayout) {
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = desiredImageLayout;
    imageInfo.imageView = this->image->getImageView();
    imageInfo.sampler = this->sampler;

    return imageInfo;
  }
} // namespace nugiEngine
