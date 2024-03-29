/*
 * Encapsulates a vulkan buffer
 *
 * Initially based off VulkanBuffer by Sascha Willems -
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */
 
#include "buffer.hpp"
#include "../command/command_buffer.hpp"
 
// std
#include <cassert>
#include <cstring>
#include <memory>
 
namespace nugiEngine {
  /**
   * Returns the minimum instance size required to be compatible with devices minOffsetAlignment
   *
   * @param instanceSize The size of an instance
   * @param minOffsetAlignment The minimum required alignment, in bytes, for the offset member (eg
   * minUniformBufferOffsetAlignment)
   *
   * @return VkResult of the buffer mapping call
   */
  VkDeviceSize EngineBuffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
    if (minOffsetAlignment > 0) {
      return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
    }
    return instanceSize;
  }
  
  EngineBuffer::EngineBuffer(
      EngineDevice &device,
      VkDeviceSize instanceSize,
      uint32_t instanceCount,
      VkBufferUsageFlags bufferUsage,
      VmaMemoryUsage memoryUsageFlags,
      VmaAllocationCreateFlags memoryPropertyFlags,
      VkDeviceSize minOffsetAlignment
    )
      : engineDevice{device},
        instanceSize{instanceSize},
        instanceCount{instanceCount},
        usageFlags{usageFlags},
        memoryPropertyFlags{memoryPropertyFlags} 
  {
    this->alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
    this->bufferSize = alignmentSize * instanceCount;

    this->createBuffer(bufferSize, bufferUsage, memoryUsageFlags, memoryPropertyFlags);
  }
  
  EngineBuffer::~EngineBuffer() {
    this->unmap();
    vmaDestroyBuffer(this->engineDevice.getMemoryAllocator(), this->buffer, this->allocation);
  }
  
  /**
   * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
   *
   * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete
   * buffer range.
   * @param offset (Optional) Byte offset from beginning
   *
   * @return VkResult of the buffer mapping call
   */
  VkResult EngineBuffer::map(VkDeviceSize size, VkDeviceSize offset) {
    return vmaMapMemory(this->engineDevice.getMemoryAllocator(), this->allocation, &this->mapped);
  }
  
  /**
   * Unmap a mapped memory range
   *
   * @note Does not return a result as vkUnmapMemory can't fail
   */
  void EngineBuffer::unmap() {
    if (this->mapped) {
      vmaUnmapMemory(this->engineDevice.getMemoryAllocator(), this->allocation);
      this->mapped = nullptr;
    }
  }
  
  /**
   * Copies the specified data to the mapped buffer. Default value writes whole buffer range
   *
   * @param data Pointer to the data to copy
   * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer
   * range.
   * @param offset (Optional) Byte offset from beginning of mapped region
   *
   */
  void EngineBuffer::writeToBuffer(void *data, VkDeviceSize size, VkDeviceSize offset) {
    assert(this->mapped && "Cannot copy to unmapped buffer");
  
    if (size == VK_WHOLE_SIZE) {
      memcpy(this->mapped, data, this->bufferSize);
    } else {
      char *memOffset = (char *)this->mapped;
      memOffset += offset;
      memcpy(memOffset, data, size);
    }
  }

  /**
   * Copies the specified data from the mapped buffer. Default value writes whole buffer range
   *
   * @param data Pointer to the data to copy
   * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer
   * range.
   * @param offset (Optional) Byte offset from beginning of mapped region
   *
   */
  void EngineBuffer::readFromBuffer(void* data, VkDeviceSize size, VkDeviceSize offset) {
    assert(this->mapped && "Cannot copy from unmapped buffer");
  
    if (size == VK_WHOLE_SIZE) {
      memcpy(data, this->mapped, this->bufferSize);
    } else {
      char *memOffset = (char *)this->mapped;
      memOffset += offset;
      memcpy(data, memOffset, size);
    }
  }
  
  /**
   * Flush a memory range of the buffer to make it visible to the device
   *
   * @note Only required for non-coherent memory
   *
   * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the
   * complete buffer range.
   * @param offset (Optional) Byte offset from beginning
   *
   * @return VkResult of the flush call
   */
  VkResult EngineBuffer::flush(VkDeviceSize size, VkDeviceSize offset) {
    return vmaFlushAllocation(this->engineDevice.getMemoryAllocator(), this->allocation, offset, size);
  }
  
  /**
   * Invalidate a memory range of the buffer to make it visible to the host
   *
   * @note Only required for non-coherent memory
   *
   * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate
   * the complete buffer range.
   * @param offset (Optional) Byte offset from beginning
   *
   * @return VkResult of the invalidate call
   */
  VkResult EngineBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
    return vmaInvalidateAllocation(this->engineDevice.getMemoryAllocator(), this->allocation, offset, size);
  }
  
  /**
   * Create a buffer info descriptor
   *
   * @param size (Optional) Size of the memory range of the descriptor
   * @param offset (Optional) Byte offset from beginning
   *
   * @return VkDescriptorBufferInfo of specified offset and range
   */
  VkDescriptorBufferInfo EngineBuffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
    return VkDescriptorBufferInfo{
        this->buffer,
        offset,
        size,
    };
  }
  
  /**
   * Copies "instanceSize" bytes of data to the mapped buffer at an offset of index * alignmentSize
   *
   * @param data Pointer to the data to copy
   * @param index Used in offset calculation
   *
   */
  void EngineBuffer::writeToIndex(void *data, int index) {
    this->writeToBuffer(data, this->instanceSize, index * this->alignmentSize);
  }

  /**
   * Copies "instanceSize" bytes of data from the mapped buffer at an offset of index * alignmentSize
   *
   * @param data Pointer to the data to copy
   * @param index Used in offset calculation
   *
   */
  void EngineBuffer::readFromIndex(void* data, int index) {
    this->readFromBuffer(data, this->instanceSize, index * this->alignmentSize);
  }
  
  /**
   *  Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
   *
   * @param index Used in offset calculation
   *
   */
  VkResult EngineBuffer::flushIndex(int index) { 
    return this->flush(this->alignmentSize, index * this->alignmentSize); 
  }
  
  /**
   * Create a buffer info descriptor
   *
   * @param index Specifies the region given by index * alignmentSize
   *
   * @return VkDescriptorBufferInfo for instance at index
   */
  VkDescriptorBufferInfo EngineBuffer::descriptorInfoForIndex(int index) {
    return this->descriptorInfo(this->alignmentSize, index * this->alignmentSize);
  }
  
  /**
   * Invalidate a memory range of the buffer to make it visible to the host
   *
   * @note Only required for non-coherent memory
   *
   * @param index Specifies the region to invalidate: index * alignmentSize
   *
   * @return VkResult of the invalidate call
   */
  VkResult EngineBuffer::invalidateIndex(int index) {
    return this->invalidate(this->alignmentSize, index * this->alignmentSize);
  }

  void EngineBuffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags memoryPropertyFlags) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = bufferUsage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = memoryUsage;
    allocInfo.flags = memoryPropertyFlags;

    if (vmaCreateBuffer(this->engineDevice.getMemoryAllocator(), &bufferInfo, &allocInfo, &this->buffer, &this->allocation, &this->allocationInfo) != VK_SUCCESS) {
      throw std::runtime_error("failed to create buffer!");
    }
  }

  void EngineBuffer::copyBuffer(VkBuffer srcBuffer, VkDeviceSize size, std::shared_ptr<EngineCommandBuffer> commandBuffer) {
    bool isCommandBufferCreatedHere = false;
    
    if (commandBuffer == nullptr) {
      commandBuffer = std::make_shared<EngineCommandBuffer>(this->engineDevice);
      commandBuffer->beginSingleTimeCommand();

      isCommandBufferCreatedHere = true;  
    }

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;  // Optional
    copyRegion.dstOffset = 0;  // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer->getCommandBuffer(), srcBuffer, this->buffer, 1, &copyRegion);

    if (isCommandBufferCreatedHere) {
      commandBuffer->endCommand();
      commandBuffer->submitCommand(this->engineDevice.getTransferQueue(0));
    }
  }

  void EngineBuffer::copyBufferToImage(VkImage image, uint32_t width, uint32_t height, uint32_t layerCount, std::shared_ptr<EngineCommandBuffer> commandBuffer) {
    bool isCommandBufferCreatedHere = false;
    
    if (commandBuffer == nullptr) {
      commandBuffer = std::make_shared<EngineCommandBuffer>(this->engineDevice);
      commandBuffer->beginSingleTimeCommand();

      isCommandBufferCreatedHere = true;  
    }

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layerCount;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(
      commandBuffer->getCommandBuffer(),
      this->buffer,
      image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1,
      &region
    );

    if (isCommandBufferCreatedHere) {
      commandBuffer->endCommand();
      commandBuffer->submitCommand(this->engineDevice.getTransferQueue(0));
    }
  }
  
 
}  // namespace lve