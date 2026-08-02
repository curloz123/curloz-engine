/**
 * @file model.hpp
 * @curl0z
 * @brief Defines the data structures, classes, and functions
 * for loading, managing and rendering 3D models.
 */

#pragma once
#include "math/mat4x4.hpp"
#include "math/quat.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "renderer/entitydata/texture.hpp"
#include <expected>
#include <fastgltf/core.hpp>
#include <filesystem>
#include <vector>
#include <vulkan/vulkan.h>

namespace clz::renderer
{
/**
 * @struct PrimitiveData
 * @brief Holds the raw, CPU-side geometric and texture data for a single mesh primitive
 *        before it is uploaded to the GPU buffers.
 */
struct PrimitiveData
{
	std::vector<math::vec3> vertices; /**< @brief Vertex positions. */
	std::vector<math::vec2> uvs;	  /**< @brief Texture coordinates (UVs). */
	std::vector<uint32_t> indices;	  /**< @brief Index buffer data. */

	/**
	 * @brief Optional source for the base color texture (either a file URI or an
	 * embedded buffer view).
	 */
	std::optional<std::variant<fastgltf::sources::URI, fastgltf::sources::BufferView>>
		baseTexture;

	/** @brief The index of the base color texture in the glTF asset. */
	std::optional<size_t> baseColorTextureIndex;
};

/**
 * @struct Primitive
 * @brief Represents a single drawable primitive with GPU-side buffer offsets and texture
 * bindings.
 */
struct Primitive
{
	uint32_t indexCount;	  /**< @brief Number of indices to draw. */
	uint32_t firstIndex;	  /**< @brief Offset to the first index in the global index
				     buffer. */
	uint32_t baseVertexIndex; /**< @brief Offset to the first vertex in the global vertex
				     buffer. */
	std::optional<TextureID>
		baseTextureId; /**< @brief ID of the bound base color texture, if any. */
};

/**
 * @struct Mesh
 * @brief A collection of primitives that make up a single mesh.
 */
struct Mesh
{
	std::vector<Primitive>
		primitives; /**< @brief List of primitives contained in this mesh. */
};

/**
 * @struct Node
 * @brief Represents a node in the glTF scene graph hierarchy.
 */
struct Node
{
	std::optional<uint32_t> meshIndex; /**< @brief Index of the mesh associated with this
					      node, if any. */
	math::mat4 localTransform;  /**< @brief Local transformation matrix relative to its
				       parent. */
	std::vector<Node> children; /**< @brief Child nodes in the scene hierarchy. */
};

/**
 * @struct ModelPath
 * @brief A simple wrapper around a filesystem path for a model file.
 */
struct ModelPath
{
	std::filesystem::path path; /**< @brief The file path to the model. */

	ModelPath() = default;

	/**
	 * @brief Constructs a ModelPath from a given filesystem path.
	 * @param modelPath The path to the model file.
	 */
	ModelPath(const std::filesystem::path& modelPath) : path(modelPath)
	{
	}
};

/** @brief Unique identifier for a loaded model. */
using ModelId = uint32_t;

/**
 * @struct Model
 * @brief The core model class containing the scene graph (nodes) and mesh data.
 *        Handles both the loading logic from glTF and the Vulkan rendering logic.
 */
struct Model
{
	std::vector<std::optional<Mesh>>
		meshes; /**< @brief List of meshes (optional to handle sparse indices). */
	std::vector<Node> nodes; /**< @brief Root nodes of the scene graph. */

	/**
	 * @brief Draws the entire model starting from the root nodes.
	 * @param modelMatrix The global model transformation matrix.
	 * @param commandBuffer The Vulkan command buffer to record commands into.
	 */
	void draw(const math::mat4& modelMatrix, VkCommandBuffer commandBuffer);

	/**
	 * @brief Recursively draws a specific node and its children.
	 * @tparam T The type of the node (usually Node).
	 * @param node The current node to draw.
	 * @param parentTransform The accumulated transformation matrix from parent nodes.
	 * @param commandBuffer The Vulkan command buffer to record commands into.
	 */
	void drawNode(
		const auto& node,
		const math::mat4& parentTransform,
		VkCommandBuffer commandBuffer
	);

	/**
	 * @brief Loads a glTF node and recursively loads its children.
	 * @param node The fastgltf node to parse.
	 * @param asset The parent fastgltf asset.
	 * @param registeredId The ID of the model being loaded.
	 * @return The parsed Node structure.
	 */
	Node loadNode(
		const fastgltf::Node& node,
		const fastgltf::Asset& asset,
		ModelId registeredId
	);

	/**
	 * @brief Loads a glTF mesh and its primitives.
	 * @param mesh The fastgltf mesh to parse.
	 * @param asset The parent fastgltf asset.
	 * @param registeredId The ID of the model being loaded.
	 * @return The parsed Mesh structure.
	 */
	Mesh loadMesh(
		const fastgltf::Mesh& mesh,
		const fastgltf::Asset& asset,
		ModelId registeredId
	);

	/**
	 * @brief Registers primitive data to global GPU buffers and handles texture
	 * loading/caching.
	 * @param primitiveData The raw CPU-side primitive data.
	 * @param asset The parent fastgltf asset.
	 * @param registeredId The ID of the model being loaded.
	 * @return The GPU-ready Primitive structure.
	 */
	Primitive registerPrimitive(
		const PrimitiveData& primitiveData,
		const fastgltf::Asset& asset,
		ModelId registeredId
	);
};

/** @brief Constant representing an invalid or null model ID. */
constexpr uint32_t NULL_MODEL = std::numeric_limits<ModelId>::max();

/** @brief Global Look-Up Table (LUT) for all loaded models. */
inline std::vector<Model> ModelLUT;

/** @brief Global Look-Up Table for the file paths of all loaded models. */
inline std::vector<ModelPath> ModelPaths;

/**
 * @struct TextureCache
 * @brief Caches loaded textures for a specific model to prevent redundant loading of shared
 * textures.
 */
struct TextureCache
{
	std::unordered_map<size_t, TextureID>
		texturesLoaded; /**< @brief Map of glTF texture index to internal TextureID.
				 */

	/** @brief Clears the texture cache. */
	void clear()
	{
		texturesLoaded.clear();
	}
};

/** @brief Global vector of texture caches, indexed by ModelId. */
inline std::vector<TextureCache> TextureCaches;
} // namespace clz::renderer

namespace clz::renderer
{

/**
 * @brief Loads a 3D model from a glTF file path.
 * @param filePath The path to the glTF file.
 * @return The unique ModelId of the loaded model, or an unexpected error string on failure.
 */
std::expected<ModelId, std::string> loadModel(const std::filesystem::path& filePath);

/**
 * @brief Iterates through all ECS entities with a ModelComponent and draws them.
 * @param commandBuffer The Vulkan command buffer to record draw commands into.
 */
void drawAllModels(VkCommandBuffer commandBuffer);

/**
 * @brief Draws a specific model at a given transform.
 * @param modelId The ID of the model to draw.
 * @param position The world position.
 * @param rotation The world rotation (quaternion).
 * @param scale The world scale.
 * @param commandBuffer The Vulkan command buffer to record draw commands into.
 */
void drawModel(
	ModelId modelId,
	const math::vec3& position,
	const math::quat& rotation,
	const math::vec3& scale,
	VkCommandBuffer commandBuffer
);

/**
 * @brief Retrieves the original file path of a loaded model.
 * @param modelId The ID of the model.
 * @return The filesystem path to the model file.
 */
std::filesystem::path getModelPath(ModelId modelId);
} // namespace clz::renderer