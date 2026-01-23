#ifndef VSHADERS
#define VSHADERS

#include "vstd/vtypes.h"
#include "vstd/vlogger.h"
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include "vstd/vgeneral.h"
#include <glm/glm.hpp>

class Shader 
{
private:
	u32 m_Program;
public:
	Shader(const std::string& VS_PATH, const std::string& FS_PATH)
	{
		u32 VertexShader = glCreateShader(GL_VERTEX_SHADER);

		std::string VertexSource = LoadFile(VS_PATH);
		const char* source = VertexSource.c_str();
		glShaderSource(VertexShader, 1, &source, NULL);
		glCompileShader(VertexShader);
		
		int  Success;
		char InfoLog[512];
		glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Success);

		if (!Success)
		{
			glGetShaderInfoLog(VertexShader, 512, NULL, InfoLog);
			V_LOG_ERROR("ERROR::SHADER::VERTEX:COMPILATION\n");
			V_LOG_ERROR(InfoLog);
		}
		

		u32 FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		std::string FragmentSource = LoadFile(FS_PATH);
		source = FragmentSource.c_str();
		glShaderSource(FragmentShader, 1, &source, NULL);
		glCompileShader(FragmentShader);

		glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Success);
		if (!Success)
		{
			glGetShaderInfoLog(FragmentShader, 512, NULL, InfoLog);
			V_LOG_ERROR("ERROR::SHADER::FRAGMENT:COMPILATION\n");
			V_LOG_ERROR(InfoLog);
		}

		m_Program = glCreateProgram();
		glAttachShader(m_Program, VertexShader);
		glAttachShader(m_Program, FragmentShader);
		glLinkProgram(m_Program);

		glGetShaderiv(m_Program, GL_LINK_STATUS, &Success);
		if (!Success)
		{
			glGetShaderInfoLog(m_Program, 512, NULL, InfoLog);
			V_LOG_ERROR("ERROR::SHADER::LINKAGE\n");
			V_LOG_ERROR(InfoLog);
		}
		
		glDeleteShader(VertexShader);
		glDeleteShader(FragmentShader);

	}

	inline u32 GetProgram(void) const
	{
		return m_Program;
	}

	inline void UseProgram(void) const 
	{
		glUseProgram(m_Program);
	}

	void SetBool(const std::string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(m_Program, name.c_str()), (int)value);
	}
	void SetInt(const std::string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(m_Program, name.c_str()), value);
	}
	void SetFloat(const std::string& name, float value) const
	{
		glUniform1f(glGetUniformLocation(m_Program, name.c_str()), value);
	}
	void SetVec2(const std::string& name, float x, float y) const
	{
		glUniform2f(glGetUniformLocation(m_Program, name.c_str()), x, y);
	}
	void SetVec3(const std::string& name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(m_Program, name.c_str()), x, y, z);
	}
	void SetVec4(const std::string& name, float x, float y, float z, float w)
	{
		glUniform4f(glGetUniformLocation(m_Program, name.c_str()), x, y, z, w);
	}
	void SetMat2(const std::string& name, const glm::mat2& mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(m_Program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	void SetMat3(const std::string& name, const glm::mat3& mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(m_Program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	void SetMat4(const std::string& name, const glm::mat4& mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(m_Program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

};


#endif