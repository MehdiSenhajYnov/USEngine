#include "CameraComponent.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

USCameraComponent::USCameraComponent()
	:	fovDeg(45.0f),
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
	if (!Transform) return glm::mat4(1.0f);

	const glm::vec3 rotation = Transform->GetLocalRotation();
	const float yaw = glm::radians(rotation.x);
	const float pitch = glm::radians(rotation.y);

	glm::vec3 forward;
	forward.x = cosf(pitch) * sinf(yaw);
	forward.y = sinf(pitch);
	forward.z = cosf(pitch) * cosf(yaw);

	forward = glm::normalize(forward);
	const glm::vec3 position = Transform->GetLocalPosition();
	return glm::lookAt(position, position + forward, glm::vec3(0.0f, 1.0f, 0.0f));
}

// Projection perspective compatible Vulkan 
glm::mat4 USCameraComponent::GetProjectionMatrix(float aspect) const
{
	return glm::perspective(glm::radians(fovDeg), aspect, nearPlane, farPlane);
}

bool USCameraComponent::IsMovable() const { return isMovable; }
void USCameraComponent::SetMovable(bool value) { isMovable = value; }

USTransformComponent* USCameraComponent::GetTransform() const { return Transform; }
void USCameraComponent::SetTransform(USTransformComponent* transform) { Transform = transform; }