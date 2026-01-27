#pragma once
#include <glm/glm.hpp>
#include "Component.h"
#include "TransformComponent.h"

class USCameraComponent : public USComponent
{
public:
	USCameraComponent();

	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetProjectionMatrix(float aspect) const;

	bool IsMovable() const;
	void SetMovable(bool value);

	USTransformComponent* GetTransform() const;
	void SetTransform(USTransformComponent* transform);

private:
	USTransformComponent* Transform = nullptr;
	float fovDeg;
	float nearPlane;
	float farPlane;
	float movementSpeed;
	float mouseSensitivity;
	bool isMovable;
};
