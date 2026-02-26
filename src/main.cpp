
#include <array>
#include <stdexcept>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "vstd/vlogger.h"
#include "main.h"
#include "camera.h"
#include "shaders.h"
#include <Windows.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "assimp_layer.h"
#include "shapes.h"
#include <imgui.h>

constexpr auto WIDTH = 1000;
constexpr auto HEIGHT = 800;
constexpr auto ASPECT_RATIO = (f32)WIDTH / (f32)HEIGHT;
constexpr auto FOVY = 45.0f;
constexpr auto NEAR_PLANE = 0.01f;
constexpr auto FAR_PLANE = 100.0f;

//constexpr auto CHANNEL_NUM = 4;

struct {
    bool move_camera = true;
} Cursor;


Camera camera({ 0.0f, 0.0f, 3.0f });
float lastX = WIDTH * 0.5f;
float lastY = HEIGHT * 0.5f;
bool firstMouse = true;

constexpr auto MAX_KEYS = 512;
// {a,b,c, 0, a,b,c, 0}
// a=pressed, b=down, c=released, 0 to make my life easier
struct keyboard_state
{
    std::array<u8, (MAX_KEYS * 4) / 8> keys;
    std::array<u16, MAX_KEYS / 8 > keys_released;
    std::array<u16, MAX_KEYS / 8 > keys_down;
    u8 count_released;
    u8 count_down;
    void set_pressed(int key, bool value)
    {
        size_t byte = key / 2;
        size_t bit_offset = key % 2 != 0 ? 4 : 0;
        
        int bit_val = value ? 1 : 0;
        // First part forces 0 on the desired bit.
        // Second part is a normal or.
        keys[byte] = (keys[byte] & ~(1<<bit_offset)) | ((bit_val) << bit_offset);
    };

    void set_down(int key, bool value)
    {
        size_t byte = key / 2;
        size_t bit_offset = key % 2 != 0 ? 4 + 1 : 0;
        bit_offset += 1;
        int bit_val = value ? 1 : 0;
        keys[byte] = (keys[byte] & ~(1<<bit_offset)) | ((bit_val) << bit_offset);

        if(value) {
            keys_down[count_down] = key;
            count_down += 1;
        }
    };

    void set_released(int key, bool value)
    {
        size_t byte = key / 2;
        size_t bit_offset = key % 2 != 0 ? 4 : 0;
        bit_offset += 2;

        int bit_val = value ? 1 : 0;
        keys[byte] = (keys[byte] & ~(1<<bit_offset)) | ((bit_val) << bit_offset);

        if(value) {
            keys_down[count_released] = key;
            count_released += 1;
        }
    };

    bool is_pressed(int key) { 
        size_t byte = key / 2;
        size_t bit_offset = key % 2 != 0 ? 4 : 0;
        
        return keys[byte] & (1 << bit_offset);
    };

    bool is_down(int key) {
        size_t byte = key / 2;
        size_t bit_offset = key % 2 != 0 ? 4 : 0;
        bit_offset += 1;

        return keys[byte] & (1 << (bit_offset));
    };

    bool is_released(int key) {
        size_t byte = key / 2;
        size_t bit_offset = key % 2 != 0 ? 4 : 0;
        bit_offset += 2;

        return keys[byte] & (1 << (bit_offset));
    };

    void reset_state() {
        for(int i = 0; i < count_released; i++)
        {
            set_released(keys_released[i], false);
        }
        count_released = 0;

        for(int i = 0; i < count_down; i++)
        {
            set_down(keys_down[i], false);
        }
        count_down = 0;
    };

} Keyboard;


int main() 
{
	Logger::SetLevelDefault();

	GLFWwindow *Window = GetGLFWWindow();
    glfwSwapInterval(1);



	glViewport(0, 0, WIDTH, HEIGHT);
	glfwSetFramebufferSizeCallback(Window, framebuffer_size_callback);	
	glfwSetCursorPosCallback(Window, mouse_callback);
	glfwSetScrollCallback(Window, scroll_callback);
	glfwSetKeyCallback(Window, keyboard_callback);
        
    if (Cursor.move_camera){
        glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

	Shader shader(SHADERS_PATH "vs.glsl", SHADERS_PATH "fs.glsl");
    Shader screen_shader(SHADERS_PATH "screen_vs.glsl", SHADERS_PATH "screen_fs.glsl");

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

    u32 framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    
    u32 texture_framebuffer;
    glGenTextures(1, &texture_framebuffer);
    glBindTexture(GL_TEXTURE_2D, texture_framebuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_framebuffer, 0);

    u32 rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT); 
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); 

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { 
        printf("ERROR::FRAMEBUFFER:: NOT COMPLETE\n");
    }
    screen_shader.UseProgram();
    screen_shader.SetInt("screen_texture", 0);
    

    auto imgui_ctx = ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui::SetCurrentContext(imgui_ctx);

    ImGui_ImplGlfw_InitForOpenGL(Window, true);
    ImGui_ImplOpenGL3_Init();

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

    vec4 colors[5];
    colors[0] = {0.95, 0.95, 0.85, 1.0};
    colors[1] = {0.859, 0.617, 0.507, 1.0};
    colors[2] = {0.753, 0.43, 0.43, 1.0};
    colors[3] = {0.08, 0.12, 0.23, 1.0};
    colors[4] = {0.01, 0.0, 0.152, 1.0};
    
    bool no_postprocessing = false;



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
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        screen_shader.UseProgram();

        // const vec3 colors[5] = vec3[5](vec3(0.95,0.95,0.85), vec3(0.859,0.617,0.507), vec3(0.753,0.43,0.43), vec3(0.08,0.12,0.23), vec3(0.01,0.0,0.152));
        screen_shader.SetVec4("colors[0]", colors[0].x, colors[0].y, colors[0].z, colors[0].w);
        screen_shader.SetVec4("colors[1]", colors[1].x, colors[1].y, colors[1].z, colors[1].w);
        screen_shader.SetVec4("colors[2]", colors[2].x, colors[2].y, colors[2].z, colors[2].w);
        screen_shader.SetVec4("colors[3]", colors[3].x, colors[3].y, colors[3].z, colors[3].w);
        screen_shader.SetVec4("colors[4]", colors[4].x, colors[4].y, colors[4].z, colors[4].w);
    
        screen_shader.SetBool("no_postprocessing", no_postprocessing);


        quad.BindVAO();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_framebuffer);
        quad.DrawIndices();
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::SetNextWindowSize({330, 220});
            ImGui::Begin("Select Color Gradient");
            ImGui::SeparatorText("Color Gradient");
            ImGui::ColorEdit4("Color [0]", &colors[0].x);
            ImGui::ColorEdit4("Color [1]", &colors[1].x);
            ImGui::ColorEdit4("Color [2]", &colors[2].x);
            ImGui::ColorEdit4("Color [3]", &colors[3].x);
            ImGui::ColorEdit4("Color [4]", &colors[4].x);
            ImGui::SeparatorText("Draw Original");
            ImGui::Checkbox("Original", &no_postprocessing);
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(Window);
		glfwPollEvents();
	}
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(imgui_ctx);

	glfwTerminate();
    glDeleteFramebuffers(1, &framebuffer);
    
	return 0;
}


void ProcessInput(GLFWwindow *window, f32 delta_time)
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
    if (Keyboard.is_down(GLFW_KEY_P)){
        Cursor.move_camera = !Cursor.move_camera;
        if (Cursor.move_camera)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        else 
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // This is at the end because the POLLEVENTS is called at the end of the loop
    Keyboard.reset_state();
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
    if(Cursor.move_camera)
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
}


void scroll_callback(GLFWwindow* window, f64 xoffset, f64 yoffset)
{
	camera.ProcessMouseScroll(static_cast<f32>(yoffset));
}


void keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_PRESS)
    {
        if (!Keyboard.is_pressed(key)){
            Keyboard.set_down(key, true);
        }
        Keyboard.set_pressed(key, true);
    }
    
    if(action == GLFW_RELEASE)
    {
        Keyboard.set_released(key, true);
        Keyboard.set_pressed(key, false);
    }
}

