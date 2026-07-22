/**
 * @file model.cpp
 * @curl0z
 * @brief Implementation of the 3D model loading, management, and rendering systems.
 */

#include "renderer/model/model.hpp"
#include "core/assert.hpp"
#include "core/logs.hpp"
#include "fastgltf/tools.hpp"
#include "math/quat.hpp"
#include "math/vec2.hpp"
#include "math/worldtransform.hpp"
#include "renderer/entitydata/indexbuffer.hpp"
#include "renderer/entitydata/texture.hpp"
#include "renderer/entitydata/uvbuffer.hpp"
#include "renderer/entitydata/vertexbuffer.hpp"
#include "renderer/pipelinedata/pushconstants.hpp"
#include "renderer/vk_types.hpp"
#include "entity/componentmanager.hpp"
#include "entity/components.hpp"

namespace clz::renderer
{
	/**
	 * @copydoc loadModel
	 */
	std::expected<ModelId, std::string> loadModel(const std::filesystem::path& filePath)
	{
		const ModelId Id = ModelLUT.size();
		ModelLUT.resize(ModelLUT.size() + 1);
		TextureCaches.resize(TextureCaches.size() + 1);
		ModelPaths.resize(ModelPaths.size() + 1);
		ModelPaths[Id] = ModelPath(filePath);

		fastgltf::Parser parser{};
		auto data = fastgltf::GltfDataBuffer::FromPath(filePath);

		// Load glTF asset with external buffers
		fastgltf::Expected<fastgltf::Asset> assetResult =
		    parser.loadGltf(data.get(), filePath.parent_path(), fastgltf::Options::LoadExternalBuffers);

		if (!assetResult)
		{
			const auto error = getErrorMessage(assetResult.error());
			clz::log::error(error);
			return std::unexpected(std::string(error));
		}

		const fastgltf::Asset& asset = assetResult.get();
		Model ourModel;

		// Parse scene graph starting from root nodes
		for (const auto& scene : asset.scenes)
		{
			for (const auto& nodeIndex : scene.nodeIndices)
			{
				ourModel.nodes.emplace_back(ourModel.loadNode(asset.nodes[nodeIndex], asset, Id));
			}
		}

		ModelLUT[Id] = std::move(ourModel);
		TextureCaches[Id].clear();
		return Id;
	}

	/**
	 * @copydoc drawAllModels
	 */
	void drawAllModels(VkCommandBuffer& commandBuffer)
	{
		const auto& model_entities = clz::ecs::getEntitiesWithComponent<ecs::ModelComponent>();

		for (const auto model_entity : model_entities)
		{
			const auto& modelId = ecs::getComponent<ecs::ModelComponent>(model_entity).modelId;
			const auto& transform = ecs::getComponent<ecs::TransformComponent>(model_entity);

			drawModel(modelId, transform.position, transform.rotation, transform.scale, commandBuffer);
		}
	}

	/**
	 * @copydoc drawModel
	 */
	void drawModel(const ModelId modelId, const math::vec3& position, const math::quat& rotation, const math::vec3& scale,
		       VkCommandBuffer commandBuffer)
	{
		CLZ_ASSERT(modelId < ModelLUT.size() && modelId != NULL_MODEL, "Invalid model Id sent for drawing");

		ModelLUT[modelId].draw(math::getModelMatrix(rotation, position, scale), commandBuffer);
	}

	/**
	 * @copydoc getModelPath
	 */
	std::filesystem::path getModelPath(const ModelId modelId)
	{
		CLZ_ASSERT(modelId < ModelLUT.size() && modelId != NULL_MODEL, "Invalid model Id sent for retrieving path");
		return ModelPaths[modelId].path;
	}
} // namespace clz::renderer

namespace clz::renderer
{
	/**
	 * @copydoc Model::draw
	 */
	void Model::draw(const math::mat4& modelMatrix, VkCommandBuffer commandBuffer)
	{
		for (const auto& node : nodes)
		{
			drawNode(node, modelMatrix, commandBuffer);
		}
	}

	/**
	 * @copydoc Model::drawNode
	 */
	void Model::drawNode(const auto& node, const math::mat4& parentTransform, VkCommandBuffer commandBuffer)
	{
		const auto worldTransform = node.localTransform * parentTransform;

		if (!node.meshIndex.has_value())
			goto drawChildren;

		// Draw all primitives in the associated mesh
		for (const auto& primitive : meshes[node.meshIndex.value()].value().primitives)
		{
			constexpr std::array<VkDeviceSize, 2> offsets = {0, 0};
			const std::array buffers = {r_vertexBuffer, r_uvBuffer};

			vkCmdBindVertexBuffers(commandBuffer, 0, buffers.size(), buffers.data(), offsets.data());
			vkCmdBindIndexBuffer(commandBuffer, r_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			ModelDataPC pc = {.modelMatrix = worldTransform,
					  .textureID = primitive.baseTextureId.has_value() ? primitive.baseTextureId.value() : r_NULL_TEXTURE};

			vkCmdPushConstants(commandBuffer, r_pipelineContext.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ModelDataPC), &pc);

			vkCmdDrawIndexed(commandBuffer, primitive.indexCount, 1, primitive.firstIndex, primitive.baseVertexIndex, 0);
		}

	drawChildren:
		// Recursively draw child nodes
		for (const auto& child : node.children)
		{
			drawNode(child, worldTransform, commandBuffer);
		}
	}

	/**
	 * @copydoc Model::loadNode
	 */
	Node Model::loadNode(const fastgltf::Node& node, const fastgltf::Asset& asset, const ModelId registeredId)
	{
		Node ourNode;

		// Extract transformation matrix (either direct matrix or TRS components)
		if (const auto* mat = std::get_if<fastgltf::math::fmat4x4>(&node.transform))
		{
			// fastgltf's fmat4x4 is column-major
			memcpy(&ourNode.localTransform, mat, sizeof(float) * 16);
		}
		else if (const auto* trs = std::get_if<fastgltf::TRS>(&node.transform))
		{
			const math::vec3 translation(trs->translation[0], trs->translation[1], trs->translation[2]);
			// fastgltf stores x,y,z,w. Our math library expects w,x,y,z
			const math::quat rotation(trs->rotation[3], trs->rotation[0], trs->rotation[1], trs->rotation[2]);
			const math::vec3 scale(trs->scale[0], trs->scale[1], trs->scale[2]);

			ourNode.localTransform = math::getModelMatrix(rotation, translation, scale);
		}

		const auto& meshIndex = node.meshIndex;
		if (!meshIndex.has_value())
		{
			goto loadChildren;
		}

		if (meshes.size() <= meshIndex.value())
		{
			meshes.resize(meshIndex.value() + 1);
			goto loadMesh;
		}

		if (meshes[meshIndex.value()].has_value())
		{
			ourNode.meshIndex = meshIndex.value();
			goto loadChildren;
		}

	loadMesh:
		meshes[meshIndex.value()] = loadMesh(asset.meshes[meshIndex.value()], asset, registeredId);
		ourNode.meshIndex = meshIndex.value();

	loadChildren:
		for (const auto& childIndex : node.children)
		{
			ourNode.children.emplace_back(loadNode(asset.nodes[childIndex], asset, registeredId));
		}

		return ourNode;
	}

	/**
	 * @copydoc Model::loadMesh
	 */
	Mesh Model::loadMesh(const fastgltf::Mesh& mesh, const fastgltf::Asset& asset, const ModelId registeredId)
	{
		Mesh ourMesh;

		for (const auto& primitive : mesh.primitives)
		{
			PrimitiveData primitiveData;

			// --- Extract Positions ---
			const auto& positionIt = primitive.findAttribute("POSITION");
			if (positionIt == primitive.attributes.end())
				continue;

			const auto& positionAccessor = asset.accessors[positionIt->accessorIndex];
			std::vector<math::vec3> positions(positionAccessor.count);

			fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(
			    asset, positionAccessor,
			    [&](const fastgltf::math::fvec3& pos, const std::size_t posIndex) { positions[posIndex] = {pos.x(), pos.y(), pos.z()}; });
			primitiveData.vertices = std::vector(positions.begin(), positions.end());

			// --- Extract UVs ---
			const auto& uvIt = primitive.findAttribute("TEXCOORD_0");
			if (uvIt != primitive.attributes.end())
			{
				const auto& uvAccessor = asset.accessors[uvIt->accessorIndex];
				std::vector<math::vec2> uvs(uvAccessor.count);
				CLZ_ASSERT(uvs.size() == positions.size(), "positions and uv's size are not equal");

				fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>(
				    asset, uvAccessor,
				    [&](const fastgltf::math::fvec2& pos, const std::size_t uvIndex) { uvs[uvIndex] = {pos.x(), pos.y()}; });
				primitiveData.uvs = std::vector(uvs.begin(), uvs.end());
			}
			else
			{
				primitiveData.uvs = std::vector(positions.size(), math::vec2(0.0f, 0.0f));
				clz::log::warn("model has no textures");
			}

			// --- Extract Indices ---
			const auto& indexAccessor = asset.accessors[primitive.indicesAccessor.value()];
			std::vector<uint32_t> indices(indexAccessor.count);

			fastgltf::iterateAccessorWithIndex<uint32_t>(
			    asset, indexAccessor, [&](const uint32_t drawIndex, const std::size_t it) { indices[it] = drawIndex; });
			primitiveData.indices = std::vector(indices.begin(), indices.end());

			// --- Extract Material / Texture ---
			if (!primitive.materialIndex)
				continue;

			const auto& material = asset.materials[primitive.materialIndex.value()];
			if (!material.pbrData.baseColorTexture.has_value())
				continue;

			const auto BaseColorTextureIndex = material.pbrData.baseColorTexture.value().textureIndex;
			primitiveData.baseColorTextureIndex = BaseColorTextureIndex;

			const auto& texture = asset.textures[BaseColorTextureIndex];
			const auto& image = asset.images[texture.imageIndex.value()];

			if (std::holds_alternative<fastgltf::sources::URI>(image.data))
			{
				primitiveData.baseTexture = std::get<fastgltf::sources::URI>(image.data);
			}
			else if (std::holds_alternative<fastgltf::sources::BufferView>(image.data))
			{
				primitiveData.baseTexture = std::get<fastgltf::sources::BufferView>(image.data);
			}

			// Submit extracted data to GPU buffers and register textures
			ourMesh.primitives.emplace_back(registerPrimitive(primitiveData, asset, registeredId));
		}

		return ourMesh;
	}

	/**
	 * @copydoc Model::registerPrimitive
	 */
	Primitive Model::registerPrimitive(const PrimitiveData& primitiveData, const fastgltf::Asset& asset, const ModelId registeredId)
	{
		Primitive ourPrimitive;

		// Upload geometry data to global GPU buffers
		ourPrimitive.baseVertexIndex = getVertexBaseIndex();
		registerVertices(primitiveData.vertices);
		registerUVs(primitiveData.uvs);

		auto [first, count] = registerIndices(primitiveData.indices);
		ourPrimitive.firstIndex = first;
		ourPrimitive.indexCount = count;

		// Helper lambda to check if a texture is already loaded in the cache
		auto getTextureIdIfExistsInCache = [&](std::unordered_map<size_t, TextureID>& textureCache, const size_t textureIndex) {
			if (const auto& it = textureCache.find(textureIndex); it != textureCache.end())
			{
				return std::make_tuple(true, it->second);
			}
			return std::make_tuple(false, r_NULL_TEXTURE);
		};

		// Handle Texture Loading (URI or BufferView)
		if (std::holds_alternative<fastgltf::sources::URI>(primitiveData.baseTexture.value()))
		{
			const std::filesystem::path texturePath = getModelPath(registeredId).parent_path() /
								  std::get<fastgltf::sources::URI>(primitiveData.baseTexture.value()).uri.c_str();

			auto [result, Id] =
			    getTextureIdIfExistsInCache(TextureCaches[registeredId].texturesLoaded, primitiveData.baseColorTextureIndex.value());

			if (result)
			{
				ourPrimitive.baseTextureId = Id;
			}
			else
			{
				auto textureId = registerTexture(texturePath);
				ourPrimitive.baseTextureId = textureId;
				TextureCaches[registeredId].texturesLoaded[primitiveData.baseColorTextureIndex.value()] = textureId;
			}
		}
		else if (std::holds_alternative<fastgltf::sources::BufferView>(primitiveData.baseTexture.value()))
		{
			const auto& bufferViewSource = std::get<fastgltf::sources::BufferView>(primitiveData.baseTexture.value());
			const auto& bufferView = asset.bufferViews[bufferViewSource.bufferViewIndex];
			const auto& buffer = asset.buffers[bufferView.bufferIndex];

			const std::byte* imageBytes = nullptr;
			const size_t imageDataOffset = bufferView.byteOffset;
			const size_t imageDataLength = bufferView.byteLength;

			if (const auto* arraySource = std::get_if<fastgltf::sources::Array>(&buffer.data))
			{
				imageBytes = arraySource->bytes.data() + imageDataOffset;
			}
			if (const auto* arraySource = std::get_if<fastgltf::sources::Vector>(&buffer.data))
			{
				imageBytes = arraySource->bytes.data() + imageDataOffset;
			}

			auto [result, Id] =
			    getTextureIdIfExistsInCache(TextureCaches[registeredId].texturesLoaded, primitiveData.baseColorTextureIndex.value());

			if (result)
			{
				ourPrimitive.baseTextureId = Id;
			}
			else
			{
				auto textureId =
				    registerTexture(imageBytes, imageDataLength, "base texture for " + getModelPath(registeredId).string());
				ourPrimitive.baseTextureId = textureId;
				TextureCaches[registeredId].texturesLoaded[primitiveData.baseColorTextureIndex.value()] = textureId;
			}
		}

		return ourPrimitive;
	}
} // namespace clz::renderer