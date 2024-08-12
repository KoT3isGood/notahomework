#include "nhwdevice.h"
#include "GLFW/glfw3.h"

bool nhwIsKeyPressed(void* window, uint32_t key)
{
	return glfwGetKey((GLFWwindow*)window, key);
}

bool nhwIsButtonPressed(void* window, uint32_t key)
{
	return glfwGetMouseButton((GLFWwindow*)window, key);
}

void nhwGetCursorPos(void* window, float* x, float* y)
{
	double x1, y1;
	glfwGetCursorPos((GLFWwindow*)window, &x1, &y1);
	*x = x1; *y = y1;
}

void nhwSetCursorPos(void* window, float x, float y) {
	glfwSetCursorPos((GLFWwindow*)window, x, y);
}

void nhwUseRaw(void* window, bool use)
{
	glfwSetInputMode((GLFWwindow*)window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}