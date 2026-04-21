#include "CameraManager.h"

#include <glm/gtx/norm.inl>
#include <GLFW/glfw3.h>
#include "InputManager.h"

void CameraManager::Update(float dt)
{
	if (!MainCamera || !MainCamera->GetTransform()) return;
	
	auto& input = InputManager::GetInstance();
	auto* transform = MainCamera->GetTransform();
	glm::vec2 mouse = input.ConsumeMouseDelta();

	if (MainCamera->IsMovable())
	{
		glm::vec3 rot = transform->GetLocalRotation();
		rot.x += mouse.x * mouseSensitivity;
		rot.y += -mouse.y * mouseSensitivity;
		rot.y = glm::clamp(rot.y, -89.0f, 89.0f);
		transform->SetRotation(rot);

		const float yaw = glm::radians(rot.x);
		const float pitch = glm::radians(rot.y);

		glm::vec3 forward;
		forward.x = cosf(pitch) * sinf(yaw);
		forward.y = sinf(pitch);
		forward.z = cosf(pitch) * cosf(yaw);
		forward = glm::normalize(forward);

		glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), forward));

		// D�placement dans la direction du regard (forward complet avec Y)
		glm::vec3 move(0.0f);
		if (input.IsKeyDown(Key::Z) || input.IsKeyDown(Key::W)) move += moveForward;
		if (input.IsKeyDown(Key::S)) move -= moveForward;
		if (input.IsKeyDown(Key::Q) || input.IsKeyDown(Key::A)) move -= right;
		if (input.IsKeyDown(Key::D)) move += right;

		if (glm::length2(move) > 0.0f)
		{
			glm::vec3 pos = transform->GetLocalPosition();
			pos += glm::normalize(move) * moveSpeed * dt;
			transform->SetPosition(pos);
		}
	}
}
