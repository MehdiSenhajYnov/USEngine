#pragma once
#include <glm/glm.hpp>
#include "Component.h"

class USCameraComponent : public USComponent
{
public:
	USCameraComponent();

	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetProjectionMatrix(float aspect) const;

	bool IsMovable() const;
	void SetMovable(bool value);

	const glm::vec3& GetPosition() const;
	void SetPosition(const glm::vec3& pos);
	void Move(const glm::vec3& delta);

	glm::vec2 GetRotation() const;
	void SetRotation(const glm::vec2& rot);
	void AddRotation(const glm::vec2& delta);

private:
	glm::vec3 position;
	glm::vec2 rotation;
	float fovDeg;
	float nearPlane;
	float farPlane;
	float movementSpeed;
	float mouseSensitivity;
	bool isMovable;
};
