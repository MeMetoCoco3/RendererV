
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "vstd/vlogger.h"
#include "vstd/vgeneral.h"
#include "main.h"
#include "camera.h"
#include "iostream"
#include <array>
#include "shaders.h"
#include <Windows.h>
#include "perlin.h"
#include <random>
#include "shapes.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/>

constexpr auto WIDTH = 1000.0f;
constexpr auto HEIGHT = 800.0f;
constexpr auto ASPECT_RATIO = WIDTH / HEIGHT;
constexpr auto FOVY = 45.0f;
constexpr auto NEAR_PLANE = 0.01f;
constexpr auto FAR_PLANE = 100.0f;

constexpr auto CHANNEL_NUM = 4;

constexpr auto SIZE_COLOR_BUFFER = WIDTH * HEIGHT * CHANNEL_NUM;

// camera
Camera camera({ 0.0f, 0.0f, 3.0f });
float lastX = WIDTH * 0.5f;
float lastY = HEIGHT * 0.5f;
bool firstMouse = true;

int main() 
{
	Logger::SetLevelDefault();

	
	GLFWwindow *Window = GetGLFWWindow();
	glViewport(0, 0, WIDTH, HEIGHT);
	glfwSetFramebufferSizeCallback(Window, framebuffer_size_callback);	
	glfwSetCursorPosCallback(Window, mouse_callback);
	glfwSetScrollCallback(Window, scroll_callback);
	
	glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	Shader shader(SHADERS_PATH "vs.glsl", SHADERS_PATH "fs.glsl");

	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Quad quad(1);
	quad.AddTexture(TEXTURE_PATH "wood.png");
	glm::mat4 model_mat = glm::mat4(1.0f);
	model_mat = glm::translate(model_mat, glm::vec3(0.0f, .0f, -2.0f));

	glm::mat4 proj_mat = glm::perspective(glm::radians(FOVY), ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);
	f32 delta_time = 0.0f;
	f32 last_frame = 0.0f;
	while (!glfwWindowShouldClose(Window))
	{
		f32 current_frame = static_cast<f32>(glfwGetTime());
		delta_time = current_frame - last_frame;
		last_frame = current_frame;

		ProcessInput(Window, delta_time);
		

		glClearColor(0.1f, 0.4f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		shader.SetMat4("model_mat", model_mat);
		shader.SetMat4("proj_mat", proj_mat);
		shader.SetMat4("view_mat", camera.GetViewMatrix());
		quad.Draw(shader);

		glfwSwapBuffers(Window);
		glfwPollEvents();
	}

	glfwTerminate();

	//VirtualFree(ColorBuffer, 0, MEM_RELEASE);
	return 0;
}


void ProcessInput(GLFWwindow * window, f32 delta_time)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, delta_time);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, delta_time);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, delta_time);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, delta_time);
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

GLFWwindow* GetGLFWWindow() 
{
	if (!glfwInit())
	{
		glfwTerminate();
		throw std::runtime_error("GLFW WAS NOT INITIALIZED\n");
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifndef MACOS
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* Window = glfwCreateWindow(WIDTH, HEIGHT, "Perlin Noise", NULL, NULL);
	if (Window == NULL)
	{
		glfwTerminate();
		throw std::runtime_error("FAILED CREATE GLFW WINDOW");
	}

	glfwMakeContextCurrent(Window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		glfwTerminate();
		throw std::runtime_error("FAILED INITIALIZE GLAD");
	}

	return Window;	
}

void mouse_callback(GLFWwindow* window, f64 xposIn, f64 yposIn)
{
	float xpos = static_cast<f32>(xposIn);
	float ypos = static_cast<f32>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}


void scroll_callback(GLFWwindow* window, f64 xoffset, f64 yoffset)
{
	camera.ProcessMouseScroll(static_cast<f32>(yoffset));
}
