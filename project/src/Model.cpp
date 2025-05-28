#include "Model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <unordered_map>
#include <stdexcept>

Model::Model(const std::string& modelPath)
{
    std::string extension = modelPath.substr(modelPath.find_last_of('.') + 1);

    if (extension == "obj")
    {
        LoadModelObj(modelPath);
    }
    else if (extension == "gltf" || extension == "glb")
    {
        LoadModelGltf(modelPath);
    }
    else
    {
        throw std::runtime_error("Unsupported file format: " + extension);
    }
}

Model::~Model()
{
}

void Model::LoadModelObj(const std::string& modelPath)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str()))
    {
        throw std::runtime_error(err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = { 1.0f, 1.0f, 1.0f };

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(m_Vertices.size());
                m_Vertices.push_back(vertex);
            }

            m_Indices.push_back(uniqueVertices[vertex]);
        }
    }
}

void Model::LoadModelGltf(const std::string& modelPath)
{
    tinygltf::TinyGLTF loader{};
    tinygltf::Model gltfModel{};
    std::string error{};
    std::string warn{};

    // Try to read file
    bool result = loader.LoadASCIIFromFile(&gltfModel, &error, &warn, modelPath);

    if (!warn.empty())
    {
        throw std::runtime_error("gltf Warning: " + warn);
    }

    if (!error.empty())
    {
        throw std::runtime_error("gltf Error: " + error);
    }

    if (!result)
    {
        throw std::runtime_error("Unable to load model");
    }

    // Go over meshes
    for (auto& currentMesh : gltfModel.meshes)
    {
        // Go over primitives
        for (auto& primitive : currentMesh.primitives)
        {
            // Extract vertices, indices and texture names
            FillVertices(gltfModel, primitive);
            FillIndices(gltfModel, primitive);
            FillDiffuseTextures(gltfModel, primitive, GetFolderPath(modelPath));
        }
    }
}

void Model::FillVertices(const tinygltf::Model& gltfModel, const tinygltf::Primitive& primitive)
{
    // Find attributes in the primitive
    auto posIt = primitive.attributes.find("POSITION");
    auto colIt = primitive.attributes.find("COLOR_0");
    auto texIt = primitive.attributes.find("TEXCOORD_0");

    const float* posData = nullptr, * normData = nullptr, * texData = nullptr, * colData = nullptr;
    size_t vertexCount = 0;

    // Extract POSITION data
    if (posIt != primitive.attributes.end())
    {
        const tinygltf::Accessor& accessor = gltfModel.accessors[posIt->second];
        const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];

        posData = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
        vertexCount = accessor.count;
    }

    // Extract COLOR data
    if (colIt != primitive.attributes.end())
    {
        const tinygltf::Accessor& accessor = gltfModel.accessors[colIt->second];
        const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];

        colData = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
    }

    // Extract TEXCOORD_0 data
    if (texIt != primitive.attributes.end())
    {
        const tinygltf::Accessor& accessor = gltfModel.accessors[texIt->second];
        const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];

        texData = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
    }

    // Fill Vertex Data
    for (size_t i = 0; i < vertexCount; i++)
    {
        Vertex v = {};

        if (posData) v.pos = glm::vec3(posData[i * 3 + 0], posData[i * 3 + 1], posData[i * 3 + 2]);
        if (colData) v.color = glm::vec4(colData[i * 4 + 0], colData[i * 4 + 1], colData[i * 4 + 2], colData[i * 4 + 3]);
        if (texData) v.texCoord = glm::vec2(texData[i * 2 + 0], texData[i * 2 + 1]);

        m_Vertices.push_back(v);
    }
}

void Model::FillIndices(const tinygltf::Model& gltfModel, const tinygltf::Primitive& primitive)
{
    // If there is no indices
    if (primitive.indices < 0) 
    {
        return;
    }

    const tinygltf::Accessor& accessor = gltfModel.accessors[primitive.indices];
    const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
    const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];

    const void* dataPtr = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
    size_t count = accessor.count;

    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
    {
        const uint16_t* data = static_cast<const uint16_t*>(dataPtr);
        for (size_t i = 0; i < count; i++)
        {
            // Convert uint16_t to uint32_t
            m_Indices.push_back(static_cast<uint32_t>(data[i]));
        }
    }
    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
    {
        const uint32_t* data = static_cast<const uint32_t*>(dataPtr);
        for (size_t i = 0; i < count; i++)
        {
            // No conversion needed for uint32_t to uint32_t
            m_Indices.push_back(data[i]);
        }
    }
    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
    {
        const uint8_t* data = static_cast<const uint8_t*>(dataPtr);
        for (size_t i = 0; i < count; i++)
        {
            // Convert uint8_t to uint32_t
            m_Indices.push_back(static_cast<uint32_t>(data[i]));
        }
    }
}

void Model::FillDiffuseTextures(const tinygltf::Model& model, const tinygltf::Primitive& primitive, const std::string&& path)
{
    const int& materialIndex = primitive.material;

    if (materialIndex < 0) 
    {
        return;
    }

    auto& mat = model.materials[materialIndex];
    int texIdx = mat.pbrMetallicRoughness.baseColorTexture.index;

    if (texIdx < 0)
    {
        return;
    }

    const tinygltf::Texture& text = model.textures[texIdx];
    const tinygltf::Image& img = model.images[text.source];

    m_DiffuseTextures.push_back(path + img.uri);
}

std::string Model::GetFolderPath(const std::string& filename)
{
    auto index = filename.find_last_of("/");

    if (index >= filename.length() - 1)
    {
        return "";
    }

    return filename.substr(0, index + 1);
}