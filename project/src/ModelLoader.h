#pragma once
#include "Structs.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <string>

#include "Model.h"

#include "tiny_gltf.h"

class ModelLoader
{
public:
	ModelLoader();
	~ModelLoader();
	std::vector<Model*> LoadModel(const std::string& modelPath);
	std::vector<Model*> LoadModelObj(const std::string& modelPath);
	std::vector<Model*> LoadModelGltf(const std::string& modelPath);

private:
	void FillVertices(const tinygltf::Model& gltfModel, const tinygltf::Primitive& primitive, std::vector<Vertex>& vertices, const glm::mat4& transform);
	void FillIndices(const tinygltf::Model& gltfModel, const tinygltf::Primitive& primitive, std::vector<uint32_t>& indices);
	void FillDiffuseTextures(const tinygltf::Model& model, const tinygltf::Primitive& primitive, const std::string&& path, std::vector<std::string>& diffuseTextures);

	void ProcessNode(const tinygltf::Model& model, int nodeIdx, const glm::mat4& parentTransform, std::vector<Model*>& models, const std::string& modelPath);

	std::string GetFolderPath(const std::string& filename);
};