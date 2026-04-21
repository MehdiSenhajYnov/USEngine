#pragma once
#include <GLFW/glfw3.h>

#include "core/window.h"

class WindowHandler {
public:
	WindowHandler(vde::core::Window& window) : Handle(window.GetGLFWWindow()) {}
	GLFWwindow* Handle;
};
