#ifndef SHAPES_H
#define SHAPES_H

#include "vstd/vtypes.h"
#include "shaders.h"
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <unordered_map>

namespace S_FILES
{
	inline std::unordered_map<std::string, u32> Textures;
}

constexpr auto MAX_DEF_LEVEL = 1000;


struct v_Vertex {
    f32 px, py, pz;
    f32 u, v;
};

class Shape 
{
private:
	u32 m_LoadTexture(const std::string file_path);
protected: 
	u32 m_VAO = 0;
	u32 m_EBO = 0;
	u32 m_VBO = 0;
	u32 m_DefinitionLevel;
	std::vector<v_Vertex> m_Vertices  = {};
	std::vector<u32> m_Indices  = {};
	std::vector<u32> m_Textures  = {};
public:
	Shape(u32 def_level)
	{
		m_DefinitionLevel = def_level > MAX_DEF_LEVEL ? MAX_DEF_LEVEL : def_level;
	}
	virtual ~Shape() = default;
	void Draw(Shader const &);
    void BindVAO(void);
    void DrawIndices(void);

	void AddTexture(std::string file_path);
};

class Quad: public Shape
{
private:
public:
	Quad(u32 def_level): Shape(def_level)
	{
		m_Vertices.reserve((m_DefinitionLevel + 1) * (m_DefinitionLevel + 1));
		for (u32 y = 0; y <= m_DefinitionLevel; y++)
		{
			for (u32 x = 0; x <= m_DefinitionLevel; x++)
			{
				f32 NDCX = ((f32(x) / f32(m_DefinitionLevel)) * 2.0f) - 1.0f;
				f32 NDCY = ((f32(y) / f32(m_DefinitionLevel)) * 2.0f) - 1.0f;
                f32 U = f32(x) / f32(m_DefinitionLevel);
                f32 V = f32(y) / f32(m_DefinitionLevel);
				m_Vertices.push_back({NDCX, NDCY, 0.0f, U, V});
                printf("X: %02f Y: %02f\n", NDCX, NDCY);
                printf("U: %02f V: %02f\n", U, V);
			}
		}
		m_Indices.reserve(m_DefinitionLevel * m_DefinitionLevel * 6);
		for (u32 y = 0; y < m_DefinitionLevel; y++)
		{
			u32 RowStart = y * (m_DefinitionLevel + 1);
			u32 NextRowStart = (y + 1) * (m_DefinitionLevel + 1);
			for (u32 x = 0; x < m_DefinitionLevel; x++)
			{
				m_Indices.insert(m_Indices.end(), {
					RowStart + x,
					RowStart + x + 1,
					NextRowStart + x,
					RowStart + x + 1,
					NextRowStart + x + 1,
					NextRowStart + x
				});
			}
		}

		glGenVertexArrays(1, &m_VAO);
		glBindVertexArray(m_VAO);

		glGenBuffers(1, &m_VBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(v_Vertex), m_Vertices.data(), GL_STATIC_DRAW);
		
		glGenBuffers(1, &m_EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(u32), m_Indices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(v_Vertex), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(v_Vertex), (void*)(3 * sizeof(f32)));
		glEnableVertexAttribArray(1);

        glBindVertexArray(0);
	}

};



#endif
