#pragma once

#include "core/logs.hpp"
#include "math/mat4x4.hpp"
#include "math/vec4.hpp"
#include "renderer/vk_types.hpp"
#include "entitydata/buffer.hpp"
#include "vulkan/vulkan.h"

namespace clz::renderer
{
	/// SHAPE enum
	enum class SHAPE : uint32_t
	{
		BOX = 0,
		SPHERE = 1,
		CYLINDER = 2,
		CAPSULE = 3
	};

	/// Shape vertex buffer data
	struct ShapeMesh
	{
		math::vec3 position;
		math::vec2 uv;
	};

	/// Shapes vertex buffer manager
	inline VertexBufferInterface<ShapeMesh> ShapeBufferManager;
	/// Shapes index buffer manager
	inline IndexBufferInterface ShapeIndicesManager;

	/// Initializes shapes
	/// basically loads all shapes.gltf from assets/shapes directory
	bool initShapes();

	/// Releases all resurces held by shapes
	void closeShapes();

	/// Draws a shape
	void drawShape(
		VkCommandBuffer commandBuffer,
		const SHAPE shape,
		const math::mat4& projection,
		const math::mat4& view,
		const math::mat4& model
	);

	inline VkVertexInputBindingDescription getShapeVertexBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(ShapeMesh);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	inline VkVertexInputAttributeDescription
	getShapeVertexAttributeDescription(const VertexAttributeType type)
	{
		VkVertexInputAttributeDescription attributeDescription{};
		attributeDescription.binding = 0;

		switch (type)
		{
		case VertexAttributeType::POSITION:
			attributeDescription.location = 0;
			attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescription.offset = offsetof(ShapeMesh, position);
			break;
		case VertexAttributeType::UV:
			attributeDescription.location = 1;
			attributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescription.offset = offsetof(ShapeMesh, uv);
			break;
		default:
			CLZ_ASSERT(
				false,
				"Attempt to retrieve vertex attribute description"
				" other than pos and uv for shape pipeline"
			);
		}

		return attributeDescription;
	}

} // namespace clz::renderer
