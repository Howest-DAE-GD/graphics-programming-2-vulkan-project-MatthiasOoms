#include "ModelLoader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <glm/gtx/quaternion.hpp>
#include <unordered_map>
#include <stdexcept>

ModelLoader::ModelLoader()
{}

ModelLoader::~ModelLoader()
{
}

std::vector<Model*> ModelLoader::LoadModel(const std::string& modelPath)
{
    std::string extension = modelPath.substr(modelPath.find_last_of('.') + 1);

    if (extension == "obj")
    {
        return LoadModelObj(modelPath);
    }
    else if (extension == "gltf")
    {
        return LoadModelGltf(modelPath);
    }
    else
    {
        throw std::runtime_error("Unsupported file format: " + extension);
    }

	return std::vector<Model*>{};
}

std::vector<Model*> ModelLoader::LoadModelObj(const std::string& modelPath)
{
	std::vector<Model*> models;

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
		// Create a new model
		models.push_back(new Model{});
		Model& model = *models.back();

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
                uniqueVertices[vertex] = static_cast<uint32_t>(model.GetVertices().size());
                model.GetVertices().push_back(vertex);
            }

            model.GetIndices().push_back(uniqueVertices[vertex]);
        }
    }

	return models;
}

std::vector<Model*> ModelLoader::LoadModelGltf(const std::string& modelPath)
{
    std::vector<Model*> models;

    tinygltf::TinyGLTF loader{};
    tinygltf::Model gltfModel{};
    std::string error{}, warn{};

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

    const tinygltf::Scene& scene = gltfModel.scenes[gltfModel.defaultScene];

    for (int nodeIndex : scene.nodes)
    {
        ProcessNode(gltfModel, nodeIndex, glm::mat4(1.0f), models, modelPath);
    }

    return models;
}

void ModelLoader::FillVertices(const tinygltf::Model& gltfModel, const tinygltf::Primitive& primitive, std::vector<Vertex>& vertices, const glm::mat4& transform)
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

        if (posData) 
        {
            glm::vec4 pos = glm::vec4(posData[i * 3 + 0], posData[i * 3 + 1], posData[i * 3 + 2], 1.0f);
            v.pos = glm::vec3(transform * pos);
        }
        if (colData) 
        {
            v.color = glm::vec4(colData[i * 4 + 0], colData[i * 4 + 1], colData[i * 4 + 2], colData[i * 4 + 3]);
        }
        if (texData) 
        {
            v.texCoord = glm::vec2(texData[i * 2 + 0], texData[i * 2 + 1]);
        }

        vertices.push_back(v);
    }
}

void ModelLoader::FillIndices(const tinygltf::Model& gltfModel, const tinygltf::Primitive& primitive, std::vector<uint32_t>& indices)
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
            indices.push_back(static_cast<uint32_t>(data[i]));
        }
    }
    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
    {
        const uint32_t* data = static_cast<const uint32_t*>(dataPtr);
        for (size_t i = 0; i < count; i++)
        {
            // No conversion needed for uint32_t to uint32_t
            indices.push_back(data[i]);
        }
    }
    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
    {
        const uint8_t* data = static_cast<const uint8_t*>(dataPtr);
        for (size_t i = 0; i < count; i++)
        {
            // Convert uint8_t to uint32_t
            indices.push_back(static_cast<uint32_t>(data[i]));
        }
	}
    else
    {
        throw std::runtime_error("Unsupported index component type: " + std::to_string(accessor.componentType));
    }
}

void ModelLoader::FillDiffuseTexture(const tinygltf::Model& model, const tinygltf::Primitive& primitive, const std::string&& path, std::string& diffuseTexture)
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

    diffuseTexture = path + img.uri;
}

void ModelLoader::SetTransparent(Model& modelObj, const tinygltf::Model& model, const tinygltf::Primitive& primitive)
{
    const int& materialIndex = primitive.material;
    auto& mat = model.materials[materialIndex];
    int texIdx = mat.pbrMetallicRoughness.baseColorTexture.index;

    if (mat.alphaMode == "MASK")
    {
		modelObj.SetTransparent(true);
    }
}

void ModelLoader::ProcessNode(const tinygltf::Model& model, int nodeIndex, const glm::mat4& parentTransform, std::vector<Model*>& models, const std::string& modelPath)
{
    const tinygltf::Node& node = model.nodes[nodeIndex];

    glm::mat4 localTransform = glm::mat4(1.0f);

    // Apply translation
    if (node.translation.size() == 3)
    {
        localTransform = glm::translate(localTransform, glm::vec3(
            node.translation[0],
            node.translation[1],
            node.translation[2]
        ));
    }

    // Apply rotation (as quaternion)
    if (node.rotation.size() == 4)
    {
        glm::quat q(
            node.rotation[3], // w
            node.rotation[0], // x
            node.rotation[1], // y
            node.rotation[2]  // z
        );
        localTransform *= glm::toMat4(q);
    }

    // Apply scale
    if (node.scale.size() == 3)
    {
        localTransform = glm::scale(localTransform, glm::vec3(
            node.scale[0],
            node.scale[1],
            node.scale[2]
        ));
    }

    glm::mat4 globalTransform = parentTransform * localTransform;

    if (node.mesh >= 0)
    {
        for (const auto& mesh : model.meshes)
        {
            for (auto& primitive : model.meshes[node.mesh].primitives)
            {
                // Create model and load primitives with transform
                models.push_back(new Model{});
                Model& modelObj = *models.back();

                FillVertices(model, primitive, modelObj.GetVertices(), globalTransform);
                FillIndices(model, primitive, modelObj.GetIndices());
                FillDiffuseTexture(model, primitive, GetFolderPath(modelPath), modelObj.GetDiffuseTexture());
                SetTransparent(modelObj, model, primitive);
            }
        }
    }

    // Recursively process children
    for (int childIndex : node.children)
    {
        ProcessNode(model, childIndex, globalTransform, models, modelPath);
    }
}

std::string ModelLoader::GetFolderPath(const std::string& filename)
{
    auto index = filename.find_last_of("/");

    if (index >= filename.length() - 1)
    {
        return "";
    }

    return filename.substr(0, index + 1);
}