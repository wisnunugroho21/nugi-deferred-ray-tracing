#include "triangle_light_model.hpp"

#include <cstring>
#include <iostream>
#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace nugiEngine {
	EngineTriangleLightModel::EngineTriangleLightModel(EngineDevice &device, std::shared_ptr<std::vector<TriangleLight>> triangleLights, 
		std::shared_ptr<std::vector<Vertex>> vertices, std::shared_ptr<EngineCommandBuffer> commandBuffer) : engineDevice{device} 
	{
		std::vector<std::shared_ptr<BoundBox>> boundBoxes;

		for (int i = 0; i < triangleLights->size(); i++) {
			boundBoxes.push_back(std::make_shared<TriangleLightBoundBox>(TriangleLightBoundBox{ i + 1, (*triangleLights)[i], vertices}));
		}

		this->createBuffers(triangleLights, createBvh(boundBoxes), commandBuffer);
	}

	void EngineTriangleLightModel::createBuffers(std::shared_ptr<std::vector<TriangleLight>> triangleLights, std::shared_ptr<std::vector<BvhNode>> bvhNodes, 
		std::shared_ptr<EngineCommandBuffer> commandBuffer) 
	{
		auto triangleLightBufferSize = sizeof(TriangleLight) * triangleLights->size();
		
		EngineBuffer triangleLightStagingBuffer {
			this->engineDevice,
			static_cast<VkDeviceSize>(triangleLightBufferSize),
			1,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_AUTO,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
		};

		triangleLightStagingBuffer.map();
		triangleLightStagingBuffer.writeToBuffer(triangleLights->data());

		this->triangleLightBuffer = std::make_shared<EngineBuffer>(
			this->engineDevice,
			static_cast<VkDeviceSize>(triangleLightBufferSize),
			1,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_AUTO,
			VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
		);

		this->triangleLightBuffer->copyBuffer(triangleLightStagingBuffer.getBuffer(), static_cast<VkDeviceSize>(triangleLightBufferSize), commandBuffer);

		// -------------------------------------------------

		auto bvhBufferSize = sizeof(BvhNode) * bvhNodes->size();

		EngineBuffer bvhStagingBuffer {
			this->engineDevice,
			static_cast<VkDeviceSize>(bvhBufferSize),
			1,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_AUTO,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
		};

		bvhStagingBuffer.map();
		bvhStagingBuffer.writeToBuffer(bvhNodes->data());

		this->bvhBuffer = std::make_shared<EngineBuffer>(
			this->engineDevice,
			static_cast<VkDeviceSize>(bvhBufferSize),
			1,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_AUTO,
			VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
		);

		this->bvhBuffer->copyBuffer(bvhStagingBuffer.getBuffer(), static_cast<VkDeviceSize>(bvhBufferSize), commandBuffer);
	}
    
} // namespace nugiEngine

