#include "nhwdevice.h"
#include "GLFW\glfw3.h"

bool IsKeyPressed(void* window, uint32_t key)
{
	return glfwGetKey((GLFWwindow*)window, key);
}

bool IsButtonPressed(void* window, uint32_t key)
{
	return glfwGetMouseButton((GLFWwindow*)window, key);
}

void GetCursorPos(void* window, float* x, float* y)
{
	double x1, y1;
	glfwGetCursorPos((GLFWwindow*)window, &x1, &y1);
	*x = x1; *y = y1;
}

void SetCursorPos(void* window, float x, float y) {
	glfwSetCursorPos((GLFWwindow*)window, x, y);
}

void UseRaw(void* window, bool use)
{
	glfwSetInputMode((GLFWwindow*)window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}