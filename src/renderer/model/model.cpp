/**
 * @file model.cpp
 * @curl0z
 * @brief Implementation of the 3D model loading, management, and rendering systems.
 */

#include "renderer/model/model.hpp"
#include "core/assert.hpp"
#include "core/logs.hpp"
#include "fastgltf/core.hpp"
#include "fastgltf/tools.hpp"
#include "fastgltf/types.hpp"
#include "math/quat.hpp"
#include "math/worldtransform.hpp"
#include "entity/componentmanager.hpp"
#include "entity/corecomponents.hpp"
#include "renderer/rendercomponent.hpp"
#include "renderer/entitydata/buffer.hpp"
#include "renderer/entitydata/texture.hpp"
#include "renderer/model/model_matrix.hpp"
#include "renderer/utility/memory.hpp"
#include "renderer/vk_types.hpp"
#include <cstdint>
#include "renderer/utility/singletimecommand.hpp"
#include "renderer/pipelinedata/pipeline_data.hpp"
#include "renderer/pipelinedata/pushconstants.hpp"

namespace clz::renderer
{
	bool ModelManagerClass::initialize()
	{
		m_parser = fastgltf::Parser{
			fastgltf::Extensions::KHR_materials_emissive_strength |
			fastgltf::Extensions::KHR_materials_ior
		};

		if (!m_materialManager.initialize())
		{
			clz::log::error("Could not initialize model matrix manager", true);
			return false;
		}
		if (!m_matrixManager.initialize())
		{
			clz::log::error("Could not initialize model material manager", true);
			return false;
		}

		return true;
	}

	void ModelManagerClass::close()
	{
		m_materialManager.destroy();
		m_matrixManager.destroy();
	}
	ModelId ModelManagerClass::loadBatch(const std::span<std::filesystem::path> modelPaths)
	{
		auto getModelId = [&](){
			ModelId modelId;
			if (!m_gaps.empty())
			{
				modelId = m_gaps.back();
				m_gaps.pop_back();
			}
			else
			{
				modelId = ModelId(m_newModelIndex);
				++m_newModelIndex;
			}
			return modelId;
		};

		const size_t numModels = modelPaths.size();

		/// resize all internal handles to Maximum size.
		if (numModels > m_gaps.size())
		{
			auto newSize = m_models.size() + numModels - m_gaps.size();
			m_models.resize(newSize);
			m_modelPaths.resize(newSize);
			m_modelEntities.resize(newSize);
		}

		std::vector<GLTFStagingData> gltfStagingDatas(modelPaths.size());

		for (size_t i = 0; i < modelPaths.size(); ++i)
		{
			const auto& modelPath = modelPaths[i];

			/// let model id leak.
			const auto modelId = getModelId();
			m_modelPaths[modelId.getId()] = modelPath;

			auto data = fastgltf::GltfDataBuffer::FromPath(modelPath);
			if (!data)
			{
				const auto error = getErrorMessage(data.error());
				clz::log::error(error);
				continue;
			}

			// Load glTF asset with external buffers
			fastgltf::Expected<fastgltf::Asset> assetResult = m_parser.loadGltf(
				data.get(),
				modelPath.parent_path(),
				fastgltf::Options::LoadExternalBuffers
			);
			if (!assetResult)
			{
				const auto error = getErrorMessage(assetResult.error());
				clz::log::error(error);
				continue;
			}

			gltfStagingDatas[i].modelId = modelId;
			gltfStagingDatas[i].fastAsset = std::move(assetResult.get());
			gltfStagingDatas[i].valid = true;
		}

		loadImages(gltfStagingDatas);
		loadSamplers(gltfStagingDatas);
		loadTextures(gltfStagingDatas);
		loadMaterials(gltfStagingDatas);
		loadMeshes(gltfStagingDatas);
		loadNodes(gltfStagingDatas);
		submitGPUData(gltfStagingDatas);

		TextureDescriptor.updateDescriptorSets();
		CameraDescriptor.updateMDescriptorSets();

		return gltfStagingDatas[0].modelId;
	}

	/// @copydoc drawAllModels
	void ModelManagerClass::drawAll(VkCommandBuffer commandBuffer)
	{
		StaticBufferManager.bindVertexBuffer(commandBuffer);
		StaticIndexManager.bindIndexBuffer(commandBuffer);

		const auto& model_entities = clz::ecs::getEntitiesWithComponent<ModelComponent>();

		for (const auto model_entity : model_entities)
		{
			const auto& modelId =
				ecs::getComponent<ModelComponent>(model_entity).modelId;
			const auto& transform =
				ecs::getComponent<ecs::TransformComponent>(model_entity);

			drawIndivisual(
				modelId,
				transform.position,
				transform.rotation,
				transform.scale,
				commandBuffer
			);
		}
	}

	void ModelManagerClass::drawIndivisual(
		const ModelId modelId,
		const math::vec3& position,
		const math::quat& rotation,
		const math::vec3& scale,
		VkCommandBuffer commandBuffer
	)
	{
#ifdef CLZ_ENABLE_CHECKS
		CLZ_ASSERT(
			modelId.getId() < m_models.size() && !modelId.isNull(),
			"Invalid model Id sent for drawing"
		);
#endif

		const auto& rootNodeIndices = m_models[modelId.getId()].rootNodesIndices;
		const math::mat4 worldTransform = math::getModelMatrix(rotation, position, scale);
		for (const uint32_t nodeIndex : rootNodeIndices)
		{
			drawNode(modelId, nodeIndex, worldTransform, commandBuffer);
		}
	}

	void ModelManagerClass::drawNode(
		const ModelId modelId,
		const uint32_t nodeIndex,
		const math::mat4& parentTransform,
		VkCommandBuffer commandBuffer
	)
	{
		const auto& nodeArray = m_models[modelId.getId()].nodes;
		const auto& meshArray = m_models[modelId.getId()].meshes;
		const auto& materialArray = m_models[modelId.getId()].materials;

		/// reversed to match column major format
		const Node& node = nodeArray[nodeIndex];
		const auto worldTransform = node.localTransform * parentTransform;
		m_matrixManager.setMatrix(node.matrixId, worldTransform);

		if (node.meshIndex == UINT32_MAX)
			goto drawChildren;

		// Draw all primitives in the associated mesh
		for (const Primitive& primitive : meshArray[node.meshIndex].primitives)
		{
			uint32_t materialIndex = (primitive.materialIdIndex == UINT32_MAX) ? 
							UINT32_MAX : 
							materialArray[primitive.materialIdIndex].getId();
			ModelDataPC pc{
				.matrixIndex = node.matrixId.getId(),
				.materialIndex = materialIndex 
				// .materialIndex = 0
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
				primitive.indexBuffer.indexCount,
				1,
				primitive.indexBuffer.firstIndex,
				primitive.vertexBuffer.baseIndex,
				0
			);
		}

	drawChildren:
		// Recursively draw child nodes
		for (const auto& childIndex : node.children)
		{
			drawNode(
				modelId,
				childIndex,
				worldTransform,
				commandBuffer
			);
		}
	}


	void ModelManagerClass::loadImages(
		std::vector<GLTFStagingData>& gltfStagingDatas
	)
	{
		for (size_t modelIndex = 0; modelIndex < gltfStagingDatas.size(); ++modelIndex)
		{
			auto& gltfStagingData = gltfStagingDatas[modelIndex];
			if (!gltfStagingData.valid)
				continue;

			const auto& modelPath = m_modelPaths[gltfStagingData.modelId.getId()];
			const auto& asset = gltfStagingData.fastAsset;
			auto& imageArray = gltfStagingData.images;
			imageArray.resize(asset.images.size());
			for (size_t imageIndex = 0; imageIndex < asset.images.size(); ++imageIndex)
			{
				const auto& fastImage = asset.images[imageIndex];
				imageArray[imageIndex] = std::visit(
					[&](const auto& source) -> TextureCPUData
					{
						using SourceT = std::decay_t<decltype(source)>;

						if constexpr (std::is_same_v<SourceT, fastgltf::sources::URI>)
						{
							const std::filesystem::path texturePath =
								modelPath.parent_path() / source.uri.string();

							auto registerResult = TextureManager.registerTexture(
								texturePath
							);
							if (!registerResult)
							{
								clz::log::error(
									"Unable to load texture: " +
									modelPath.string() + std::string(source.uri.c_str())
								);
								return TextureCPUData();
							}
							return registerResult.value();
						}
						else if constexpr (std::is_same_v<SourceT, fastgltf::sources::BufferView>)
						{
							const auto& bufferView = asset.bufferViews[source.bufferViewIndex];
							const auto& buffer     = asset.buffers[bufferView.bufferIndex];

							const std::byte* imageBytes = nullptr;
							const size_t imageDataOffset = bufferView.byteOffset;
							const size_t imageDataLength = bufferView.byteLength;

							if (const auto* arraySource = std::get_if<fastgltf::sources::Array>(&buffer.data))
								imageBytes = arraySource->bytes.data() + imageDataOffset;
							else if (const auto* vectorSource = std::get_if<fastgltf::sources::Vector>(&buffer.data))
								imageBytes = vectorSource->bytes.data() + imageDataOffset;

							CLZ_ASSERT(
								imageBytes != nullptr,
								"unsupported glTF buffer source type for embedded texture"
							);

							auto registerResult = TextureManager.registerTexture(
								imageBytes,
								imageDataLength,
								modelPath.string() + "texture"
							);
							if (!registerResult)
							{
								clz::log::error("Unable to load texture of model: " + modelPath.string());
								return TextureCPUData();
							}
							return registerResult.value();
						}
						else
						{
							clz::log::error(
								"Model: " + modelPath.string() +
								" has unsupported image format"
							);
							return TextureCPUData();
						}
					},
					fastImage.data
				);
			}
		}
	}

	void ModelManagerClass::loadSamplers(
		std::vector<GLTFStagingData> &gltfStagingDatas
	)
	{
		for (size_t modelIndex = 0; modelIndex < gltfStagingDatas.size(); ++modelIndex)
		{
			auto& gltfStagingData = gltfStagingDatas[modelIndex];
			if (!gltfStagingData.valid)
				continue;

			const auto& asset = gltfStagingData.fastAsset;
			auto& samplerArray = gltfStagingData.samplers;
			samplerArray.resize(asset.samplers.size());
			const auto& modelPath = m_modelPaths[gltfStagingData.modelId.getId()];
			for (size_t samplerIndex = 0; samplerIndex < asset.samplers.size(); ++samplerIndex)
			{
				auto loadSamplerData = [](const fastgltf::Sampler& sampler) -> SamplerData
				{
					SamplerData data{};

					// Magnification filter
					if (sampler.magFilter.has_value())
					{
						switch (sampler.magFilter.value())
						{
							case fastgltf::Filter::Nearest:
								data.magFilter = SamplerFilter::Nearest;
								break;
							default:
								data.magFilter = SamplerFilter::Linear;
						}
					}
					// Minification filter + mipmap mode
					if (sampler.minFilter.has_value())
					{
						switch (sampler.minFilter.value())
						{
							case fastgltf::Filter::Nearest:
								data.minFilter = SamplerFilter::Nearest;
								data.mipmapMode = MipmapMode::None;
								break;
							case fastgltf::Filter::Linear:
								data.minFilter = SamplerFilter::Linear;
								data.mipmapMode = MipmapMode::None;
								break;
							case fastgltf::Filter::NearestMipMapNearest:
								data.minFilter = SamplerFilter::Nearest;
								data.mipmapMode = MipmapMode::Nearest;
								break;
							case fastgltf::Filter::LinearMipMapNearest:
								data.minFilter = SamplerFilter::Linear;
								data.mipmapMode = MipmapMode::Nearest;
								break;
							case fastgltf::Filter::NearestMipMapLinear:
								data.minFilter = SamplerFilter::Nearest;
								data.mipmapMode = MipmapMode::Linear;
								break;
							case fastgltf::Filter::LinearMipMapLinear:
								data.minFilter = SamplerFilter::Linear;
								data.mipmapMode = MipmapMode::Linear;
								break;
						}
					}
					// Address modes
					switch (sampler.wrapS)
					{
						case fastgltf::Wrap::ClampToEdge:
							data.addressModeU = SamplerAddressMode::CLAMP_TO_EDGE;
							break;
						case fastgltf::Wrap::MirroredRepeat:
							data.addressModeU = SamplerAddressMode::MIRRORED_REPEAT;
							break;
						case fastgltf::Wrap::Repeat:
							data.addressModeU = SamplerAddressMode::REPEAT;
							break;
					}
					switch (sampler.wrapT)
					{
						case fastgltf::Wrap::ClampToEdge:
							data.addressModeV = SamplerAddressMode::CLAMP_TO_EDGE;
							break;
						case fastgltf::Wrap::MirroredRepeat:
							data.addressModeV = SamplerAddressMode::MIRRORED_REPEAT;
							break;
						case fastgltf::Wrap::Repeat:
							data.addressModeV = SamplerAddressMode::REPEAT;
							break;
					}

					// glTF has no W wrapping for a normal 2D texture.
					data.addressModeW = SamplerAddressMode::CLAMP_TO_EDGE;
					return data;
				};
				samplerArray[samplerIndex] =
					TextureManager.createTextureSampler(
						loadSamplerData(
							asset.samplers[samplerIndex]
						),
						modelPath.string() + std::to_string(samplerIndex)
					);
			}

		}
	}

	void ModelManagerClass::loadTextures(
		std::vector<GLTFStagingData> &gltfStagingDatas
	)
	{
		for (size_t modelIndex = 0; modelIndex < gltfStagingDatas.size(); ++modelIndex)
		{
			auto& gltfStagingData = gltfStagingDatas[modelIndex];
			if (!gltfStagingData.valid)
				continue;

			const auto& asset = gltfStagingData.fastAsset;
			auto& textureArray = gltfStagingData.textures;
			textureArray.resize(asset.textures.size());
			for (size_t textureIndex = 0; textureIndex < asset.textures.size(); ++textureIndex)
			{
				const auto& fastTexture = asset.textures[textureIndex];
				textureArray[textureIndex] = {
					.imageIndex = [index = fastTexture.imageIndex](){
						if (index.has_value())
							return static_cast<uint32_t>(index.value());
						return UINT32_MAX;
					}(),
					.samplerIndex = [index = fastTexture.samplerIndex](){
						if (index.has_value())
							return static_cast<uint32_t>(index.value());
						return UINT32_MAX;
					}()
				};
			}
		}
	}

	void ModelManagerClass::loadMaterials(
		std::vector<GLTFStagingData> &gltfStagingDatas
	)
	{
		for (size_t modelIndex = 0; modelIndex < gltfStagingDatas.size(); ++modelIndex)
		{
			auto& gltfStagingData = gltfStagingDatas[modelIndex];
			if (!gltfStagingData.valid)
				continue;

			const auto& asset = gltfStagingData.fastAsset;
			auto& imagesArray = gltfStagingData.images;
			const auto textureArray = gltfStagingData.textures;
			auto& materialArray = gltfStagingData.materials;
			materialArray.resize(asset.materials.size());

			for (size_t materialIndex = 0; materialIndex < asset.materials.size(); ++materialIndex)
			{
				const auto& fastMaterial = asset.materials[materialIndex];
				auto& material = materialArray[materialIndex];
				material = {};

				material.baseFactor = math::vec4(
					fastMaterial.pbrData.baseColorFactor.x(),
					fastMaterial.pbrData.baseColorFactor.y(),
					fastMaterial.pbrData.baseColorFactor.z(),
					1.0f
				);
				material.metallicFactor = fastMaterial.pbrData.metallicFactor;
				material.roughnessFactor = fastMaterial.pbrData.roughnessFactor;
				material.emissiveFactor = math::vec3(
					fastMaterial.emissiveFactor.x(),
					fastMaterial.emissiveFactor.y(),
					fastMaterial.emissiveFactor.z()
				);
				material.emissiveStrength = fastMaterial.emissiveStrength;

				if (fastMaterial.pbrData.baseColorTexture.has_value())
				{
					const auto textureIndex =
						fastMaterial.pbrData.baseColorTexture.value().textureIndex;
					material.baseTextureIndex = textureArray[textureIndex].imageIndex;
					material.baseSamplerIndex = textureArray[textureIndex].samplerIndex;

					imagesArray[material.baseTextureIndex].imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
				}
				else
				{
					material.baseTextureIndex = UINT32_MAX;
					material.baseSamplerIndex = UINT32_MAX;
				}
				if (fastMaterial.pbrData.metallicRoughnessTexture.has_value())
				{
					const auto textureIndex =
						fastMaterial.pbrData.metallicRoughnessTexture.value().textureIndex;
					material.metallicRoughnessTextureIndex = textureArray[textureIndex].imageIndex;
					material.metallicRoughnessSamplerIndex = textureArray[textureIndex].samplerIndex;

					imagesArray[material.metallicRoughnessTextureIndex].imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
				}
				else
				{
					material.metallicRoughnessTextureIndex = UINT32_MAX;
					material.metallicRoughnessSamplerIndex = UINT32_MAX;
				}
				if (fastMaterial.emissiveTexture.has_value())
				{
					const auto textureIndex =
						fastMaterial.emissiveTexture.value().textureIndex;
					material.emissiveTextureIndex = textureArray[textureIndex].imageIndex;
					material.emissiveSamplerIndex = textureArray[textureIndex].samplerIndex;

					imagesArray[material.emissiveTextureIndex].imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
				}
				else
				{
					material.emissiveTextureIndex = UINT32_MAX;
					material.emissiveSamplerIndex = UINT32_MAX;
				}

				if (fastMaterial.normalTexture.has_value())
				{
					const auto textureIndex =
						fastMaterial.normalTexture.value().textureIndex;
					material.normalTextureIndex = textureArray[textureIndex].imageIndex;
					material.normalSamplerIndex = textureArray[textureIndex].samplerIndex;

					imagesArray[material.normalTextureIndex].imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
				}
				else
				{
					material.normalTextureIndex = UINT32_MAX;
					material.normalSamplerIndex = UINT32_MAX;
				}

				material.ior = fastMaterial.ior;

				if (fastMaterial.specular)
				{
					const auto& spec = *fastMaterial.specular;

					material.specularFactor = spec.specularFactor;
					material.specularColorFactor = math::vec3(
						spec.specularColorFactor.x(),
						spec.specularColorFactor.y(),
						spec.specularColorFactor.z()
					);

					if (spec.specularTexture.has_value())
					{
						const auto ti = spec.specularTexture.value().textureIndex;
						material.specularTextureIndex = textureArray[ti].imageIndex;
						material.specularSamplerIndex = textureArray[ti].samplerIndex;
						imagesArray[material.specularTextureIndex].imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
					}
					else 
					{
						material.specularTextureIndex = UINT32_MAX;
						material.specularSamplerIndex = UINT32_MAX;
					}
					if (spec.specularColorTexture.has_value())
					{
						const auto ti = spec.specularColorTexture.value().textureIndex;
						material.specularColorTextureIndex = textureArray[ti].imageIndex;
						material.specularColorSamplerIndex = textureArray[ti].samplerIndex;
						imagesArray[material.specularColorTextureIndex].imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
					}
					else
					{
						material.specularColorTextureIndex = UINT32_MAX;
						material.specularColorSamplerIndex = UINT32_MAX;
					}
				}
				else
				{
					material.specularFactor = 1.5;
					material.specularColorFactor = math::vec3(1.0f);
					material.specularTextureIndex = UINT32_MAX;
					material.specularSamplerIndex = UINT32_MAX;
					material.specularColorTextureIndex = UINT32_MAX;
					material.specularColorSamplerIndex = UINT32_MAX;
				}
			}
		}
	}

	void ModelManagerClass::loadMeshes(
		std::vector<GLTFStagingData>& gltfStagingDatas
	)
	{
		for (size_t modelIndex = 0; modelIndex < gltfStagingDatas.size(); ++modelIndex)
		{
			auto& gltfStagingData = gltfStagingDatas[modelIndex];
			if (!gltfStagingData.valid)
				continue;

			const auto& asset = gltfStagingData.fastAsset;
			auto& meshArray = gltfStagingData.meshes;
			meshArray.resize(asset.meshes.size());

			for (size_t meshIndex = 0; meshIndex < asset.meshes.size(); ++meshIndex)
			{
				const auto& fastMesh = asset.meshes[meshIndex];
				auto& primitiveArray = meshArray[meshIndex].primitiveDatas;
				primitiveArray.resize(fastMesh.primitives.size());
				for (size_t primitiveIndex = 0; primitiveIndex < fastMesh.primitives.size(); ++primitiveIndex)
				{
					const auto& fastPrimitive = fastMesh.primitives[primitiveIndex];
					auto& primitive = primitiveArray[primitiveIndex];

					// --- Extract Positions ---
					const auto& positionIt = fastPrimitive.findAttribute("POSITION");
					if (positionIt != fastPrimitive.attributes.end())
					{
						const auto& positionAccessor = asset.accessors[positionIt->accessorIndex];
						primitive.attributes.resize(positionAccessor.count);
						fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(
							asset,
							positionAccessor,
							[&](const fastgltf::math::fvec3& pos, const std::size_t posIndex){
								primitive.attributes[posIndex]
									.position = {pos.x(), pos.y(), pos.z()};
							}
						);
					}
					else
					{
						for (size_t i = 0; i < primitive.attributes.size(); ++i)
						{
							primitive.attributes[i].position = {0.0f, 0.0f, 0.0f};
						}
					}


					// --- Extract UVs ---
					const auto& uvIt = fastPrimitive.findAttribute("TEXCOORD_0");
					if (uvIt != fastPrimitive.attributes.end())
					{
						const auto& uvAccessor = asset.accessors[uvIt->accessorIndex];
						fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>(
							asset,
							uvAccessor,
							[&](const fastgltf::math::fvec2& uv,
							    const std::size_t uvIndex) {
								primitive.attributes[uvIndex].uv = {uv.x(), uv.y()};
							}
						);
					}
					else
					{
						for (size_t i = 0; i < primitive.attributes.size(); ++i)
						{
							primitive.attributes[i].uv = {0.0f, 0.0f};
						}
					}
					// --- Extract Normals ---
					const auto& normalIt = fastPrimitive.findAttribute("NORMAL");
					if (normalIt != fastPrimitive.attributes.end())
					{
						const auto& normalAccessor = asset.accessors[normalIt->accessorIndex];
						fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(
							asset,
							normalAccessor,
							[&](const fastgltf::math::fvec3 normal, const size_t index)
							{
								primitive.attributes[index].normal = {
									normal.x(),
									normal.y(),
									normal.z()
								};
							}
						);
					}
					else
					{
						for (size_t i = 0; i < primitive.attributes.size(); ++i)
						{
							primitive.attributes[i].normal = {0.0f, 0.0f, 1.0f};
						}
					}

					// --- Calculate tangent and bitangent
					const auto &tangentIt = fastPrimitive.findAttribute("TANGENT");
					if (tangentIt != fastPrimitive.attributes.end())
					{
						const auto& tangentAccessor = asset.accessors[tangentIt->accessorIndex];
						fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec4>(
							asset,
							tangentAccessor,
							[&](const fastgltf::math::fvec4& tangent,
								const std::size_t index) {
								primitive.attributes[index].tangent = {
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
						for (size_t i = 0; i < primitive.attributes.size(); ++i)
						{
							primitive.attributes[i].tangent = {
								1.0f, 1.0f, 1.0f, 1.0f
							};
						}
					}

					// --- Extract Indices ---
					const auto& indexAccessor = asset.accessors[fastPrimitive.indicesAccessor.value()];
					primitive.indices.resize(indexAccessor.count);
					fastgltf::iterateAccessorWithIndex<uint32_t>(
						asset,
						indexAccessor,
						[&](const uint32_t drawIndex, const size_t it) {
							primitive.indices[it] = drawIndex;
						}
					);

					primitive.attributesSize = primitive.attributes.size() * sizeof(primitive.attributes[0]);
					primitive.indicesSize =  primitive.indices.size() * sizeof(primitive.indices[0]);

					/// --- Extract material index --- //
					if (fastPrimitive.materialIndex)
						primitive.materialIndex = fastPrimitive.materialIndex.value();
					else
						primitive.materialIndex = UINT32_MAX;
				}
			}
		}
	}

	static inline uint32_t loadIndivisualNode(
		const fastgltf::Node& fastNode,
		const fastgltf::Asset& asset,
		const uint32_t parentNodeIndex,
		std::vector<Node>& nodeArray,
		ModelMatrixManager& matrixManager
	)
	{
		auto retrieveTransform = [&matrixManager](
			Node& node,
			const fastgltf::Node& fastNode
		)
		{
			node.matrixId = matrixManager.allocate();
			std::visit(
				[&](const auto& transform)
				{
					using T = std::decay_t<decltype(transform)>;

					if constexpr (std::is_same_v<T, fastgltf::math::fmat4x4>)
					{
						// fastgltf's fmat4x4 is column-major
						memcpy(&node.localTransform, transform.data(), sizeof(float) * 16);
					}
					else if constexpr (std::is_same_v<T, fastgltf::TRS>)
					{
						const math::vec3 translation(
							transform.translation[0],
							transform.translation[1],
							transform.translation[2]
						);
						// fastgltf stores x,y,z,w. Our math library expects w,x,y,z
						const math::quat rotation(
							transform.rotation[3],
							transform.rotation[0],
							transform.rotation[1],
							transform.rotation[2]
						);
						const math::vec3 scale(
							transform.scale[0],
							transform.scale[1],
							transform.scale[2]
						);

						node.localTransform = math::getModelMatrix(rotation, translation, scale);
					}
				},
				fastNode.transform
			);
			matrixManager.setMatrix(node.matrixId, node.localTransform);
		};
		auto retrieveMeshIndex = [](Node& node, const fastgltf::Node& fastNode)
		{
			const auto& meshIndex = fastNode.meshIndex;
			if (!meshIndex.has_value())
				node.meshIndex = UINT32_MAX;
			else
				node.meshIndex = meshIndex.value();
		};
		Node node;
		node.ParentIndex = parentNodeIndex;
		retrieveTransform(node, fastNode);
		retrieveMeshIndex(node, fastNode);
		nodeArray.push_back(std::move(node));
		uint32_t thisNodeIndex = nodeArray.size() - 1;
		auto& nodeChildrenArray = nodeArray[thisNodeIndex].children;

		for (const auto& childIndex : fastNode.children)
		{
			uint32_t childNodeIndex = loadIndivisualNode(
				asset.nodes[childIndex],
				asset,
				thisNodeIndex,
				nodeArray,
				matrixManager
			);

			nodeChildrenArray.push_back(childNodeIndex);
		}

		return thisNodeIndex;
	}

	void ModelManagerClass::loadNodes(
		std::vector<GLTFStagingData>& gltfStagingDatas
	)
	{
		for (size_t modelIndex = 0; modelIndex < gltfStagingDatas.size(); ++modelIndex)
		{
			auto& gltfStagingData = gltfStagingDatas[modelIndex];
			if (!gltfStagingData.valid)
				continue;

			const auto& asset = gltfStagingData.fastAsset;
			auto& nodeArray = m_models[gltfStagingData.modelId.getId()].nodes;
			nodeArray.reserve(asset.nodes.size());
			auto& rootNodeIndicesArray = m_models[gltfStagingData.modelId.getId()].rootNodesIndices;

			for (const auto& scene : asset.scenes)
			{
				for (const auto& nodeIndex : scene.nodeIndices)
					rootNodeIndicesArray.push_back(
						loadIndivisualNode(
							asset.nodes[nodeIndex],
							asset,
							UINT32_MAX,
							nodeArray,
							m_matrixManager
						)
					);
			}
		}
	}

	void ModelManagerClass::submitGPUData(
		std::vector<GLTFStagingData>& gltfStagingDatas
	)
	{
		VkDeviceSize totalSize = 0;
		uint64_t numVertexBufferRegions = 0;
		uint64_t numIndexBufferRegions = 0;
		for (auto& data : gltfStagingDatas)
		{
			for (auto& mesh : data.meshes)
			{
				for (auto& primitive : mesh.primitiveDatas)
				{
					primitive.attributeOffset = totalSize;
					totalSize += primitive.attributesSize;
					primitive.indiceOffset = totalSize;
					totalSize += primitive.indicesSize;

					++numVertexBufferRegions;
					++numIndexBufferRegions;
				}
			}

			for (auto& imageData : data.images)
			{
				imageData.memoryOffset = totalSize;
				totalSize += imageData.textureSize;
			}
		}
		const auto& device = r_deviceContext.device;
		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
		const VkBufferCreateInfo bufferCI{
			.sType 		= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext 		= nullptr,
			.flags 		= 0,
			.size  		= totalSize,
			.usage 		= VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			.sharingMode 	= VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices   = nullptr
		};
		if (vkCreateBuffer(device, &bufferCI, nullptr, &stagingBuffer) != VK_SUCCESS)
		{
			clz::log::error("Vulkan failed to create a model staging buffer");
			return;
		}
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(
			device,
			stagingBuffer,
			&memRequirements
		);
		const VkMemoryAllocateInfo memAllocInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = nullptr,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(
						memRequirements.memoryTypeBits,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
					   )
		};
		if (vkAllocateMemory(device, &memAllocInfo, nullptr, &stagingBufferMemory) != VK_SUCCESS)
		{
			clz::log::error("Vulkan failed to create a model staging buffer memory");
			return;
		}
		vkBindBufferMemory(r_deviceContext.device, stagingBuffer, stagingBufferMemory, 0);

		VkCommandBuffer commandBuffer = startSingleTimeCommand();

		std::vector<VkBufferCopy> vertexBufferCopyDatas;
		std::vector<VkBufferCopy> indexBufferCopyDatas;
		vertexBufferCopyDatas.reserve(numVertexBufferRegions);
		indexBufferCopyDatas.reserve(numIndexBufferRegions);

		for (auto& data : gltfStagingDatas)
		{
			const ModelId modelId = data.modelId;

			auto& meshArray = m_models[modelId.getId()].meshes;
			meshArray.resize(data.meshes.size());
			for (size_t meshIndex = 0; meshIndex < data.meshes.size(); ++meshIndex)
			{
				auto& meshData = data.meshes[meshIndex];
				auto& mesh = meshArray[meshIndex];
				mesh.primitives.resize(meshData.primitiveDatas.size());
				for (size_t primitiveIndex = 0; primitiveIndex < meshData.primitiveDatas.size(); ++primitiveIndex)
				{
					auto [vertexRegion, vertexCopyData] = 
						StaticBufferManager.recordVerticesSubmission(
							meshData.primitiveDatas[primitiveIndex].attributes,
							commandBuffer,
							stagingBuffer,
							meshData.primitiveDatas[primitiveIndex].attributeOffset,
							stagingBufferMemory
						);
					mesh.primitives[primitiveIndex].vertexBuffer = vertexRegion;
					vertexBufferCopyDatas.push_back(std::move(vertexCopyData));

					auto [indiceRegion, indiceCopyData] = 
						StaticIndexManager.recordIndicesSubmission(
							meshData.primitiveDatas[primitiveIndex].indices,
							commandBuffer,
							stagingBuffer,
							meshData.primitiveDatas[primitiveIndex].indiceOffset,
							stagingBufferMemory
						);
					mesh.primitives[primitiveIndex].indexBuffer = indiceRegion;
					indexBufferCopyDatas.push_back(std::move(indiceCopyData));

					mesh.primitives[primitiveIndex].materialIdIndex =
						meshData.primitiveDatas[primitiveIndex].materialIndex;
				}
			}
			auto& imageArray = data.images;
			for (auto& imageData : imageArray)
			{
				TextureManager.recordSubmitTextureDataToGPUCommands(
					imageData,
					commandBuffer,
					stagingBuffer,
					stagingBufferMemory,
					imageData.memoryOffset,
					imageData.memoryOffset
				);
			}
		}

		StaticBufferManager.flushCopies(vertexBufferCopyDatas, commandBuffer, stagingBuffer);
		StaticIndexManager.flushCopies(indexBufferCopyDatas, commandBuffer, stagingBuffer);

		submitSingleTimeCommand(commandBuffer);
		vkDestroyBuffer(r_deviceContext.device, stagingBuffer, nullptr);
		vkFreeMemory(r_deviceContext.device, stagingBufferMemory, nullptr);


		/// submit material data
		for (auto& data : gltfStagingDatas)
		{
			auto& materialDataArray = data.materials;
			auto& materialArray = m_models[data.modelId.getId()].materials;
			auto& imageArray = data.images;
			auto& samplerArray = data.samplers;

			materialArray.resize(materialDataArray.size());
			for (size_t materialIndex = 0; materialIndex < materialArray.size(); ++materialIndex)
			{
				auto& materialData = materialDataArray[materialIndex];
				ModelMaterial material;

				auto resolveSampler = [&](const uint32_t samplerIndex) -> SamplerId
				{
					return (samplerIndex == UINT32_MAX)
						? TextureManager.getDefaultSamplerId()
						: samplerArray[samplerIndex];
				};
				auto resolveTexture = [&](const uint32_t textureIndex) -> TextureId
				{
					return (textureIndex == UINT32_MAX)
						? TextureId()
						: imageArray[textureIndex].stagingId;
				};

				material.baseFactor = materialData.baseFactor;
				material.baseTextureId = resolveTexture(materialData.baseTextureIndex);
				material.baseSamplerId = resolveSampler(materialData.baseSamplerIndex);

				material.metallicRoughnessTextureId = resolveTexture(materialData.metallicRoughnessTextureIndex);
				material.metallicRoughnessSamplerId = resolveSampler(materialData.metallicRoughnessSamplerIndex);
				material.metallicFactor = materialData.metallicFactor;
				material.roughnessFactor = materialData.roughnessFactor;

				material.emissiveTextureId = resolveTexture(materialData.emissiveTextureIndex);
				material.emissiveSamplerId = resolveSampler(materialData.emissiveSamplerIndex);
				material.emissiveFactor = materialData.emissiveFactor;
				material.emissiveStrength = materialData.emissiveStrength;

				material.normalTextureId = resolveTexture(materialData.normalTextureIndex);
				material.normalSamplerId = resolveSampler(materialData.normalSamplerIndex);

				material.ior = materialData.ior;

				material.specularFactor = materialData.specularFactor;
				material.specularColorFactor = materialData.specularColorFactor;
				material.specularTextureId = resolveTexture(materialData.specularTextureIndex);
				material.specularSamplerId = resolveSampler(materialData.specularSamplerIndex);
				material.specularColorTextureId = resolveTexture(materialData.specularColorTextureIndex);
				material.specularColorSamplerId = resolveSampler(materialData.specularColorSamplerIndex);

				materialArray[materialIndex] = m_materialManager.submitMaterial(material);
			}
		}
	}
} // namespace clz::renderer
