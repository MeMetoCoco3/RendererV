#include "shapes.h"
#include "algorithm"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <string>

void Shape::Draw(const Shader& shader)
{
	shader.UseProgram();
	glBindVertexArray(m_VAO);
	for (int i = 0; i < m_Textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_Textures[i]);

		shader.SetInt(std::string("texture" + std::to_string(i)), i);
	}
	glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

u32 Shape::m_LoadTexture(const std::string file_path)
{
	u32 TextureID;
	if (S_FILES::Textures.contains(file_path))
	{
		TextureID = S_FILES::Textures[file_path];
	}
	else
	{
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
			S_FILES::Textures[str] = TextureID;
		}
		else
		{
			throw std::runtime_error("Not able to read image");
		}
		stbi_image_free(data);
	}
	return TextureID;
}

void Shape::AddTexture(std::string file_path)
{
	m_Textures.push_back(m_LoadTexture(file_path));
}

