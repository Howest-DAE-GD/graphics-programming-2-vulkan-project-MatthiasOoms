#pragma once
#include "Structs.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <string>

#include "tiny_gltf.h"

class Model
{
public:
	Model() = default;
	~Model() = default;
	std::vector<Vertex>& GetVertices() { return m_Vertices; }
	std::vector<uint32_t>& GetIndices() { return m_Indices; }
	std::vector<std::string>& GetDiffuseTextures() { return m_DiffuseTextures; }
    uint32_t GetFirstIndex() const { return m_FirstIndex; }
    uint32_t GetVertexOffset() const { return m_VertexOffset; }

    void SetFirstIndex(uint32_t index) { m_FirstIndex = index; }
    void SetVertexOffset(uint32_t offset) { m_VertexOffset = offset; }

private:
    std::vector<Vertex> m_Vertices;
    std::vector<uint32_t> m_Indices;
    std::vector<std::string> m_DiffuseTextures;

    uint32_t m_FirstIndex = 0;
    uint32_t m_VertexOffset = 0;
};