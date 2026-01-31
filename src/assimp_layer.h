#ifndef ASSIMP_VLAYER
#define ASSIMP_VLAYER

#include "vstd/vgeneral.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "shaders.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <map>


// MOVE FROM HERE
u32 TextureFromFile(const std::string file_path);

struct s_Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	s_Vertex() = default;
	~s_Vertex() = default;
};

struct s_Texture {
	u32 Id;
	std::string Type;
	// TODO: Probably will not need path.
	std::string Path;
};

extern std::map<std::string, s_Texture> g_TextureResources;



class Mesh {
public: 
	std::vector<s_Vertex> m_Vertices;
	std::vector<u32> m_Indices;
	std::vector<s_Texture> m_Textures;

	Mesh(std::vector<s_Vertex> m_Vertices, std::vector<u32> m_Indices, std::vector<s_Texture> m_Textures);
	void Draw(Shader& shader);
private:
	u32 m_VAO, m_VBO, m_EBO;
	void m_SetUpMesh();
};

class Model 
{
public:

	Model(const std::string &path)
	{
		LoadModel(path);
	}
	void Draw(Shader& shader);
private:
	std::vector<Mesh> m_Meshes;
	std::string m_Directory;

	void LoadModel(std::string path);
	void ProcessNode(aiNode* node, const aiScene* scene);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<s_Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string type_name);

};
#endif