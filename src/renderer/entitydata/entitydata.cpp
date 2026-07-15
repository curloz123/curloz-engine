#include "renderer/entitydata/entitydata.hpp"
#include "renderer/entitydata/vertexbuffer.hpp"
#include "renderer/entitydata/uvbuffer.hpp"
#include "renderer/entitydata/indexbuffer.hpp"
#include "renderer/entitydata/texture.hpp"

namespace clz::renderer
{
	/// @brief Destroys all stored entity data in memory
	void destroyEntityData()
	{
		destroyTextures();
		UVBuffer::destroyUVBuffer();
		IBuffer::destroyIndexBuffer();
		VBuffer::destroyVertexBuffer();
	}
}
