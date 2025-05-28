#pragma once
#include "Structs.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <string>

#include "tiny_gltf.h"

class Model
{
public:
	Model(const std::string& modelPath);
	~Model();
	void LoadModelObj(const std::string& modelPath);
	void LoadModelGltf(const std::string& modelPath);
	const std::vector<Vertex> GetVertices() const { return m_Vertices; }
	const std::vector<uint32_t> GetIndices() const { return m_Indices; }

private:
	void FillVertices(const tinygltf::Model& gltfModel, const tinygltf::Primitive& primitive);
	void FillIndices(const tinygltf::Model& gltfModel, const tinygltf::Primitive& primitive);
	void FillDiffuseTextures(const tinygltf::Model& model, const tinygltf::Primitive& primitive, const std::string&& path);

	std::string GetFolderPath(const std::string& filename);

	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
	std::vector<std::string> m_DiffuseTextures;
};