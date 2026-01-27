#include "CameraManager.h"

#include <glm/gtx/norm.inl>
#include <GLFW/glfw3.h>
#include "InputManager.h"

void CameraManager::Update(float dt)
{
	if (!MainCamera) return;
	
	auto& input = InputManager::GetInstance();
	glm::vec2 mouse = input.ConsumeMouseDelta();

	if (MainCamera->IsMovable())
	{
		MainCamera->AddRotation({ mouse.x * mouseSensitivity, -mouse.y * mouseSensitivity });

		glm::vec2 rot = MainCamera->GetRotation();
		rot.y = glm::clamp(rot.y, -89.0f, 89.0f);
		MainCamera->SetRotation(rot);

		const float yaw = glm::radians(rot.x);
		const float pitch = glm::radians(rot.y);

		glm::vec3 forward;
		forward.x = cosf(pitch) * sinf(yaw);
		forward.y = sinf(pitch);
		forward.z = cosf(pitch) * cosf(yaw);
		forward = glm::normalize(forward);

		glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), forward));

		//Déplacement sur le plan XZ
		glm::vec3 moveForward = glm::normalize(glm::vec3(forward.x, 0.0f, forward.z));

		glm::vec3 move(0.0f);
		if (input.IsKeyDown(GLFW_KEY_Z) || input.IsKeyDown(GLFW_KEY_W)) move += moveForward;
		if (input.IsKeyDown(GLFW_KEY_S)) move -= moveForward;
		if (input.IsKeyDown(GLFW_KEY_Q) || input.IsKeyDown(GLFW_KEY_A)) move -= right;
		if (input.IsKeyDown(GLFW_KEY_D)) move += right;

		if (glm::length2(move) > 0.0f)
		{
			MainCamera->Move(glm::normalize(move) * moveSpeed * dt);
		}
	}
}
