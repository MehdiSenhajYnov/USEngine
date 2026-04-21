#pragma once
#include <glm/vec2.hpp>

#include "../Input/KeyEnum.h"

class WindowHandler;

class InputManager
{
public:
	static InputManager& GetInstance()
	{
		static InputManager instance;
		return instance;
	}

	void Initialize(WindowHandler* window);
	void Update();
	bool IsKeyDown(Key key) const;
	glm::vec2 ConsumeMouseDelta();

private:
	double lastX = 0.0;
	double lastY = 0.0;
	glm::vec2 mouseDelta = { 0.0f, 0.0f };
	bool firstMouse = true;

	WindowHandler* windowHandle = nullptr;
};
