/**
 * @file model.hpp
 * @curl0z
 * @brief Defines the data structures, classes, and functions
 * for loading, managing and rendering 3D models.
 */

#pragma once

#include "core/id_interface.hpp"
#include "entity/entitymanager.hpp"
#include "fastgltf/types.hpp"
#include "math/mat4x4.hpp"
#include "math/quat.hpp"
#include "math/vec4.hpp"
#include "renderer/entitydata/texture.hpp"
#include "renderer/entitydata/buffer.hpp"
#include <cstdint>
#include <fastgltf/core.hpp>
#include <filesystem>
#include <vector>
#include <vulkan/vulkan.h>
#include "model_matrix.hpp"
#include "model_material.hpp"
#include <span>

namespace clz::renderer
{

	class ModelId : public IdInterface<uint32_t>{};

	struct PrimitiveStagingData
	{
		std::vector<StaticMesh> attributes;
		std::vector<uint32_t> indices;

		uint64_t attributesSize = 0;
		VkDeviceSize attributeOffset = 0;
		uint64_t indicesSize = 0;
		VkDeviceSize indiceOffset = 0;
		uint32_t materialIndex = UINT32_MAX;
	};
	struct MeshStagingData
	{
		std::vector<PrimitiveStagingData> primitiveDatas;
	};

	struct TextureStagingData
	{
		uint32_t imageIndex = UINT32_MAX;
		uint32_t samplerIndex = UINT32_MAX;
	};
	struct MaterialStagingData
	{
		uint32_t baseTextureIndex = UINT32_MAX;
		uint32_t baseSamplerIndex = UINT32_MAX;
		math::vec4 baseFactor = math::vec4(0.0f, 0.0f, 0.0f, 1.0f);

		uint32_t metallicRoughnessTextureIndex = UINT32_MAX;
		uint32_t metallicRoughnessSamplerIndex = UINT32_MAX;
		float metallicFactor;
		float roughnessFactor;

		uint32_t emissiveTextureIndex = UINT32_MAX;
		uint32_t emissiveSamplerIndex = UINT32_MAX;
		math::vec3 emissiveFactor;
		float emissiveStrength;

		uint32_t normalTextureIndex = UINT32_MAX;
		uint32_t normalSamplerIndex = UINT32_MAX;

		float ior = 0.04;

		float specularFactor;
		math::vec3 specularColorFactor;
		uint32_t specularTextureIndex = UINT32_MAX;
		uint32_t specularSamplerIndex = UINT32_MAX;
		uint32_t specularColorTextureIndex = UINT32_MAX;
		uint32_t specularColorSamplerIndex = UINT32_MAX;
	};

	/// Nodes don't require any staging data as such.
	struct GLTFStagingData
	{
		ModelId modelId;
		fastgltf::Asset fastAsset;
		bool valid = false;

		std::vector<TextureCPUData> images;
		std::vector<MeshStagingData> meshes;

		std::vector<SamplerId> samplers;

		std::vector<TextureStagingData> textures;

		std::vector<MaterialStagingData> materials;
	};

	/**
	 * @struct Primitive
	 * @brief Represents a single drawable primitive with GPU-side buffer offsets and
	 * texture bindings.
	 */
	struct Primitive
	{
		/// --- 1. Vertices data ---
		VertexRange vertexBuffer;
		IndexRange indexBuffer;

		/// --- 2. Material Data ---
		uint32_t materialIdIndex = UINT32_MAX;
	};

	/**
	 * @struct Mesh
	 * @brief A collection of primitives that make up a single mesh.
	 */
	struct Mesh
	{
		/// @brief List of primitives contained in this mesh
		std::vector<Primitive> primitives;
	};

	/**
	 * @struct Node
	 * @brief Represents a node in the glTF scene graph hierarchy.
	 */
	struct Node
	{
		math::mat4 localTransform;
		/// @brief Local transformation matrix relative to its parent.
		ModelMatrixId matrixId;

		/// @brief Index of the mesh associated with this node, if any.
		uint32_t meshIndex = UINT32_MAX;

		uint32_t ParentIndex = UINT32_MAX;
		/// @brief Child nodes in the scene hierarchy.
		std::vector<uint32_t> children;

		bool dirty = true;
	};


	struct Model
	{
		std::vector<uint32_t> rootNodesIndices;

		/// @brief Root nodes of the scene graph.
		std::vector<Node> nodes;

		/// @brief List of meshes (optional to handle sparse indices).
		std::vector<Mesh> meshes;

		std::vector<ModelMaterialId> materials;
	};
	struct ModelLoadData{
		ModelId modelId;
		fastgltf::Asset asset;
	};

	class ModelManagerClass
	{
	private:
		fastgltf::Parser m_parser;

		ModelMatrixManager m_matrixManager;
		ModelMaterialManager m_materialManager;

		std::vector<Model> m_models;
		std::vector<std::filesystem::path> m_modelPaths;
		std::vector<std::vector<ecs::entity>> m_modelEntities;

		std::vector<ModelId> m_gaps;
		uint32_t m_newModelIndex = 0;


		void loadImages(
			std::vector<GLTFStagingData>& gltfStagingDatas
		);

		void loadSamplers(
			std::vector<GLTFStagingData>& gltfStagingDatas
		);

		void loadTextures(
			std::vector<GLTFStagingData>& gltfStagingDatas
		);

		/// material must be loaded after images and textures
		void loadMaterials(
			std::vector<GLTFStagingData>& gltfStagingDatas
		);

		void loadMeshes(
			std::vector<GLTFStagingData>& gltfStagingDatas
		);

		void loadNodes(
			std::vector<GLTFStagingData>& gltfStagingDatas
		);

		void submitGPUData(
			std::vector<GLTFStagingData>& gltfStagingDatas
		);

		void drawNode(
			const ModelId modelId,
			const uint32_t nodeIndex,
			const math::mat4& parentTransform,
			VkCommandBuffer commandBuffer
		);

	public:
		bool initialize();
		void close();

		ModelId loadBatch(const std::span<std::filesystem::path> modelPaths);

		void drawIndivisual(
			ModelId modelId,
			const math::vec3& position,
			const math::quat& rotation,
			const math::vec3& scale,
			VkCommandBuffer commandBuffer
		);

		void drawAll(
			VkCommandBuffer commandBuffer
		);

		void loadNodes(
			const std::vector<ModelLoadData>& loadDatas
		);

		void loadMeshesAndMaterials(
			const std::vector<ModelLoadData>& loadDatas
		);

		const ModelMaterialManager& getMaterialManager() const
		{
			return m_materialManager;
		}

		const ModelMatrixManager& getMatrixManager() const
		{
			return m_matrixManager;
		}

		std::string getModelPath(const ModelId modelId) const
		{
			return m_modelPaths[modelId.getId()].string();
		}

		const auto& getModelPaths() const
		{
			return m_modelPaths;
		}

		bool attachModelToEntity(
			const ecs::entity entt, 
			const uint32_t modelIndex
		)
		{
			if (modelIndex > m_models.size())
			{
				clz::log::error("Attempt to retrieve out of bound model");
				return false;
			}

			m_modelEntities[modelIndex].push_back(entt);
			return true;
		}

	};

	inline ModelManagerClass ModelManager;

} // namespace clz::renderer
