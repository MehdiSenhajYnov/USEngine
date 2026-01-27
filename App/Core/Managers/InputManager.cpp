#include "InputManager.h"

#include <GLFW/glfw3.h>
#include <core/window.h>


void InputManager::Initialize(vde::core::Window& window)
{
	windowHandle = window.GetGLFWWindow();
	// Optional: lock the cursor for FPS camera
	glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void InputManager::Update()
{
	if (!windowHandle) return;

	double x, y;
	glfwGetCursorPos(windowHandle, &x, &y);

	if (firstMouse)
	{
		lastX = x;
		lastY = y;
		firstMouse = false;
		mouseDelta = { 0.0f, 0.0f };
		return;
	}

	mouseDelta = {float(x - lastX), float(y-lastY)};
	lastX = x;
	lastY = y;
}

bool InputManager::IsKeyDown(int key) const
{
	if (!windowHandle) return false;
	return glfwGetKey(windowHandle, key) == GLFW_PRESS;
}

glm::vec2 InputManager::ConsumeMouseDelta()
{
	glm::vec2 d = mouseDelta;
	mouseDelta = { 0.0f, 0.0f };
	return d;
}