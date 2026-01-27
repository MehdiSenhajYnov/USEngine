#include "CameraComponent.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

USCameraComponent::USCameraComponent()
	:	position(0.0f, 0.0f, 3.0f), 
		rotation(0.0f, 0.0f), fovDeg(45.0f),
		nearPlane(0.1f), 
		farPlane(100.0f), 
		isMovable(true),
		movementSpeed(5.0f), 
		mouseSensitivity(0.1f)
{
}

// Convertit yaw/pitch en vecteur forward et génère la matrice de vue
glm::mat4 USCameraComponent::GetViewMatrix() const
{
	const float yaw = glm::radians(rotation.x);
	const float pitch = glm::radians(rotation.y);

	glm::vec3 forward;
	forward.x = cosf(pitch) * sinf(yaw);
	forward.y = sinf(pitch);
	forward.z = cosf(pitch) * cosf(yaw);

	forward = glm::normalize(forward);
	return glm::lookAt(position, position + forward, glm::vec3(0.0f, 1.0f, 0.0f));
}

// Projection perspective compatible Vulkan 
glm::mat4 USCameraComponent::GetProjectionMatrix(float aspect) const
{
	return glm::perspective(glm::radians(fovDeg), aspect, nearPlane, farPlane);
}

bool USCameraComponent::IsMovable() const { return isMovable; }
void USCameraComponent::SetMovable(bool value) { isMovable = value; }

const glm::vec3& USCameraComponent::GetPosition() const { return position; }
void USCameraComponent::SetPosition(const glm::vec3& pos) { position = pos; }
void USCameraComponent::Move(const glm::vec3& delta) { position += delta; }

glm::vec2 USCameraComponent::GetRotation() const { return rotation; }
void USCameraComponent::SetRotation(const glm::vec2& rot) { rotation = rot; }
void USCameraComponent::AddRotation(const glm::vec2& delta) { rotation += delta; }