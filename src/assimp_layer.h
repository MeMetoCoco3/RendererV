#indef ASSIMP_VLAYER
#define ASSIMP_VLAYER

#include "vstd/vgeneral.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec3 TexCoords;
};

struct Texture {
	u32 id;
	std::string type;
};

class Mesh {
public: 
	vector<Vertex> vertices;
	vector<u32> indices;
	vector<Texture> textures;

	Mesh(vector<Vertex> vertices, vector<u32> indices, vector<Texture> textures);
	void Draw(Shader& shader);
private:
	u32 m_VAO, m_VBO, m_EBO;
	void m_SetUpMesh();
};


#endif