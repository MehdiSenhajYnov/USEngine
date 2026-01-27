#pragma once
#include <glm/vec2.hpp>

struct GLFWwindow;
namespace vde::core { class Window; }

class InputManager
{
public:
	static InputManager& GetInstance()
	{
		static InputManager instance;
		return instance;
	}

	void Initialize(vde::core::Window& window);
	void Update();
	bool IsKeyDown(int key) const;
	glm::vec2 ConsumeMouseDelta();

private:
GLFWwindow* windowHandle = nullptr;
	double lastX = 0.0;
	double lastY = 0.0;
	glm::vec2 mouseDelta = { 0.0f, 0.0f };
	bool firstMouse = true;
	
};
