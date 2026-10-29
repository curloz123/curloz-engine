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
	class ModelMaterialId : public IdInterface<uint32_t>{};

	constexpr inline uint32_t MAX_MATERIAL_COUNT = 512;

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

		float ior;

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
		SSBO m_ssbo;
		std::vector<ModelMaterialId> m_freeIndices;
		uint32_t m_nextFreshIndex = 0;
		uint32_t m_capacity;

	public:
		bool initialize(
			const uint32_t modelMaterialCount = MAX_MATERIAL_COUNT
		);

		ModelMaterialId submitMaterial(
			const ModelMaterial& modelMaterial
		);

		void destroy(const bool explicitDestroy = false);

		void freeMaterial(ModelMaterialId& id);

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
