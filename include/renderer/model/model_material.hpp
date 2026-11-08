/**
 * @file model_material.hpp
 * @author curl0z
 * @brief Provides each model's material data.
 * Each glTF file has materials, that is passed to shaders.
 * Materials are stored in ssbo array, indexed by material Ids.
 */
#pragma once

#include "core/id_interface.hpp"
#include "math/vec4.hpp"
#include "math/vec3.hpp"
#include "math/vec2.hpp"
#include "renderer/entitydata/texture.hpp"
#include "renderer/utility/ssbo.hpp"
#include <fastgltf/core.hpp>

namespace clz::renderer
{
	constexpr inline uint32_t MAX_MATERIAL_COUNT = 1024;

	class ModelMaterialId : public IdInterface<uint32_t>{};

	/// @brief The SSBO that'll go to GPU
	struct ModelMaterial
	{
		/// @brief Base color factor
		math::vec4 baseFactor;
		/// @brief ID of the bound base color texture
		TextureId baseTextureId;
		SamplerId baseSamplerId;

		/// @brief ID of metallic texture
		TextureId metallicRoughnessTextureId;
		SamplerId metallicRoughnessSamplerId;
		/// @brief metallic color factor
		float metallicFactor;
		/// @brief roughness color factor
		float roughnessFactor;

		/// @brief ID of emissive texture
		TextureId emissiveTextureId;
		/// @brief emissive color strength
		float emissiveStrength;
		/// @brief emissive factor
		math::vec3 emissiveFactor;
		SamplerId emissiveSamplerId;

		/// @brief ID of normal texture
		TextureId normalTextureId;
		SamplerId normalSamplerId;

		/// Index of refraction
		float ior;

		/// Specular Data
		float specularFactor;
		math::vec3 specularColorFactor;
		TextureId specularTextureId;
		SamplerId specularSamplerId;
		TextureId specularColorTextureId;
		SamplerId specularColorSamplerId;

		float pad;
	};

	class ModelMaterialManager
	{
	private:
		/// Actual storage buffer
		SSBO m_ssbo;

		/// CPU side stored materials.
		/// Helpful in deletion of inbuilt textures.
		std::vector<ModelMaterial> m_cachedMaterials;

		/// Holds free indices. Whenever new material has to be allocated,
		/// free index will be checked from here first.
		std::vector<ModelMaterialId> m_freeIndices;

		/// Actual new start index
		uint32_t m_nextFreshIndex = 0;

		/// Max'm capacity of materials.
		uint32_t m_capacity;

	public:
		/**
		 * @brief Initializes the materials storage buffer,
		 * and other stuff for it.
		 * @param modelMaterialCount Size to allocate to storage buffer
		 * in terms of total number of materials that can be allocated.
		 * @return true on success, false otherwise.
		 */
		bool initialize(
			const uint32_t modelMaterialCount = MAX_MATERIAL_COUNT
		);

		/**
		 * @brief Submits a material to storage buffer.
		 * @param modelMaterial Actual material data.
		 * @return ModelMaterialId, basically index into array.
		 */
		ModelMaterialId submitMaterial(
			const ModelMaterial& modelMaterial
		);

		/**
		 * @brief Closes the material manager.
		 * @param explicitDestroy Tells whether to destroy Exactly Now!!!
		 * No by default, engine will cleanup resources at end.
		 */
		void destroy(const bool explicitDestroy = false);

		/**
		 * @brief Frees a material from storage buffer
		 * This id will be used as a free resource for other materials to use.
		 * Passed id is null after this.
		 * @param id Material's Id to make null.
		 * @note Don't iterate this function over all models at ALL!!
		 * Those resources will be cleared up on their own.
		 */
		void freeMaterial(
			ModelMaterialId& id
		);

		uint32_t getMaterialArrayLimit() const
		{
			return m_nextFreshIndex;
		}

		const SSBO& getStorageBuffer() const
		{
			return m_ssbo;
		}
	};
}
