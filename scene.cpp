#include "scene.h"
#include "global.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <assimp/Exporter.hpp>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

//std::string MODEL_PATH = "E:/games202/homework3/assets/cube/cube2.gltf";
std::string MODEL_PATH = "E:/games202/homework3/assets/cave/cave.gltf";

//const std::string PRECOMPUTED_LIGHT_PATH = "E:/games202/Assignment2/prt/scenes/cubemap/Indoor/light.txt";
//const std::string PRECOMPUTED_TRANSPORT_PATH = "E:/games202/Assignment2/prt/scenes/cubemap/Indoor/transport.txt";

const std::string PRECOMPUTED_LIGHT_PATH = "E:/games202/Assignment2/prt/scenes/cubemap/CornellBox/light.txt";
const std::string PRECOMPUTED_TRANSPORT_PATH = "E:/games202/Assignment2/prt/scenes/cubemap/CornellBox/transport.txt";

//const std::string PRECOMPUTED_TRANSPORT_PATH = "E:/games202/Assignment2/prt/scenes/cubemap/Skybox/transport.txt";
//const std::string PRECOMPUTED_LIGHT_PATH = "E:/games202/Assignment2/prt/scenes/cubemap/Skybox/light.txt";

//const std::string PRECOMPUTED_TRANSPORT_PATH = "E:/games202/Assignment2/prt/scenes/cubemap/GraceCathedral/transport.txt";
//const std::string PRECOMPUTED_LIGHT_PATH = "E:/games202/Assignment2/prt/scenes/cubemap/GraceCathedral/light.txt";

VkVertexInputBindingDescription Vertex::getBindingDescription() {
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 3> Vertex::getAttributeDescriptions() {
	std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, normal);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

	return attributeDescriptions;
}

bool Vertex::operator==(const Vertex& other) const {
	return pos == other.pos && normal == other.normal && texCoord == other.texCoord;
}

void Scene::loadModel(Context* context) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
		MODEL_PATH.c_str())) {
		throw std::runtime_error(warn + err);
	}

	float ymin = FLT_MAX;

	Mesh mesh;
	std::unordered_map<Vertex, uint32_t> uniqueVertices{};
	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex{};
			vertex.pos = { attrib.vertices[3 * index.vertex_index + 0],
						  attrib.vertices[3 * index.vertex_index + 1],
						  attrib.vertices[3 * index.vertex_index + 2] };
			vertex.texCoord = { attrib.texcoords[2 * index.texcoord_index + 0],
							   1.0f - attrib.texcoords[2 * index.texcoord_index + 1] };
			vertex.normal = { attrib.normals[3 * index.normal_index  + 0],
							attrib.normals[3 * index.normal_index + 1],
							attrib.normals[3 * index.normal_index + 2] };

			if (vertex.pos.y < ymin) ymin = vertex.pos.y;

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(mesh.vertices.size());
				mesh.vertices.push_back(vertex);
			}
			mesh.indices.push_back(uniqueVertices[vertex]);
		}
	}
	this->meshes.push_back(mesh);

	/**********************************************************************/
#ifdef PCSS
	Vertex v[4];
	uint32_t index[4];
	float side = 27.924055;
	v[0].pos = vec3(side, ymin, side); 
	v[0].normal = vec3(0.0, 1.0, 0.0);
	v[1].pos = vec3(side, ymin, -side);
	v[1].normal = vec3(0.0, 1.0, 0.0);
	v[2].pos = vec3(-side, ymin, side);
	v[2].normal = vec3(0.0, 1.0, 0.0);
	v[3].pos = vec3(-side, ymin, -side);
	v[3].normal = vec3(0.0, 1.0, 0.0);
	for (int i = 0; i < 4; ++i) {
		index[i] = meshes[0].vertices.size();
		meshes[0].vertices.push_back(v[i]);
	}
	std::vector<uint32_t> temp = { index[0],index[1],index[2],index[2],index[1],index[3] };
	for (auto t : temp) meshes[0].indices.push_back(t);
#elif defined PRT
	Vertex v[8];
	uint32_t index[36] = { 0, 1, 2, 0, 2, 3,
						   4, 6, 5, 4, 7, 6,
						   1, 5, 6, 1, 6, 2,
						   0, 7, 4, 0, 3, 7,
						   3, 2, 6, 3, 6, 7,
						   0, 5, 1, 0, 4, 5};
	uint32_t indexFromScene[8];
	float side = 150.0f;
	v[0].pos = vec3(-side, -side, side);
	v[1].pos = vec3(side, -side, side);
	v[2].pos = vec3(side, side, side);
	v[3].pos = vec3(-side, side, side);
	v[4].pos = vec3(-side, -side, -side);
	v[5].pos = vec3(side, -side, -side);
	v[6].pos = vec3(side, side, -side);
	v[7].pos = vec3(-side, side, -side);
	for (int i = 0; i < 8; ++i) {
		indexFromScene[i] = meshes[0].vertices.size();
		meshes[0].vertices.push_back(v[i]);
	}
	std::vector<uint32_t> temp;
	for (uint32_t i : index) {
		temp.push_back(indexFromScene[i]);
	}
	for (auto t : temp) meshes[0].indices.push_back(t);
#endif
	/**********************************************************************/

	VkDeviceSize size = meshes[0].vertices.size() * sizeof(Vertex);
	vertexBuffer.create(context, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vertexBuffer.upload(meshes[0].vertices.data(), true);

	size = meshes[0].indices.size() * sizeof(uint32_t);
	indexBuffer.create(context, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	indexBuffer.upload(meshes[0].indices.data(), true);
}

void Scene::loadGLFT(Context* context) {
	// Create an instance of the Importer class
	Assimp::Importer importer;

	// And have it read the given file with some example postprocessing
	// Usually - if speed is not the most important aspect for you - you'll
	// probably to request more postprocessing than we do in this example.
	const aiScene* scene = importer.ReadFile(MODEL_PATH, aiProcess_JoinIdenticalVertices | aiProcess_PreTransformVertices);

	float ymin = FLT_MAX;

	meshes.resize(scene->mNumMeshes);
	unsigned numVerticesAll = 0;
	unsigned numIndicesAll = 0;
	for (unsigned i = 0; i < scene->mNumMeshes; ++i) {
		unsigned numVertices = scene->mMeshes[i]->mNumVertices;
		numVerticesAll += numVertices;
		for (unsigned j = 0; j < numVertices; ++j) {
			Vertex v;
			v.pos.x = scene->mMeshes[i]->mVertices[j].x;
			v.pos.y = scene->mMeshes[i]->mVertices[j].y;
			v.pos.z = scene->mMeshes[i]->mVertices[j].z;
			v.normal.x = scene->mMeshes[i]->mNormals[j].x;
			v.normal.y = scene->mMeshes[i]->mNormals[j].y;
			v.normal.z = scene->mMeshes[i]->mNormals[j].z;
			v.texCoord.x = scene->mMeshes[i]->mTextureCoords[0][j].x;
			v.texCoord.y = scene->mMeshes[i]->mTextureCoords[0][j].y;
			
			meshes[i].vertices.push_back(v);
			if (v.pos.y < ymin) ymin = v.pos.y;
		}

		unsigned numFaces = scene->mMeshes[i]->mNumFaces;
		numIndicesAll += numFaces * 3;
		for (unsigned j = 0; j < numFaces; ++j) {
			meshes[i].indices.push_back(scene->mMeshes[i]->mFaces[j].mIndices[0]);
			meshes[i].indices.push_back(scene->mMeshes[i]->mFaces[j].mIndices[1]);
			meshes[i].indices.push_back(scene->mMeshes[i]->mFaces[j].mIndices[2]);
		}
	}
	
	Buffer stagingBuffer;
	stagingBuffer.create(context, numVerticesAll * sizeof(Vertex), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	void* pData;
	VkDeviceSize offset = 0;
	for (int i = 0; i < meshes.size(); ++i) {
		VkDeviceSize size = meshes[i].vertices.size() * sizeof(Vertex);
		vkMapMemory(context->device, stagingBuffer.memory, offset, size, 0, &pData);
		memcpy(pData, meshes[i].vertices.data(), size);
		vkUnmapMemory(context->device, stagingBuffer.memory);
		meshes[i].vertexOffset = offset / sizeof(Vertex);
		offset += size;
	}
	vertexBuffer.create(context, offset, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	stagingBuffer.copyTo(&vertexBuffer, offset);
	stagingBuffer.destroy();

	stagingBuffer.create(context, numIndicesAll * sizeof(uint32_t), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	offset = 0;
	for (int i = 0; i < meshes.size(); ++i) {
		VkDeviceSize size = meshes[i].indices.size() * sizeof(uint32_t);
		vkMapMemory(context->device, stagingBuffer.memory, offset, size, 0, &pData);
		memcpy(pData, meshes[i].indices.data(), size);
		vkUnmapMemory(context->device, stagingBuffer.memory);
		meshes[i].indexOffset = offset / sizeof(uint32_t);
		offset += size;
	}
	indexBuffer.create(context, offset, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	stagingBuffer.copyTo(&indexBuffer, offset);
	stagingBuffer.destroy();
}

void Scene::cleanup() {
	meshes.resize(0);
	if (vertexBuffer.handle != VK_NULL_HANDLE) vertexBuffer.destroy();
	if (indexBuffer.handle != VK_NULL_HANDLE) indexBuffer.destroy();
}

void Scene::loadModelPRT(Context* context) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
		MODEL_PATH.c_str())) {
		throw std::runtime_error(warn + err);
	}

	float ymin = FLT_MAX;

	Mesh mesh;
	std::unordered_map<Vertex, uint32_t> uniqueVertices{};
	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex{};
			vertex.pos = { attrib.vertices[3 * index.vertex_index + 0],
						  attrib.vertices[3 * index.vertex_index + 1],
						  attrib.vertices[3 * index.vertex_index + 2] };
			vertex.texCoord = { attrib.texcoords[2 * index.texcoord_index + 0],
							   1.0f - attrib.texcoords[2 * index.texcoord_index + 1] };
			vertex.normal = { attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2] };

			if (vertex.pos.y < ymin) ymin = vertex.pos.y;

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(mesh.vertices.size());
				mesh.vertices.push_back(vertex);
				VertexPRT vertexPRT;
				vertexPRT.pos = vertex.pos;
				vertexPRT.texCoord = vertex.texCoord;
				vertexPRT.transport0 = vec3(0.0);
				vertexPRT.transport1 = vec3(0.0);
				vertexPRT.transport2 = vec3(0.0);
				mesh.verticesPRT.push_back(vertexPRT);
			}
			mesh.indices.push_back(uniqueVertices[vertex]);
		}
	}
	this->meshes.push_back(mesh);
	loadPrecomputedData(PRECOMPUTED_LIGHT_PATH, PRECOMPUTED_TRANSPORT_PATH);
	for (int i = 0; i < meshes[0].verticesPRT.size(); ++i) {
		meshes[0].verticesPRT[i].transport0 = vec3(transportCoef[i][0][0], transportCoef[i][0][1], transportCoef[i][0][2]);
		meshes[0].verticesPRT[i].transport1 = vec3(transportCoef[i][1][0], transportCoef[i][1][1], transportCoef[i][1][2]);
		meshes[0].verticesPRT[i].transport2 = vec3(transportCoef[i][2][0], transportCoef[i][2][1], transportCoef[i][2][2]);
	}
	/**********************************************************************/
	VertexPRT v[8];
	uint32_t index[36] = { 0, 1, 2, 0, 2, 3,
						   4, 6, 5, 4, 7, 6,
						   1, 5, 6, 1, 6, 2,
						   0, 7, 4, 0, 3, 7,
						   3, 2, 6, 3, 6, 7,
						   0, 5, 1, 0, 4, 5 };
	uint32_t indexFromScene[8];
	float side = 10.f;
	v[0].pos = vec3(-side, -side, side);
	v[1].pos = vec3(side, -side, side);
	v[2].pos = vec3(side, side, side);
	v[3].pos = vec3(-side, side, side);
	v[4].pos = vec3(-side, -side, -side);
	v[5].pos = vec3(side, -side, -side);
	v[6].pos = vec3(side, side, -side);
	v[7].pos = vec3(-side, side, -side);
	for (int i = 0; i < 8; ++i) {
		indexFromScene[i] = meshes[0].verticesPRT.size();
		meshes[0].verticesPRT.push_back(v[i]);
	}
	std::vector<uint32_t> temp;
	for (uint32_t i : index) {
		temp.push_back(indexFromScene[i]);
	}
	for (auto t : temp) meshes[0].indices.push_back(t);
	/**********************************************************************/


	VkDeviceSize size = meshes[0].verticesPRT.size() * sizeof(VertexPRT);
	vertexBuffer.create(context, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vertexBuffer.upload(meshes[0].verticesPRT.data(), true);

	size = meshes[0].indices.size() * sizeof(uint32_t);
	indexBuffer.create(context, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	indexBuffer.upload(meshes[0].indices.data(), true);
}

void Scene::loadPrecomputedData(std::string lightPath, std::string transportPath) {
	int numChannels = 3;

	lightCoef.resize(numChannels);
	std::fstream input(lightPath, std::ios::in);
	for (int i = 0; i < numChannels; ++i) {
		//input >> lightCoef[i][0][0] >> lightCoef[i][0][1] >> lightCoef[i][0][2]
		//	>> lightCoef[i][1][0] >> lightCoef[i][1][1] >> lightCoef[i][1][2]
		//	>> lightCoef[i][2][0] >> lightCoef[i][2][1] >> lightCoef[i][2][2];

		input >> lightCoef[i][0][0] >> lightCoef[i][1][0] >> lightCoef[i][2][0]
			>> lightCoef[i][0][1] >> lightCoef[i][1][1] >> lightCoef[i][2][1]
			>> lightCoef[i][0][2] >> lightCoef[i][1][2] >> lightCoef[i][2][2];
	}
	input.close();

	input.open(transportPath, std::ios::in);
	transportCoef.resize(meshes[0].verticesPRT.size());
	for (int i = 0; i < meshes[0].indices.size(); ++i) {
		uint32_t index = meshes[0].indices[i];
		//input >> transportCoef[index][0][0] >> transportCoef[index][0][1] >> transportCoef[index][0][2]
		//	>> transportCoef[index][1][0] >> transportCoef[index][1][1] >> transportCoef[index][1][2]
		//	>> transportCoef[index][2][0] >> transportCoef[index][2][1] >> transportCoef[index][2][2];

		input >> transportCoef[index][0][0] >> transportCoef[index][1][0] >> transportCoef[index][2][0]
			>> transportCoef[index][0][1] >> transportCoef[index][1][1] >> transportCoef[index][2][1]
			>> transportCoef[index][0][2] >> transportCoef[index][1][2] >> transportCoef[index][2][2];
	}
	input.close();
}
