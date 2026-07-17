/**
 * @file modeldata.cpp
 * @brief Assimp-based model loading: scene traversal, mesh registration, and
 * base-color material/texture resolution.
 */

#include "renderer/entitydata/modeldata.hpp"
#include "core/assert.hpp"
#include "core/logs.hpp"
#include "math/vec2.hpp"
#include "renderer/entitydata/indexbuffer.hpp"
#include "renderer/entitydata/texture.hpp"
#include "renderer/entitydata/uvbuffer.hpp"
#include "renderer/entitydata/vertexbuffer.hpp"
#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace clz::renderer::Asset
{
	/**
	 * @brief Recursively walks an Assimp node tree, registering every mesh it finds.
	 * @param node Current Assimp node being visited.
	 * @param parentTransform Current node's parent's transform
	 * @param scene Assimp scene the node belongs to.
	 * @param modelID ID of the model currently being loaded.
	 * @param modelPath Path of the model file, used for log/diagnostic messages.
	 */
	void processNode(const aiNode* node, const aiMatrix4x4& parentTransform, const aiScene* scene, const ModelID modelID,
			 const std::string& modelPath);

	/**
	 * @brief Registers a single Assimp mesh's vertices, indices, UVs, and material.
	 * @param mesh Assimp mesh to process.
	 * @param localTransform Current node's local transform, applied to all meshes inside the node
	 * @param node Owning Assimp node, used for log/diagnostic messages.
	 * @param scene Assimp scene the mesh belongs to.
	 * @param modelPath Path of the model file, used for log/diagnostic messages.
	 * @return The ModelMeshID of the newly registered mesh.
	 */
	ModelMeshID processMesh(const aiMesh* mesh, const aiMatrix4x4& localTransform, const aiNode* node, const aiScene* scene,
				const std::string& modelPath);

	/**
	 * @brief Resolves and registers the first texture of a given type on a material.
	 * @param mat Assimp material to query.
	 * @param type Assimp texture type to look up (e.g. aiTextureType_BASE_COLOR).
	 * @param node Owning Assimp node, used for log/diagnostic messages.
	 * @param modelPath Path of the model file, used for log/diagnostic messages.
	 * @return The TextureID of the resolved texture, or r_NULL_TEXTURE if none was found.
	 */
	TextureID processBaseMaterial(const aiMaterial* mat, aiTextureType type, const aiNode* node, const std::string& modelPath);

} // namespace clz::renderer::Asset
namespace clz::renderer::Asset
{
	ModelID loadModel(const std::string& modelPath)
	{
		const ModelID modelID = r_numModelCount;

		r_modelLUT.resize(r_numModelCount + 1);
		modelName.resize(r_numModelCount + 1);
		modelName[modelID] = modelPath;
		++r_numModelCount;

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(modelPath.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			clz::log::error("Assimp error: " + std::string(importer.GetErrorString()));
			clz::log::error("Could not load file" + modelPath);
			return NULL_ASSET;
		}

		Asset::processNode(scene->mRootNode, aiMatrix4x4(), scene, modelID, modelPath);

		Asset::textureLoadedThisModel.clear();
		return modelID;
	}

	std::string getModelName(const ModelID id)
	{
		return modelName[id];
	}

	// Process Nodes
	/**
	 * @brief Recursively walks an Assimp node tree, registering every mesh it finds.
	 *
	 * For each mesh referenced by the current node, processMesh() is called and
	 * the resulting ModelMeshID is appended to the model's mesh list. The node's
	 * children are then visited in the same way.
	 *
	 * @param node Current Assimp node being visited.
	 * @param parentTransform Current node's parent's transform
	 * @param scene Assimp scene the node belongs to.
	 * @param modelID ID of the model currently being loaded.
	 * @param modelPath Path of the model file, used for log/diagnostic messages.
	 */
	void processNode(const aiNode* node, const aiMatrix4x4& parentTransform, const aiScene* scene, const ModelID modelID,
			 const std::string& modelPath)
	{
		const aiMatrix4x4 localTransform = parentTransform * node->mTransformation;
		for (uint32_t i = 0; i < node->mNumMeshes; ++i)
		{
			const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			r_modelLUT[modelID].meshes.emplace_back(processMesh(mesh, localTransform, node, scene, modelPath));
		}

		for (uint32_t i = 0; i < node->mNumChildren; ++i)
		{
			processNode(node->mChildren[i], localTransform, scene, modelID, modelPath);
		}
	}

	// Mesh Processor
	ModelMeshID processMesh(const aiMesh* mesh, const aiMatrix4x4& localTransform, const aiNode* node, const aiScene* scene,
				const std::string& modelPath)
	{
		uint32_t indexCount = NULL_MESH;
		uint32_t firstIndex = NULL_MESH;
		uint32_t baseVertex = NULL_MESH;
		// ModelMaterialID texture = NULL_MATERIAL;

		TextureID baseTextureID = r_NULL_TEXTURE;

		// Part 1 - Register vertices, indices and UVs
		std::vector<clz::math::vec3> vertices;
		std::vector<clz::math::vec2> UVs;
		for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
		{
			const aiVector3D vertex = localTransform * mesh->mVertices[i];
			vertices.emplace_back(math::vec3(vertex.x, vertex.y, vertex.z));

			clz::math::vec2 vec;
			// Process UVs
			if (mesh->mTextureCoords[0])
			{
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
			}
			else
			{
				vec.x = 0.0f;
				vec.y = 0.0f;
				log::debug("Artist-san you forgot about uv coordinates of some vertex");
			}
			UVs.emplace_back(vec);
		}

		if (vertices.empty())
		{
			clz::log::warn("Model: " + modelPath + "\n" + "Node: " + node->mName.C_Str() + "\n" + "Has no vertices");
		}
		if (UVs.size() != vertices.size())
		{
			CLZ_ASSERT(UVs.size() == vertices.size(),
				   "Model: " + modelPath + "\n" + "Node: " + node->mName.C_Str() + "\n" + "has irregular uv's and vertex's");
		}

		// Process indices
		std::vector<uint32_t> indices;
		for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
		{
			const aiFace face = mesh->mFaces[i];
			for (uint32_t j = 0; j < face.mNumIndices; ++j)
			{
				indices.push_back(face.mIndices[j]);
			}
		}
		if (indices.empty())
		{
			log::warn("Model: " + modelPath + "\n" + "Node: " + node->mName.C_Str() + "\n" + "Has no indices");
		}

		// Register Only Vertex, UVs and indices for now
		if (!vertices.empty())
		{
			baseVertex = getVertexBaseIndex();
			registerVertices(vertices);
			registerUVs(UVs);
		}
		if (!indices.empty())
		{
			auto [first, count] = registerIndices(indices);
			firstIndex = first;
			indexCount = count;
		}

		// Part 2 - Register textures

		// Process Textures
		const aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
		baseTextureID = processBaseMaterial(mat, aiTextureType_BASE_COLOR, node, modelPath);
		if (baseTextureID == r_NULL_TEXTURE)
		{
			baseTextureID = processBaseMaterial(mat, aiTextureType_DIFFUSE, node, modelPath);
		}
		if (baseTextureID == r_NULL_TEXTURE)
		{
			log::warn("Model: " + modelPath + "\n" +
				  "has no base texture"
				  "will apply grey color");
		}

		mesh::baseMaterialLUT.texture.emplace_back(baseTextureID);

		mesh::meshLUT.indexCount.emplace_back(indexCount);
		mesh::meshLUT.firstIndex.emplace_back(firstIndex);
		mesh::meshLUT.baseVertex.emplace_back(baseVertex);
		mesh::meshLUT.material.emplace_back(mesh::baseMaterialLUT.texture.size() - 1);

		return mesh::meshLUT.indexCount.size() - 1;
	}

	TextureID processBaseMaterial(const aiMaterial* mat, const aiTextureType type, const aiNode* node, const std::string& modelPath)
	{
		const std::string parentDir = modelPath.substr(0, modelPath.find_last_of('/'));
		for (uint32_t i = 0; i < mat->GetTextureCount(type); ++i)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			const std::filesystem::path filePath(parentDir + "/" + str.C_Str());
			if (Asset::textureLoadedThisModel.contains(filePath.string()))
			{
				return Asset::textureLoadedThisModel[filePath.string()];
			}

			const TextureID textureID = registerTexture(filePath.string());
			Asset::textureLoadedThisModel[filePath.string()] = textureID;
			return textureID;
		}

		clz::log::warn("Model: " + modelPath + " has no base texture, retrying...");
		return r_NULL_TEXTURE;
	}

} // namespace clz::renderer::Asset
