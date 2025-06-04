#pragma once
#include "Structs.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <string>

#include "Texture.h"
#include "DescriptorSets.h"

#include "tiny_gltf.h"

class Model
{
public:
	Model() = default;
    ~Model()
    {
		delete m_pTexture;
		m_pTexture = nullptr;

		delete m_pDescriptorSets;
		m_pDescriptorSets = nullptr;
    };

	std::vector<Vertex>& GetVertices() { return m_Vertices; }
	std::vector<uint32_t>& GetIndices() { return m_Indices; }
	std::string& GetDiffuseTexture() { return m_DiffuseTexture; }
    uint32_t GetFirstIndex() const { return m_FirstIndex; }
    uint32_t GetVertexOffset() const { return m_VertexOffset; }
	Texture* GetTexture() { return m_pTexture; }
	DescriptorSets* GetDescriptorSets() { return m_pDescriptorSets; }
	bool IsTransparent() const { return m_IsTransparent; }

	void SetDescriptorSets(DescriptorSets* descriptorSets) { m_pDescriptorSets = descriptorSets; }
	void SetTexture(Texture* texture) { m_pTexture = texture; }
    void SetFirstIndex(uint32_t index) { m_FirstIndex = index; }
    void SetVertexOffset(uint32_t offset) { m_VertexOffset = offset; }
	void SetTransparent(bool isTransparent) { m_IsTransparent = isTransparent; }

private:
    std::vector<Vertex> m_Vertices;
    std::vector<uint32_t> m_Indices;
    std::string m_DiffuseTexture;

	Texture* m_pTexture = nullptr;
    DescriptorSets* m_pDescriptorSets = nullptr;

    uint32_t m_FirstIndex = 0;
    uint32_t m_VertexOffset = 0;

	bool m_IsTransparent = false;
};