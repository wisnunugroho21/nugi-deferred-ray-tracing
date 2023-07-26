#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>

#include <stb_image.h>

#include "../buffer/buffer.hpp"
#include "../command/command_buffer.hpp"
#include "../image/image.hpp"

#include <memory>

namespace nugiEngine
{
  class EngineTexture
  {
    public:
      EngineTexture(EngineDevice &appDevice, const char* textureFileName, VkFilter filterMode, VkSamplerAddressMode addressMode, 
        VkBool32 anistropyEnable, VkBorderColor borderColor, VkCompareOp compareOp, VkSamplerMipmapMode mipmapMode);
      EngineTexture(EngineDevice &appDevice, std::shared_ptr<EngineImage> image, VkFilter filterMode, VkSamplerAddressMode addressMode, 
        VkBool32 anistropyEnable, VkBorderColor borderColor, VkCompareOp compareOp, VkSamplerMipmapMode mipmapMode);

      ~EngineTexture();

      std::shared_ptr<EngineImage> getTextureImage() const { return this->image; }
      VkDescriptorImageInfo getDescriptorInfo(VkImageLayout desiredImageLayout);

    private:
      EngineDevice &appDevice;
      std::shared_ptr<EngineImage> image;

      VkSampler sampler;
      uint32_t mipLevels;

      void createTextureImage(const char* textureFileName);
      void createTextureSampler(VkFilter filterMode, VkSamplerAddressMode addressMode, VkBool32 anistropyEnable, 
        VkBorderColor borderColor, VkCompareOp compareOp, VkSamplerMipmapMode mipmapMode);
  };
  
  
} // namespace nugiEngine
