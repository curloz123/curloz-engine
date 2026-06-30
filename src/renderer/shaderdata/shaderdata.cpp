#include "renderer/shaderdata/shaderdata.hpp"
#include "core/logs.hpp"
#include "renderer/shaderdata/bufferarray/indexbuffer.hpp"
#include "renderer/shaderdata/bufferarray/vertexbuffer.hpp"
#include "renderer/shaderdata/bufferarray/uvbuffer.hpp"
#include "renderer/shaderdata/descriptor/descriptor.hpp"
#include "renderer/shaderdata/descriptor/texture.hpp"

namespace clz::renderer
{
	bool initShaderData()
	{
		// Init vertexbuffer and indexbuffer should go here

		if (!initDescriptors())
		{
			clz::log::error("Could not initialize descriptors");
			return false;
		}

		return true;
	}

	bool createEntityData()
	{
		if (!VBuffer::submitVertexBuffer())
		{
			clz::log::error("Could not submit vertex buffer");
			return false;
		}

		if (!IBuffer::submitIndexBuffer())
		{
			clz::log::error("Could not submit index buffer");
			return false;
		}

		if (!UVBuffer::submitUVBuffer())
		{
			clz::log::error("Could not submit UV buffer");
			return false;
		}

		if (!createTextures())
		{
			clz::log::error("Could not create textures");
			return false;
		}
		
		if (!createDescriptors())
		{
			clz::log::error("Could not intiialize descrtiptors");
			return false;
		}

		return true;
	}

	void updateShaderData(VkCommandBuffer commandBuffer, uint32_t currentFrame)
	{
		updateDescriptors(commandBuffer, currentFrame);
	}

	void destroyShaderData()
	{
		destroyDescriptors();
		UVBuffer::destroyUVBuffer();
		IBuffer::destroyIndexBuffer();
		VBuffer::destroyVertexBuffer();
	}
}
