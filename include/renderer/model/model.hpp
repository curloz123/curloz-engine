/**
 * @file model.hpp
 * @author curl0z
 * @brief Handles all model loading in engine.
 *
 * Has kind of weird loading mechanism. At first, loads all glTF arrays
 * like samplers, textures, images etc.. Then at end everything is
 * connected via indices defined in the schema.
 *
 * @TODO add model deletion function.
 */

#pragma once

#include "core/id_interface.hpp"
#include "entity/corecomponents.hpp"
#include "entity/entitymanager.hpp"
#include "fastgltf/types.hpp"
#include "math/mat4x4.hpp"
#include "math/vec4.hpp"
#include "core/secure_types.hpp"
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
#include "renderer/pipelinedata/pipeline_data.hpp"

namespace clz::renderer
{
	/// @brief Model Id stores index in model's array.
	class ModelId : public IdInterface<uint32_t>{};

	/// All defined below maps gltf's mental model ///

	/// glTF's Mesh's primitive data
	struct PrimitiveStagingData
	{
		std::vector<StaticMesh> attributes;
		std::vector<uint32_t> indices;

		uint64_t attributesSize = 0;
		VkDeviceSize attributeOffset = 0;
		uint64_t indicesSize = 0;
		VkDeviceSize indiceOffset = 0;
		SecureType_t<Types::U32> materialIndex;
	};

	///< @brief glTF's Mesh Data.
	struct MeshStagingData
	{
		std::vector<PrimitiveStagingData> primitiveDatas;
	};

	///< @brief glTF's texture data.
	struct TextureStagingData
	{
		SecureType_t<Types::U32> imageIndex;
		SecureType_t<Types::U32> samplerIndex;
	};

	///< @brief glTF's material data
	struct MaterialStagingData
	{
		SecureType_t<Types::U32> baseTextureIndex;
		SecureType_t<Types::U32> baseSamplerIndex;
		math::vec4 baseFactor = math::vec4(0.0f, 0.0f, 0.0f, 1.0f);

		SecureType_t<Types::U32> metallicRoughnessTextureIndex;
		SecureType_t<Types::U32> metallicRoughnessSamplerIndex;
		float metallicFactor;
		float roughnessFactor;

		SecureType_t<Types::U32> emissiveTextureIndex;
		SecureType_t<Types::U32> emissiveSamplerIndex;
		math::vec3 emissiveFactor;
		float emissiveStrength;

		SecureType_t<Types::U32> normalTextureIndex;
		SecureType_t<Types::U32> normalSamplerIndex;

		float ior = 0.04;

		float specularFactor;
		math::vec3 specularColorFactor;
		SecureType_t<Types::U32> specularTextureIndex;
		SecureType_t<Types::U32> specularSamplerIndex;
		SecureType_t<Types::U32> specularColorTextureIndex;
		SecureType_t<Types::U32> specularColorSamplerIndex;
	};

	///< @brief Engine's internal gltf staging data
	///< @note Nodes don't require any staging data as such.
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
		SecureType_t<Types::U32> materialIdIndex;
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
		/// Nodes local transform, as defined in gltf file
		math::mat4 localTransform;

		/// @brief Index of the mesh associated with this node, if any.
		SecureType_t<Types::U32> meshIndex;

		/// Parent node's index in node array.
		SecureType_t<Types::U32> ParentIndex;

		/// Child nodes in the scene hierarchy.
		std::vector<uint32_t> children;
	};


	/// @brief Engine's internal model structure.
	struct Model
	{
		/// Indices of all root nodes.
		std::vector<uint32_t> rootNodesIndices;

		/// All nodes.
		std::vector<Node> nodes;

		/// List of all meshes
		std::vector<Mesh> meshes;

		/// Array of indices of all materials.
		std::vector<ModelMaterialId> materials;
	};

	/// Temporary load data, required while loading.
	struct ModelLoadData{
		ModelId modelId;
		fastgltf::Asset asset;
	};

	/**
	 * @brief Model manager class.
	 */
	class ModelManagerClass
	{
	private:
		/// Global parser
		fastgltf::Parser m_parser;

		/// Matrix manager
		ModelMatrixManager m_matrixManager;

		/// Material manager
		ModelMaterialManager m_materialManager;

		/// Array storing all models.
		std::vector<Model> m_models;

		/// Array storing all model paths
		std::vector<std::filesystem::path> m_modelPaths;
		/// Array storing array to all entities that
		/// use that particular model.
		std::vector<std::vector<ecs::entity>> m_modelEntities;

		/// Model array gaps
		std::vector<ModelId> m_gaps;
		/// If a new model is added, where to add it.
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
			const ModelMatrixRange& matricesRange,
			const uint32_t nodeIndex,
			const math::mat4& parentTransform,
			const bool dirty,
			VkCommandBuffer commandBuffer
		);

	public:
		/**
		 * @brief Initializes model manager.
		 * Simply initialzes the parser,
		 * material and matrix manager.
		 * @return false on failure, success otherwise.
		 */
		bool initialize();

		/// @brief Cleans up all model data.
		void close();

		/**
		 * @brief Loads a batch of models
		 * Workflow goes like this ->
		 * First retrieve a model id, if gaps have none,
		 * increment the counter and make new.
		 * Then create a global array holding all glTF's staging data,
		 * size to num of paths.
		 * If failure happens, we let id leak so as to not
		 * disturb the actual JSON data.
		 */
		void loadBatch(const std::span<std::filesystem::path> modelPaths);

		/// @brief Frees a model and its resources
		/// @note Not to be called during engine closure,
		/// this is a on-play function
		void freeModelResources(ModelId* pModelId);

		void drawIndivisual(
			const ModelId modelId,
			const ModelMatrixRange& matricesRange,
			const ecs::TransformComponent& transform,
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

		ModelMaterialManager& getMaterialManager()
		{
			return m_materialManager;
		}

		ModelMatrixManager& getMatrixManager()
		{
			return m_matrixManager;
		}

		/// This denotes null model path, used to pass wnv JSON has null index,
		/// or when a model is deleted.
		static inline constexpr std::string NULL_MODEL_PATH = "lmao_XD";

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

		std::expected<uint32_t, std::string> getMatrixCount(const ModelId modelId) const
		{
			if (modelId.getId() > m_models.size())
			{
				const std::string error =
					"Attemp to retrieve matrix count for non existing model Id";

				clz::log::error(error);
				return std::unexpected(error);
			}

			return m_models[modelId.getId()].nodes.size();
		}

	};

	inline ModelManagerClass ModelManager;

} // namespace clz::renderer
