/**
 * @file shapes.cpp
 * @brief Loads the primitive shapes (box/sphere/cylinder/capsule) from
 * assets/shapes/*.glb into dedicated vertex/index buffers and draws them.
 *
 * Each .glb is flattened into ONE vertex range + ONE index range
 * (all meshes/primitives merged, node transforms ignored), so a shape is
 * always a single draw call. Only POSITION, TEXCOORD_0 and indices are read.
 * The base colour texture (if any) is registered through TextureManager.
 */

#include "renderer/shapes.hpp"

#include "core/assert.hpp"
#include "core/logs.hpp"
#include "fastgltf/core.hpp"
#include "fastgltf/tools.hpp"
#include "fastgltf/types.hpp"
#include "renderer/entitydata/buffer.hpp"
#include "renderer/entitydata/texture.hpp"
#include "renderer/pipelinedata/pipeline_data.hpp"
#include "renderer/pipelinedata/pushconstants.hpp"
#include "renderer/utility/memory.hpp"
#include "renderer/utility/singletimecommand.hpp"
#include "renderer/vk_types.hpp"

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <variant>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace clz::renderer
{
	namespace
	{
		constexpr uint32_t SHAPE_COUNT = 4;

		/// Order MUST match enum class SHAPE.
		const std::array<std::filesystem::path, SHAPE_COUNT> SHAPE_PATHS = {
			"assets/shapes/cube.glb",
			"assets/shapes/sphere.glb",
			"assets/shapes/cylinder.gltf",
			"assets/shapes/cuboid.gltf"
		};

		/// Everything loaded from one glb, CPU side.
		struct ShapeCPUData
		{
			std::vector<ShapeMesh> vertices;
			std::vector<uint32_t> indices;
			TextureCPUData texture;
			bool hasTexture = false;
			math::vec4 baseFactor = math::vec4(1.0f, 1.0f, 1.0f, 1.0f);

			VkDeviceSize vertexOffset = 0;
			VkDeviceSize indexOffset = 0;
			bool valid = false;
		};

		/// Everything needed to draw one shape.
		struct ShapeGPUData
		{
			VertexRange vertexRange{};
			IndexRange indexRange{};
			/// Indices pushed to the shader. UINT32_MAX means "no texture".
			uint32_t textureIndex = UINT32_MAX;
			uint32_t samplerIndex = UINT32_MAX;
			math::vec4 baseFactor = math::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			bool valid = false;
		};

		std::array<ShapeGPUData, SHAPE_COUNT> s_shapes;
		bool s_initialized = false;

		constexpr VkDeviceSize alignUp(const VkDeviceSize v, const VkDeviceSize a)
		{
			return (v + a - 1) / a * a;
		}

		/// Registers the base colour texture of the first material that has one.
		bool loadBaseTexture(
			const fastgltf::Asset& asset,
			const std::filesystem::path& modelPath,
			ShapeCPUData& out
		)
		{
			for (const auto& material : asset.materials)
			{
				if (!material.pbrData.baseColorTexture.has_value())
					continue;

				const auto& fastTexture =
					asset.textures[material.pbrData.baseColorTexture->textureIndex];
				if (!fastTexture.imageIndex.has_value())
					continue;

				const auto& image = asset.images[fastTexture.imageIndex.value()];

				const auto result = std::visit(
					[&](const auto& source) -> std::expected<TextureCPUData, std::string>
					{
						using SourceT = std::decay_t<decltype(source)>;

						if constexpr (std::is_same_v<SourceT, fastgltf::sources::URI>)
						{
							auto r = TextureManager.registerTexture(
								modelPath.parent_path() / source.uri.string()
							);
							if (!r)
								return std::unexpected(std::string("uri texture failed"));
							return r.value();
						}
						else if constexpr (std::is_same_v<SourceT, fastgltf::sources::BufferView>)
						{
							const auto& view = asset.bufferViews[source.bufferViewIndex];
							const auto& buffer = asset.buffers[view.bufferIndex];

							const std::byte* bytes = nullptr;
							if (const auto* a = std::get_if<fastgltf::sources::Array>(&buffer.data))
								bytes = a->bytes.data() + view.byteOffset;
							else if (const auto* v = std::get_if<fastgltf::sources::Vector>(&buffer.data))
								bytes = v->bytes.data() + view.byteOffset;
							else if (const auto* b = std::get_if<fastgltf::sources::ByteView>(&buffer.data))
								bytes = b->bytes.data() + view.byteOffset;

							if (bytes == nullptr)
								return std::unexpected(std::string("unsupported buffer source"));

							auto r = TextureManager.registerTexture(
								bytes,
								view.byteLength,
								modelPath.string() + "texture"
							);
							if (!r)
								return std::unexpected(std::string("embedded texture failed"));
							return r.value();
						}
						else
						{
							return std::unexpected(std::string("unsupported image source"));
						}
					},
					image.data
				);

				if (!result)
				{
					clz::log::error(
						"Shape " + modelPath.string() + ": " + result.error()
					);
					return false;
				}

				out.texture = result.value();
				out.texture.imageFormat = VK_FORMAT_R8G8B8A8_SRGB; // base colour
				out.hasTexture = true;
				return true;
			}
			return true; // no texture is fine, push constant colour is used
		}

		/// Parses one glb entirely into CPU data (buffer stays alive in scope).
		bool loadShape(
			fastgltf::Parser& parser,
			const std::filesystem::path& path,
			ShapeCPUData& out
		)
		{
			auto data = fastgltf::GltfDataBuffer::FromPath(path);
			if (!data)
			{
				clz::log::error(
					"Could not open shape: " + path.string() + " (" +
					std::string(fastgltf::getErrorMessage(data.error())) + ")"
				);
				return false;
			}

			auto assetResult = parser.loadGltf(
				data.get(),
				path.parent_path(),
				fastgltf::Options::LoadExternalBuffers
			);
			if (!assetResult)
			{
				clz::log::error(
					"Could not parse shape: " + path.string() + " (" +
					std::string(fastgltf::getErrorMessage(assetResult.error())) + ")"
				);
				return false;
			}
			const fastgltf::Asset& asset = assetResult.get();

			// Merge every primitive of every mesh into one vertex/index list.
			for (const auto& mesh : asset.meshes)
			{
				for (const auto& primitive : mesh.primitives)
				{
					const auto posIt = primitive.findAttribute("POSITION");
					if (posIt == primitive.attributes.end() ||
					    !primitive.indicesAccessor.has_value())
					{
						clz::log::error(
							"Shape " + path.string() +
							" has a primitive without POSITION or indices"
						);
						return false;
					}

					const uint32_t baseVertex = static_cast<uint32_t>(out.vertices.size());
					const auto& posAccessor = asset.accessors[posIt->accessorIndex];
					out.vertices.resize(baseVertex + posAccessor.count);

					fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(
						asset, posAccessor,
						[&](const fastgltf::math::fvec3& p, const std::size_t i)
						{
							out.vertices[baseVertex + i].position = {p.x(), p.y(), p.z()};
							out.vertices[baseVertex + i].uv = {0.0f, 0.0f};
						}
					);

					const auto uvIt = primitive.findAttribute("TEXCOORD_0");
					if (uvIt != primitive.attributes.end())
					{
						fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>(
							asset, asset.accessors[uvIt->accessorIndex],
							[&](const fastgltf::math::fvec2& uv, const std::size_t i)
							{
								out.vertices[baseVertex + i].uv = {uv.x(), uv.y()};
							}
						);
					}

					const auto& indexAccessor =
						asset.accessors[primitive.indicesAccessor.value()];
					const size_t baseIndex = out.indices.size();
					out.indices.resize(baseIndex + indexAccessor.count);
					fastgltf::iterateAccessorWithIndex<uint32_t>(
						asset, indexAccessor,
						[&](const uint32_t idx, const std::size_t i)
						{
							// indices are local to the primitive, rebase them
							out.indices[baseIndex + i] = idx + baseVertex;
						}
					);
				}
			}

			if (out.vertices.empty() || out.indices.empty())
			{
				clz::log::error("Shape " + path.string() + " has no geometry");
				return false;
			}

			// Base colour factor: material of the first primitive that has one,
			// otherwise the first material in the file, otherwise white.
			{
				const fastgltf::Material* baseMaterial = nullptr;
				for (const auto& mesh : asset.meshes)
				{
					for (const auto& primitive : mesh.primitives)
					{
						if (primitive.materialIndex.has_value())
						{
							baseMaterial = &asset.materials[primitive.materialIndex.value()];
							break;
						}
					}
					if (baseMaterial != nullptr)
						break;
				}
				if (baseMaterial == nullptr && !asset.materials.empty())
					baseMaterial = &asset.materials.front();

				if (baseMaterial != nullptr)
				{
					const auto& f = baseMaterial->pbrData.baseColorFactor;
					out.baseFactor = math::vec4(f.x(), f.y(), f.z(), f.w());
				}
			}

			if (!loadBaseTexture(asset, path, out))
				return false;

			out.valid = true;
			return true;
		}
	} // namespace

	bool initShapes()
	{
		if (s_initialized)
			return true;

		fastgltf::Parser parser;

		std::array<ShapeCPUData, SHAPE_COUNT> cpuShapes;
		for (uint32_t i = 0; i < SHAPE_COUNT; ++i)
		{
			if (!loadShape(parser, SHAPE_PATHS[i], cpuShapes[i]))
			{
				clz::log::error("Failed to load shape: " + SHAPE_PATHS[i].string(), true);
				return false;
			}
		}

		// ---- Size the dedicated vertex/index buffers exactly + staging layout ----
		VkDeviceSize totalVertices = 0;
		VkDeviceSize totalIndices = 0;
		VkDeviceSize stagingSize = 0;
		for (auto& s : cpuShapes)
		{
			s.vertexOffset = stagingSize;
			stagingSize = alignUp(stagingSize + s.vertices.size() * sizeof(ShapeMesh), 16);
			s.indexOffset = stagingSize;
			stagingSize = alignUp(stagingSize + s.indices.size() * sizeof(uint32_t), 16);

			if (s.hasTexture)
			{
				s.texture.memoryOffset = stagingSize;
				stagingSize = alignUp(stagingSize + s.texture.textureSize, 16);
			}

			totalVertices += s.vertices.size();
			totalIndices += s.indices.size();
		}

		if (!ShapeBufferManager.initialize(totalVertices * sizeof(ShapeMesh)))
		{
			clz::log::error("Could not create shapes vertex buffer", true);
			return false;
		}
		if (!ShapeIndicesManager.initialize(totalIndices * sizeof(uint32_t)))
		{
			clz::log::error("Could not create shapes index buffer", true);
			ShapeBufferManager.destroy(true);
			return false;
		}

		// ---- Staging buffer ----
		const auto& device = r_deviceContext.device;
		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VkDeviceMemory stagingMemory = VK_NULL_HANDLE;

		const VkBufferCreateInfo bufferCI{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.size = stagingSize,
			.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = nullptr
		};

		auto cleanupFailure = [&]()
		{
			if (stagingBuffer != VK_NULL_HANDLE)
				vkDestroyBuffer(device, stagingBuffer, nullptr);
			if (stagingMemory != VK_NULL_HANDLE)
				vkFreeMemory(device, stagingMemory, nullptr);
			ShapeBufferManager.destroy(true);
			ShapeIndicesManager.destroy(true);
		};

		if (vkCreateBuffer(device, &bufferCI, nullptr, &stagingBuffer) != VK_SUCCESS)
		{
			clz::log::error("Vulkan failed to create shapes staging buffer", true);
			cleanupFailure();
			return false;
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, stagingBuffer, &memRequirements);
		const VkMemoryAllocateInfo allocInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = nullptr,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(
				memRequirements.memoryTypeBits,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			)
		};
		if (vkAllocateMemory(device, &allocInfo, nullptr, &stagingMemory) != VK_SUCCESS)
		{
			clz::log::error("Vulkan failed to allocate shapes staging memory", true);
			cleanupFailure();
			return false;
		}
		vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0);

		// ---- Record uploads ----
		VkCommandBuffer commandBuffer = startSingleTimeCommand();

		std::vector<VkBufferCopy> vertexCopies;
		std::vector<VkBufferCopy> indexCopies;
		vertexCopies.reserve(SHAPE_COUNT);
		indexCopies.reserve(SHAPE_COUNT);

		for (uint32_t i = 0; i < SHAPE_COUNT; ++i)
		{
			auto& cpu = cpuShapes[i];
			auto& gpu = s_shapes[i];

			auto [vRange, vCopy] = ShapeBufferManager.recordVerticesSubmission(
				cpu.vertices,
				commandBuffer,
				stagingBuffer,
				cpu.vertexOffset,
				stagingMemory
			);
			gpu.vertexRange = vRange;
			gpu.baseFactor = cpu.baseFactor;
			vertexCopies.push_back(vCopy);

			auto [iRange, iCopy] = ShapeIndicesManager.recordIndicesSubmission(
				cpu.indices,
				commandBuffer,
				stagingBuffer,
				cpu.indexOffset,
				stagingMemory
			);
			gpu.indexRange = iRange;
			indexCopies.push_back(iCopy);

			if (cpu.hasTexture)
			{
				TextureManager.recordSubmitTextureDataToGPUCommands(
					cpu.texture,
					commandBuffer,
					stagingBuffer,
					stagingMemory,
					cpu.texture.memoryOffset,
					cpu.texture.memoryOffset
				);
				const TextureId textureId = cpu.texture.stagingId;
				gpu.textureIndex = textureId.isNull() ? UINT32_MAX : textureId.getId();
				// glb sampler settings are ignored, shapes use the default sampler.
				gpu.samplerIndex = TextureManager.getDefaultSamplerId().getId();
			}

			gpu.valid = true;
		}

		ShapeBufferManager.flushCopies(vertexCopies, commandBuffer, stagingBuffer);
		ShapeIndicesManager.flushCopies(indexCopies, commandBuffer, stagingBuffer);

		submitSingleTimeCommand(commandBuffer);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingMemory, nullptr);

		TextureDescriptor.updateDescriptorSets();

		s_initialized = true;
		return true;
	}

	void closeShapes()
	{
		if (!s_initialized)
			return;

		// Buffers were allocated by initShapes, so release their memory too.
		ShapeBufferManager.destroy(true);
		ShapeIndicesManager.destroy(true);

		for (auto& s : s_shapes)
			s = {};

		s_initialized = false;
	}

	void drawShape(
		VkCommandBuffer commandBuffer,
		const SHAPE shape,
		const math::mat4& projection,
		const math::mat4& view,
		const math::mat4& model
	)
	{
		const auto shapeIndex = static_cast<uint32_t>(shape);

#ifdef CLZ_ENABLE_CHECKS /*************************************************/
		CLZ_ASSERT(shapeIndex < SHAPE_COUNT, "Invalid shape enum passed to drawShape");
#endif /*******************************************************************/

		const ShapeGPUData& data = s_shapes[shapeIndex];
		if (!data.valid)
			return;

		vkCmdBindPipeline(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			r_shapePipelineContext.pipeline
		);
		ShapeBufferManager.bindVertexBuffer(commandBuffer);
		ShapeIndicesManager.bindIndexBuffer(commandBuffer);
		TextureDescriptor.bindDescriptor(
			commandBuffer,
			r_shapePipelineContext
		);

		// Engine's mat4 multiplication is reversed (see ModelManager::drawNode),
		// so model * view * projection == P * V * M in column-major maths.
		ShapePushConstants pc{
			.mvp = model * view * projection,
			.baseFactor = data.baseFactor,
			.textureIndex = data.textureIndex,
			.samplerIndex = data.samplerIndex
		};

		vkCmdPushConstants(
			commandBuffer,
			r_shapePipelineContext.layout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(ShapePushConstants),
			&pc
		);

		vkCmdDrawIndexed(
			commandBuffer,
			data.indexRange.indexCount,
			1,
			data.indexRange.firstIndex,
			static_cast<int32_t>(data.vertexRange.baseIndex),
			0
		);
	}

} // namespace clz::renderer
