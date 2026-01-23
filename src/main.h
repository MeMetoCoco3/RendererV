#pragma once

GLFWwindow* GetGLFWWindow(void);
void mouse_callback(GLFWwindow* window, f64 xposIn, f64 yposIn);
void scroll_callback(GLFWwindow* window, f64 xoffset, f64 yoffset);
//void WriteWeirdGradient(void* buff, int XOffset, int YOffset);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void ProcessInput(GLFWwindow* window, f32 delta_time);
//int CountFilesInFolder(const char* file_path);


