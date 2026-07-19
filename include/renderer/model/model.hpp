#pragma once

#include <filesystem>
#include <vulkan/vulkan.h>
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/mat4x4.hpp"
#include "math/quat.hpp"
#include <vector>
#include <expected>
#include "renderer/entitydata/texture.hpp"
#include <fastgltf/core.hpp>

namespace clz::renderer
{
	struct PrimitiveData
	{
		std::vector<math::vec3> vertices;
		std::vector<math::vec2> uvs;
		std::vector<uint32_t> indices;

		std::optional<std::variant<fastgltf::sources::URI,
			fastgltf::sources::BufferView>> baseTexture;
		std::optional<size_t> baseColorTextureIndex;

	};

	struct Primitive
	{
		uint32_t indexCount;
		uint32_t firstIndex;
		uint32_t baseVertexIndex;
		std::optional<TextureID> baseTextureId;
	};
	struct Mesh
	{
		std::vector<Primitive> primitives;
	};


	struct Node
	{
		std::optional<uint32_t> meshIndex;
		math::mat4 localTransform;
		std::vector<Node> children;
	};


	struct ModelPath
	{
		std::filesystem::path path;
		ModelPath() = default;
		ModelPath(const std::filesystem::path& modelPath) :
			path(modelPath)
		{
		}
	};

	using ModelId = uint32_t;
	struct Model
	{
		std::vector<std::optional<Mesh>> meshes;
		std::vector<Node> nodes;

		void draw(const math::mat4& modelMatrix,
			VkCommandBuffer commandBuffer);
		void drawNode(const auto& node,
			const math::mat4& parentTransform,
			VkCommandBuffer commandBuffer);

		Node loadNode(const fastgltf::Node& node,
			const fastgltf::Asset& asset,
			ModelId registeredId);

		Mesh loadMesh(const fastgltf::Mesh& mesh,
			const fastgltf::Asset& asset,
			ModelId registeredId);


		Primitive registerPrimitive(const PrimitiveData& primitiveData,
			const fastgltf::Asset& asset,
			ModelId registeredId);
	};

	constexpr uint32_t NULL_MODEL =
		std::numeric_limits<ModelId>::max();

	inline std::vector<Model> ModelLUT;
	inline std::vector<ModelPath> ModelPaths;

	struct TextureCache
	{
		std::unordered_map<size_t, TextureID> texturesLoaded;
		void clear()
		{
			texturesLoaded.clear();
		}
	};
	inline std::vector<TextureCache> TextureCaches;
}

namespace clz::renderer
{
	std::expected<ModelId, std::string>
	loadModel(const std::filesystem::path& filePath);

	void drawAllModels(VkCommandBuffer& commandBuffer);
	void drawModel(ModelId modelId,
		const math::vec3& position,
		const math::quat& rotation,
		const math::vec3& scale,
		VkCommandBuffer commandBuffer);

	std::filesystem::path getModelPath(ModelId modelId);
}