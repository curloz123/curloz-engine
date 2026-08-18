/**
 * @file model.cpp
 * @curl0z
 * @brief Implementation of the 3D model loading, management, and rendering systems.
 */

#include "renderer/model/model.hpp"
#include "core/assert.hpp"
#include "core/logs.hpp"
#include "entity/componentmanager.hpp"
#include "entity/corecomponents.hpp"
#include "fastgltf/tools.hpp"
#include "math/quat.hpp"
#include "math/vec2.hpp"
#include "math/worldtransform.hpp"
#include "renderer/entitydata/indexbuffer.hpp"
#include "renderer/entitydata/texture.hpp"
#include "renderer/entitydata/vertexbuffer.hpp"
#include "renderer/pipelinedata/pushconstants.hpp"
#include "renderer/rendercomponent.hpp"
#include "renderer/vk_types.hpp"
#include "core/time.hpp"


namespace clz::renderer
{
	/// @copydoc loadModel
	std::expected<ModelId, std::string> loadModel(const std::filesystem::path& filePath)
	{
		const ModelId Id = ModelLUT.size();
		ModelLUT.resize(ModelLUT.size() + 1);
		TextureCaches.resize(TextureCaches.size() + 1);
		ModelPaths.resize(ModelPaths.size() + 1);
		ModelPaths[Id] = ModelPath(filePath);

		fastgltf::Parser parser{};
		auto data = fastgltf::GltfDataBuffer::FromPath(filePath);
		if (!data)
		{
			const auto error = getErrorMessage(data.error());
			clz::log::error(error);
			return std::unexpected(std::string(error));
		}

		// Load glTF asset with external buffers
		fastgltf::Expected<fastgltf::Asset> assetResult = parser.loadGltf(
			data.get(),
			filePath.parent_path(),
			fastgltf::Options::LoadExternalBuffers
		);

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
				ourModel.nodes.emplace_back(
					ourModel.loadNode(asset.nodes[nodeIndex], asset, Id)
				);
			}
		}

		ModelLUT[Id] = std::move(ourModel);
		TextureCaches[Id].clear();
		return Id;
	}

	/**
	 * @copydoc drawAllModels
	 */
	void drawAllModels(VkCommandBuffer commandBuffer)
	{
		const auto& model_entities = clz::ecs::getEntitiesWithComponent<ModelComponent>();

		for (const auto model_entity : model_entities)
		{
			const auto& modelId =
				ecs::getComponent<ModelComponent>(model_entity).modelId;
			const auto& transform =
				ecs::getComponent<ecs::TransformComponent>(model_entity);

			drawModel(
				modelId,
				transform.position,
				transform.rotation,
				transform.scale,
				commandBuffer
			);
		}
	}

	/**
	 * @copydoc drawModel
	 */
	void drawModel(
		const ModelId modelId,
		const math::vec3& position,
		const math::quat& rotation,
		const math::vec3& scale,
		VkCommandBuffer commandBuffer
	)
	{
		CLZ_ASSERT(
			modelId < ModelLUT.size() && modelId != NULL_MODEL,
			"Invalid model Id sent for drawing"
		);

		ModelLUT[modelId].draw(
			math::getModelMatrix(rotation, position, scale),
			commandBuffer
		);
	}

	/**
	 * @copydoc getModelPath
	 */
	std::filesystem::path getModelPath(const ModelId modelId)
	{
		CLZ_ASSERT(
			modelId < ModelLUT.size() && modelId != NULL_MODEL,
			"Invalid model Id sent for retrieving path"
		);
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
	void Model::drawNode(
		const auto& node,
		const math::mat4& parentTransform,
		VkCommandBuffer commandBuffer
	)
	{
		const auto worldTransform = node.localTransform * parentTransform;

		if (!node.meshIndex.has_value())
			goto drawChildren;

		// Draw all primitives in the associated mesh
		for (const auto& primitive : meshes[node.meshIndex.value()].value().primitives)
		{
			constexpr std::array<VkDeviceSize, 1> offsets = {0};
			const std::array<VkBuffer, 1> buffers = {r_vertexBuffer};

			vkCmdBindVertexBuffers(
				commandBuffer,
				0,
				buffers.size(),
				buffers.data(),
				offsets.data()
			);
			vkCmdBindIndexBuffer(commandBuffer, r_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			ModelDataPC pc = {
				.modelMatrix = worldTransform,
				.baseColorFactor = primitive.baseColorFactor,
				.baseTextureIndex = primitive.baseTextureId.value,
				.metallicFactor = primitive.metallicFactor,
				.roughnessFactor = primitive.roughnessFactor,
				.metallic_roughnessTextureIndex =
					primitive.metallic_roughnessTextureId.value,
				.normalTextureIndex = primitive.normalTextureId.value,
			};

			vkCmdPushConstants(
				commandBuffer,
				r_pipelineContext.layout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(ModelDataPC),
				&pc
			);

			vkCmdDrawIndexed(
				commandBuffer,
				primitive.indexCount,
				1,
				primitive.firstIndex,
				primitive.baseVertexIndex,
				0
			);
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
	Node Model::loadNode(
		const fastgltf::Node& node,
		const fastgltf::Asset& asset,
		const ModelId registeredId
	)
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
			const math::vec3 translation(
				trs->translation[0],
				trs->translation[1],
				trs->translation[2]
			);
			// fastgltf stores x,y,z,w. Our math library expects w,x,y,z
			const math::quat rotation(
				trs->rotation[3],
				trs->rotation[0],
				trs->rotation[1],
				trs->rotation[2]
			);
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
		meshes[meshIndex.value()] =
			loadMesh(asset.meshes[meshIndex.value()], asset, registeredId);
		ourNode.meshIndex = meshIndex.value();

	loadChildren:
		for (const auto& childIndex : node.children)
		{
			ourNode.children.emplace_back(
				loadNode(asset.nodes[childIndex], asset, registeredId)
			);
		}

		return ourNode;
	}

	/// @copydoc Model::loadMesh
	Mesh Model::loadMesh(
		const fastgltf::Mesh& mesh,
		const fastgltf::Asset& asset,
		const ModelId registeredId
	)
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
			primitiveData.attributes.resize(positionAccessor.count);

			fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(
				asset,
				positionAccessor,
				[&](const fastgltf::math::fvec3& pos, const std::size_t posIndex) {
					primitiveData.attributes[posIndex]
						.position = {pos.x(), pos.y(), pos.z()};
				}
			);

			// --- Extract UVs ---
			const auto& uvIt = primitive.findAttribute("TEXCOORD_0");
			if (uvIt != primitive.attributes.end())
			{
				const auto& uvAccessor = asset.accessors[uvIt->accessorIndex];
				if (uvAccessor.count != primitiveData.attributes.size())
				{
					clz::log::error(
						"positions and uv's size are not equal"
					);
				}

				fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>(
					asset,
					uvAccessor,
					[&](const fastgltf::math::fvec2& uv,
					    const std::size_t uvIndex) {
						primitiveData.attributes[uvIndex].uv = {uv.x(), uv.y()};
					}
				);
			}
			else
			{
				for (size_t i = 0; i < primitiveData.attributes.size(); ++i)
				{
					primitiveData.attributes[i].uv = {0.0f, 0.0f};
				}
				clz::log::warn("mesh has no textures");
			}
			// --- Extract Normals ---
			const auto& normalIt = primitive.findAttribute("NORMAL");
			if (normalIt != primitive.attributes.end())
			{
				const auto& normalAccessor =
					asset.accessors[normalIt->accessorIndex];
				if (normalAccessor.count != primitiveData.attributes.size())
				{
					clz::log::error(
						"positions and normal size are not equal");
				}

				fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(
					asset,
					normalAccessor,
					[&](const fastgltf::math::fvec3 normal,
					    const std::size_t index) {
						primitiveData.attributes[index].normal = {
							normal.x(),
							normal.y(),
							normal.z()
						};
					}
				);
			}
			// --- Calculate tangent and bitangent
			const auto &tangentIt = primitive.findAttribute("TANGENT");
			if (tangentIt != primitive.attributes.end())
			{
				const auto& tangentAccessor =
					asset.accessors[tangentIt->accessorIndex];
				if (tangentAccessor.count != primitiveData.attributes.size())
				{
					clz::log::error(
						"positions and tangent size are not equal");
				}
				fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec4>(
					asset,
					tangentAccessor,
					[&](const fastgltf::math::fvec4& tangent,
						const std::size_t index) {
						primitiveData.attributes[index].tangent = {
							tangent.x(),
							tangent.y(),
							tangent.z(),
							tangent.w()
						};
					}
				);
			}
			else
			{
				for (size_t i = 0; i < primitiveData.attributes.size(); ++i)
				{
					primitiveData.attributes[i].tangent = {
						1.0f, 1.0f, 1.0f, 1.0f
					};
				}
			}

			// --- Extract Indices ---
			const auto& indexAccessor =
				asset.accessors[primitive.indicesAccessor.value()];
			primitiveData.indices.resize(indexAccessor.count);
			fastgltf::iterateAccessorWithIndex<uint32_t>(
				asset,
				indexAccessor,
				[&](const uint32_t drawIndex, const std::size_t it) {
					primitiveData.indices[it] = drawIndex;
				}
			);

			// --- Extract Material / Texture ---
			if (primitive.materialIndex)
			{
				const auto& material =
					asset.materials[primitive.materialIndex.value()];

				/// --- Load base texture ---
				if (material.pbrData.baseColorTexture.has_value())
				{
					const auto BaseColorTextureIndex =
						material.pbrData.baseColorTexture.value()
							.textureIndex;
					primitiveData.baseColorTextureIndex = BaseColorTextureIndex;

					const auto& texture = asset.textures[BaseColorTextureIndex];
					const auto& image =
						asset.images[texture.imageIndex.value()];

					if (std::holds_alternative<fastgltf::sources::URI>(
						    image.data
					    ))
					{
						primitiveData.baseTexture =
							std::get<fastgltf::sources::URI>(
								image.data
							);
					}
					else if (std::holds_alternative<
							 fastgltf::sources::BufferView>(image.data))
					{
						primitiveData.baseTexture =
							std::get<fastgltf::sources::BufferView>(
								image.data
							);
					}

					primitiveData.baseColorFactor = math::vec4(
						material.pbrData.baseColorFactor.x(),
						material.pbrData.baseColorFactor.y(),
						material.pbrData.baseColorFactor.z(),
						1.0f
					);
				}
				/// --- load metallic-roughness texture
				if (material.pbrData.metallicRoughnessTexture.has_value())
				{
					const auto MetallicRoughnessTextureIndex =
						material.pbrData.metallicRoughnessTexture.value()
							.textureIndex;
					primitiveData.metallic_roughnessColorTextureIndex =
						MetallicRoughnessTextureIndex;

					const auto& fastTexture =
						asset.textures[MetallicRoughnessTextureIndex];
					const auto& fastImage =
						asset.images[fastTexture.imageIndex.value()];

					if (std::holds_alternative<fastgltf::sources::URI>(
						    fastImage.data
					    ))
					{
						primitiveData.metallic_roughnessTexture =
							std::get<fastgltf::sources::URI>(
								fastImage.data
							);
					}
					else if (std::holds_alternative<
							 fastgltf::sources::BufferView>(
							 fastImage.data
						 ))
					{
						primitiveData.metallic_roughnessTexture =
							std::get<fastgltf::sources::BufferView>(
								fastImage.data
							);
					}

					primitiveData.metallicFactor =
						material.pbrData.metallicFactor;
					primitiveData.roughnessFactor =
						material.pbrData.roughnessFactor;
				}
				/// --- load normal texture
				if (material.normalTexture.has_value())
				{
					const auto NormalTextureIndex =
						material.normalTexture.value().textureIndex;
					primitiveData.normalColorTextureIndex = NormalTextureIndex;

					const auto& fastTexture =
						asset.textures[NormalTextureIndex];
					const auto& fastImage =
						asset.images[fastTexture.imageIndex.value()];

					if (std::holds_alternative<fastgltf::sources::URI>(
						    fastImage.data
					    ))
					{
						primitiveData.normalTexture =
							std::get<fastgltf::sources::URI>(
								fastImage.data
							);
					}
					else if (std::holds_alternative<
							 fastgltf::sources::BufferView>(
							 fastImage.data
						 ))
					{
						primitiveData.normalTexture =
							std::get<fastgltf::sources::BufferView>(
								fastImage.data
							);
					}
				}
			}

			// Submit extracted data to GPU buffers and register textures
			ourMesh.primitives.emplace_back(
				registerPrimitive(primitiveData, asset, registeredId)
			);
		}

		return ourMesh;
	}

	/**
	 * @copydoc Model::registerPrimitive
	 */
	Primitive Model::registerPrimitive(
		const PrimitiveData& primitiveData,
		const fastgltf::Asset& asset,
		const ModelId registeredId
	)
	{
		Primitive ourPrimitive;

		// Upload geometry data to global GPU buffers
		ourPrimitive.baseVertexIndex = getVertexBaseIndex();
		registerVertexAttributes(primitiveData.attributes);

		auto [first, count] = registerIndices(primitiveData.indices);
		ourPrimitive.firstIndex = first;
		ourPrimitive.indexCount = count;

		// Helper lambda to GLTF's path'ed texture
		auto loadGLTFTexture = [](std::unordered_map<size_t, TextureID>& textureCache,
					  const size_t fastTextureIndex,
					  const std::filesystem::path& texturePath,
					  VkFormat textureFormat) {
			if (const auto& it = textureCache.find(fastTextureIndex);
			    it != textureCache.end())
			{
				return it->second;
			}

			auto textureId = registerTexture(texturePath, textureFormat);
			textureCache[fastTextureIndex] = textureId;

			return textureId;
		};
		// Helper lambda to GLB's binary texture
		auto loadGLBTexture = [](std::unordered_map<size_t, TextureID>& textureCache,
					 const size_t fastTextureIndex,
					 const std::byte* imageByte,
					 const size_t imageByteSize,
					 const std::string_view textureName,
					 const VkFormat textureFormat) {
			if (const auto& it = textureCache.find(fastTextureIndex);
			    it != textureCache.end())
			{
				return it->second;
			}
			auto textureId = registerTexture(imageByte, imageByteSize, textureName, textureFormat);
			textureCache[fastTextureIndex] = textureId;

			return textureId;
		};

		// Handle Texture Loading (URI or BufferView)

		std::optional<std::variant<fastgltf::sources::URI, fastgltf::sources::BufferView>> lookupKey;
		if (primitiveData.baseTexture.has_value())
		{
			lookupKey = primitiveData.baseTexture.value();
		}
		else if (primitiveData.metallic_roughnessTexture.has_value())
		{
			lookupKey = primitiveData.metallic_roughnessTexture.value();
		}
		else if (primitiveData.normalTexture.has_value())
		{
			lookupKey = primitiveData.normalTexture.value();
		}
		else
		{
			return ourPrimitive;
		}

		if (std::holds_alternative<fastgltf::sources::URI>(
			lookupKey.value()
		    ))
		{
			ourPrimitive.baseTextureId = r_NULL_TEXTURE;
			ourPrimitive.baseColorFactor = primitiveData.baseColorFactor;
			if (primitiveData.baseTexture.has_value())
			{
				const std::filesystem::path baseTexturePath =
					getModelPath(registeredId).parent_path() /
					std::get<fastgltf::sources::URI>(
						primitiveData.baseTexture.value()
					)
						.uri.c_str();

				ourPrimitive.baseTextureId = loadGLTFTexture(
					TextureCaches[registeredId].texturesLoaded,
					primitiveData.baseColorTextureIndex,
					baseTexturePath,
					VK_FORMAT_R8G8B8A8_SRGB
				);

			}

			ourPrimitive.metallic_roughnessTextureId = r_NULL_TEXTURE;
			ourPrimitive.roughnessFactor = primitiveData.roughnessFactor;
			ourPrimitive.metallicFactor = primitiveData.metallicFactor;
			if (primitiveData.metallic_roughnessTexture.has_value())
			{
				const std::filesystem::path mrTexturePath =
					getModelPath(registeredId).parent_path() /
					std::get<fastgltf::sources::URI>(
						primitiveData.metallic_roughnessTexture.value()
					)
						.uri.c_str();

				ourPrimitive.metallic_roughnessTextureId = loadGLTFTexture(
					TextureCaches[registeredId].texturesLoaded,
					primitiveData.metallic_roughnessColorTextureIndex,
					mrTexturePath,
					VK_FORMAT_R8G8B8A8_UNORM
				);


			}

			ourPrimitive.normalTextureId = r_NULL_TEXTURE;
			if (primitiveData.normalTexture.has_value())
			{
				const std::filesystem::path normalTexturePath =
					getModelPath(registeredId).parent_path() /
					std::get<fastgltf::sources::URI>(
						primitiveData.normalTexture.value()
					)
						.uri.c_str();

				ourPrimitive.normalTextureId = loadGLTFTexture(
					TextureCaches[registeredId].texturesLoaded,
					primitiveData.normalColorTextureIndex,
					normalTexturePath,
					VK_FORMAT_R8G8B8A8_UNORM
				);
			}
		}
		else if (std::holds_alternative<fastgltf::sources::BufferView>(
			lookupKey.value()
			))
		{
			ourPrimitive.baseTextureId = r_NULL_TEXTURE;
			ourPrimitive.baseColorFactor = primitiveData.baseColorFactor;
			if (primitiveData.baseTexture.has_value())
			{
				const auto& bufferViewSource =
					std::get<fastgltf::sources::BufferView>(
						primitiveData.baseTexture.value()
					);
				const auto& bufferView =
					asset.bufferViews[bufferViewSource.bufferViewIndex];
				const auto& buffer = asset.buffers[bufferView.bufferIndex];

				const std::byte* imageBytes = nullptr;
				const size_t imageDataOffset = bufferView.byteOffset;
				const size_t imageDataLength = bufferView.byteLength;

				if (const auto* arraySource =
					    std::get_if<fastgltf::sources::Array>(&buffer.data))
				{
					imageBytes = arraySource->bytes.data() + imageDataOffset;
				}
				if (const auto* arraySource =
					    std::get_if<fastgltf::sources::Vector>(&buffer.data))
				{
					imageBytes = arraySource->bytes.data() + imageDataOffset;
				}

				ourPrimitive.baseTextureId = loadGLBTexture(
					TextureCaches[registeredId].texturesLoaded,
					primitiveData.baseColorTextureIndex,
					imageBytes,
					imageDataLength,
					"base texture for " + getModelPath(registeredId).string(),
					VK_FORMAT_R8G8B8A8_SRGB
				);
			}

			ourPrimitive.metallic_roughnessTextureId = r_NULL_TEXTURE;
			ourPrimitive.metallicFactor = primitiveData.metallicFactor;
			ourPrimitive.roughnessFactor = primitiveData.roughnessFactor;
			if (primitiveData.metallic_roughnessTexture.has_value())
			{
				const auto& bufferViewSource =
					std::get<fastgltf::sources::BufferView>(
						primitiveData.metallic_roughnessTexture.value()
					);
				const auto& bufferView =
					asset.bufferViews[bufferViewSource.bufferViewIndex];
				const auto& buffer = asset.buffers[bufferView.bufferIndex];

				const std::byte* imageBytes = nullptr;
				const size_t imageDataOffset = bufferView.byteOffset;
				const size_t imageDataLength = bufferView.byteLength;

				if (const auto* arraySource =
					    std::get_if<fastgltf::sources::Array>(&buffer.data))
				{
					imageBytes = arraySource->bytes.data() + imageDataOffset;
				}
				if (const auto* arraySource =
					    std::get_if<fastgltf::sources::Vector>(&buffer.data))
				{
					imageBytes = arraySource->bytes.data() + imageDataOffset;
				}

				ourPrimitive.metallic_roughnessTextureId = loadGLBTexture(
					TextureCaches[registeredId].texturesLoaded,
					primitiveData.metallic_roughnessColorTextureIndex,
					imageBytes,
					imageDataLength,
					"metallic-roughness texture for " +
						getModelPath(registeredId).string(),
					VK_FORMAT_R8G8B8A8_UNORM
				);

			}

			ourPrimitive.normalTextureId = r_NULL_TEXTURE;
			if (primitiveData.normalTexture.has_value())
			{
				const auto& bufferViewSource =
					std::get<fastgltf::sources::BufferView>(
						primitiveData.normalTexture.value()
					);
				const auto& bufferView =
					asset.bufferViews[bufferViewSource.bufferViewIndex];
				const auto& buffer = asset.buffers[bufferView.bufferIndex];

				const std::byte* imageBytes = nullptr;
				const size_t imageDataOffset = bufferView.byteOffset;
				const size_t imageDataLength = bufferView.byteLength;

				if (const auto* arraySource =
					    std::get_if<fastgltf::sources::Array>(&buffer.data))
				{
					imageBytes = arraySource->bytes.data() + imageDataOffset;
				}
				if (const auto* arraySource =
					    std::get_if<fastgltf::sources::Vector>(&buffer.data))
				{
					imageBytes = arraySource->bytes.data() + imageDataOffset;
				}

				ourPrimitive.normalTextureId = loadGLBTexture(
					TextureCaches[registeredId].texturesLoaded,
					primitiveData.normalColorTextureIndex,
					imageBytes,
					imageDataLength,
					"normal texture for " + getModelPath(registeredId).string(),
					VK_FORMAT_R8G8B8A8_UNORM
				);
			}
		}

		return ourPrimitive;
	}
} // namespace clz::renderer
