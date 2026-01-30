#include "assimp_layer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "stb_image.h"

std::map<std::string, s_Texture> g_TextureResources;

Mesh::Mesh(std::vector<s_Vertex> m_Vertices, std::vector<u32> m_Indices, std::vector<s_Texture> m_Textures)
{
	this->m_Vertices = m_Vertices;
	this->m_Indices = m_Indices;
	this->m_Textures = m_Textures;
	m_SetUpMesh();
}


void Mesh::m_SetUpMesh() 
{

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);

	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(m_Vertices[0]), &m_Vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(m_Indices[0]), &m_Indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(s_Vertex), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(s_Vertex), (void*)offsetof(s_Vertex, Normal));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(s_Vertex), (void*)offsetof(s_Vertex, TexCoords));

	glBindVertexArray(0);
}

void Mesh::Draw(Shader& shader)
{
	u32 NumDiffuse = 1;
	u32 NumSpecular = 1;

	for (u32 i = 0; i < m_Textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		std::string number;
		std::string name = m_Textures[i].Type;
		if (name == "texture_diffuse")
			number = std::to_string(NumDiffuse++);
		else if (name == "texture_specular")
			number = std::to_string(NumSpecular++);
		shader.SetFloat(("material." + name + number).c_str(), i);
		glBindTexture(GL_TEXTURE_2D, m_Textures[i].Id);
	}
	glActiveTexture(GL_TEXTURE0);
	
	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

}

void Model::Draw(Shader& shader)
{
	for (u32 i = 0; i < m_Meshes.size(); i++)
		m_Meshes[i].Draw(shader);
}
void Model::LoadModel(std::string path)
{
	Assimp::Importer Imp;
	const aiScene* Scene = Imp.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
	if (!Scene || Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !Scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << Imp.GetErrorString() << std::endl;
		return;
	}
	m_Directory = path.substr(0, path.find_last_of('/'));
	ProcessNode(Scene->mRootNode, Scene);
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
	for (u32 i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* Mesh = scene->mMeshes[node->mMeshes[i]];
		m_Meshes.push_back(ProcessMesh(Mesh, scene));
	}

	for (u32 i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<s_Vertex> Vertices;
	std::vector<u32> Indices;
	std::vector<s_Texture> Textures;

	for (u32 i = 0; i < mesh->mNumVertices; i++)
	{
		s_Vertex Vertex = {};
		glm::vec3 V3;
		V3.x = mesh->mVertices[i].x;
		V3.y = mesh->mVertices[i].y;
		V3.z = mesh->mVertices[i].z;
		Vertex.Position = V3;

		V3.x = mesh->mNormals[i].x;
		V3.y = mesh->mNormals[i].y;
		V3.z = mesh->mNormals[i].z;
		Vertex.Normal = V3;
		
		// Assimp allows to define 8 differente texture coordinates per vertex, we will use just first one.
		// Check if mesh got UVCoordinates 
		if (mesh->mTextureCoords[0])
		{
			glm::vec2 V2;
			V2.x = mesh->mTextureCoords[0][i].x;
			V2.y = mesh->mTextureCoords[0][i].y;
			Vertex.TexCoords = V2;
		}
		else
		{
			Vertex.TexCoords = { 0, 0 };
		}

		Vertices.push_back(Vertex);
	}

	for (u32 i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace Face = mesh->mFaces[i];
		for (u32 j = 0; j < Face.mNumIndices; j++)
		{
			Indices.push_back(Face.mIndices[j]);
		}
	}

	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* Material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<s_Texture> DiffuseMaps = LoadMaterialTextures(Material, aiTextureType_DIFFUSE, "texture_diffuse");
		Textures.insert(Textures.end(), DiffuseMaps.begin(), DiffuseMaps.end());

		std::vector<s_Texture> SpecularMaps = LoadMaterialTextures(Material, aiTextureType_SPECULAR, "texture_specular");
		Textures.insert(Textures.end(), SpecularMaps.begin(), SpecularMaps.end());
	}	

	return Mesh(Vertices, Indices, Textures);
}

std::vector<s_Texture> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string type_name)
{
	std::vector<s_Texture> Textures;


	for (u32 i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString Str;
		mat->GetTexture(type, i, &Str);
			
		
		auto it = g_TextureResources.find(std::string(Str.C_Str()));

		//s_Texture Texture = g_TextureResources.find(std::string(Str.C_Str()));

		s_Texture Texture;
		if (it == g_TextureResources.end())
		{
			Texture.Id = TextureFromFile(Str.C_Str());
			Texture.Type = type_name;
			Texture.Path = std::string(Str.C_Str());
			Textures.push_back(Texture);
			g_TextureResources[Str.C_Str()] = Texture;
			Textures.push_back(Texture);
		}
		else
		{
			Textures.push_back(it->second);
		}

	}
	return Textures;
}

u32 TextureFromFile(const std::string file_path)
{
	u32 TextureID;
	
	glGenTextures(0, &TextureID);
	glBindTexture(GL_TEXTURE_2D, TextureID);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);

	int Width = 0;
	int Height = 0;
	int NumChannels = 0;

	GLenum ImageFormat;

	unsigned char* data = stbi_load(file_path.c_str(), &Width, &Height, &NumChannels, 0);
	switch (NumChannels)
	{
	case 1: {
		ImageFormat = GL_RED;
	} break;
	case 3: {
		ImageFormat = GL_RGB;
	} break;
	case 4: {
		ImageFormat = GL_RGBA;
	} break;
	default: {
		throw std::runtime_error("IMAGE FORMAT NOT SUPOORTED");
	}
	}

	if (data)
	{
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA,
			Width, Height, 0, ImageFormat,
			GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		std::string str(file_path);
	}
	else
	{
		throw std::runtime_error("Not able to read image");
	}
	stbi_image_free(data);

	return TextureID;
}
