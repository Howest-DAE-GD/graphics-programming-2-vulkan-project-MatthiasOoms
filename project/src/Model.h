#pragma once
#include "Structs.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <string>

class Model
{
public:
	Model(const std::string& modelPath);
	~Model();
	void LoadModelObj();
	void LoadModelGltf();
	const std::vector<Vertex> GetVertices() const { return m_Vertices; }
	const std::vector<uint32_t> GetIndices() const { return m_Indices; }

private:
	std::string m_ModelPath;
	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
};