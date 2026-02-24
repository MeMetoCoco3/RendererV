
#include <stdexcept>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "vstd/vlogger.h"
#include "main.h"
#include "camera.h"
#include "shaders.h"
#include <Windows.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "assimp_layer.h"
#include "shapes.h"

constexpr auto WIDTH = 1000;
constexpr auto HEIGHT = 800;
constexpr auto ASPECT_RATIO = (f32)WIDTH / (f32)HEIGHT;
constexpr auto FOVY = 45.0f;
constexpr auto NEAR_PLANE = 0.01f;
constexpr auto FAR_PLANE = 100.0f;

//constexpr auto CHANNEL_NUM = 4;

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
    Shader screen_shader(SHADERS_PATH "screen_vs.glsl", SHADERS_PATH "screen_fs.glsl");

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a color attachment texture
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { 
        printf("ERROR::FRAMEBUFFER:: NOT COMPLETE\n");
    }
    screen_shader.UseProgram();
    screen_shader.SetInt("screen_texture", 0);
    


    // u32 rbo;
    // glGenRenderbuffers(1, &rbo);
    // glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    //
    // if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { 
    //     printf("ERROR::FRAMEBUFFER:: NOT COMPLETE\n");
    // }
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Model Backpack(R"(G:\Render\assets\backpack\backpack.obj)");
	glm::mat4 model_mat = glm::mat4(1.0f);
	model_mat = glm::translate(model_mat, glm::vec3(0.0f, .0f, -2.0f));

	glm::mat4 proj_mat = glm::perspective(glm::radians(FOVY), ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);
	f32 delta_time = 0.0f;
	f32 last_frame = 0.0f;

    Quad quad(1);

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	while (!glfwWindowShouldClose(Window))
	{
		f32 current_frame = static_cast<f32>(glfwGetTime());
		delta_time = current_frame - last_frame;
		last_frame = current_frame;

		ProcessInput(Window, delta_time);
		

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glEnable(GL_DEPTH_TEST);
		glClearColor(0.1f, 0.4f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader.UseProgram();
		shader.SetMat4("model_mat", model_mat);
		shader.SetMat4("proj_mat", proj_mat);
		shader.SetMat4("view_mat", camera.GetViewMatrix());
			
		Backpack.Draw(shader);
        //
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //
        glDisable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        //
        screen_shader.UseProgram();
        //
        quad.BindVAO();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        quad.DrawIndices();
		glfwSwapBuffers(Window);
		glfwPollEvents();
	}

	glfwTerminate();
    // glDeleteFramebuffers(1, &fbo);
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

	GLFWwindow* Window = glfwCreateWindow(WIDTH, HEIGHT, "Render", NULL, NULL);
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
