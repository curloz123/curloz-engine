/**
 * @file modeldata.hpp
 * @brief Model/mesh asset types and SoA lookup tables for the model loading pipeline.
 *
 * Models are loaded via Assimp and decomposed into meshes. Mesh data itself
 * (vertices/indices/UVs) lives in the global buffer arrays (VBuffer/IBuffer/UVBuffer);
 * this header only defines the ID types that references the
 * meshes and base-color materials are laid out within those buffers.
 */

#pragma once

#include "renderer/shaderdata/descriptor/texture.hpp"
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

namespace clz::renderer
{

	/// Identifies a loaded model (index into Asset::r_modelLUT).
	using ModelID = uint32_t;
	/// Identifies a single mesh belonging to a model (index into mesh::meshLUT).
	using ModelMeshID = uint32_t;
	/// Identifies a material entry associated with a mesh (index into mesh::baseMaterialLUT).
	using ModelMaterialID = uint32_t;

	/// Sentinel value indicating no valid model/asset was produced.
	constexpr uint32_t NULL_ASSET = std::numeric_limits<ModelID>::max();
	/// Sentinel value indicating no valid mesh data (e.g. no vertices/indices) was produced.
	constexpr uint32_t NULL_MESH = std::numeric_limits<ModelID>::max();
	/// Sentinel value indicating no valid material/UV data was produced.
	constexpr uint32_t NULL_MATERIAL = std::numeric_limits<ModelID>::max();

	namespace mesh
	{

		/**
		 * @brief describer per-mesh base-color material data.
		 * Each index `i` across all three vectors describes one mesh's base material:
		 *
		 * @warning when binding UVBuffer in main rendering, base vertex would be
		 * decided by VBuffer, not UVBuffer. This is to maintain same indexing
		 * and is a must-have thing.
		 */
		struct BaseMaterials
		{
			std::vector<TextureID> texture; ///< Base-color texture used by this mesh.
		};

		/**
		 * @brief describes per-mesh draw/index data.
		 *
		 * Parallel arrays indexed by ModelMeshID. `material[i]` is an index into
		 * BaseMaterials describing the material used to render mesh `i`.
		 */
		struct Meshes
		{
			std::vector<uint32_t> indexCount;      ///< Number of indices belonging to this mesh.
			std::vector<uint32_t> firstIndex;      ///< Offset of this mesh's first index in the global index buffer.
			std::vector<int32_t> baseVertex;       ///< Base offset into the global vertex buffer for this mesh's vertices.
			std::vector<ModelMaterialID> material; ///< Index into BaseMaterials for this mesh's material.
		};

		/// Global Look up Table of base-color material data, one entry per mesh.
		inline BaseMaterials baseMaterialLUT;
		/// Global Look up Table of mesh draw data, one entry per mesh.
		inline Meshes meshLUT;
	} // namespace mesh

	namespace Asset
	{
		/**
		 * @brief A loaded model, expressed as a list of mesh IDs.
		 */
		struct Model
		{
			std::vector<ModelMeshID> meshes; ///< Meshes that make up this model.
		};

		/// Global table of all loaded models, indexed by ModelID.
		inline std::vector<Model> r_modelLUT;
		/// Total number of models loaded so far; also used to allocate the next ModelID.
		inline uint32_t r_numModelCount = 0;

		/**
		 * @brief Temporary per-model cache mapping texture file paths to TextureID.
		 *
		 * Used to avoid reprocessing the same texture file multiple times while
		 * loading a single model. Must be cleared after each call to loadModel().
		 */
		inline std::unordered_map<std::string, TextureID> textureLoadedThisModel;

		/**
		 * @brief Loads a model file from disk via Assimp and registers its meshes/materials.
		 *
		 * @param modelPath Filesystem path to the model file to load.
		 * @return The ModelID of the newly loaded model, or NULL_ASSET on failure.
		 */
		ModelID loadModel(const std::string& modelPath);

		/// Name's (basically path) of all models
		inline std::vector<std::string> modelName;
		/**
		 * @brief Retrieve's name (basically path) of a model
		 * @param id Model's ID
		 * @return name std::string name
		 */
		std::string getModelName(const ModelID id);
	} // namespace Asset

} // namespace clz::renderer