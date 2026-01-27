#pragma once
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "Component.h"
#include "../List.h"

class USTransformComponent : public USComponent {
public:
	glm::vec3 GetLocalPosition() const {
		return Position;
	}
	void SetPosition(const glm::vec3& NewPosition) {
		Position = NewPosition;
	}
	glm::vec3 GetLocalRotation() const {
		return Rotation;
	}
	void SetRotation(const glm::vec3& NewRotation) {
		Rotation = NewRotation;
	}
	glm::vec3 GetLocalScale() const {
		return Scale;
	}
	void SetLocalScale(const glm::vec3& NewScale) {
		Scale = NewScale;
	}

	void Tick(float deltaTime) override;

	glm::vec3 GetWorldPosition() const
	{
		if (Parent) {
		}
		return Position;
	}

	glm::mat4 GetTransform() const
	{
		auto S = glm::scale(glm::mat4(1.0f), GetLocalScale());
		auto RS = S * glm::orientate4(Rotation);
		return glm::translate(RS, Position);
	}

	glm::mat4 GetWorldTransform() const
	{
		if (Parent) {
			return Parent->GetWorldTransform() * GetTransform();
		}
		return GetTransform();
	}

	USTransformComponent* GetParent() const {
		return Parent;
	}

	size_t GetChildCount() const
	{
		return Children.size();
	}


	void AddChild(USTransformComponent* Child) {
		Child->Parent = this;
		Children.push_back(Child);
	}

	void RemoveChild(USTransformComponent* Child) {
		Child->Parent = nullptr;
		Children.RemoveElement(Child);
	}

	void SetParent(USTransformComponent* NewParent) {
		if (Parent) {
			Parent->RemoveChild(this);
		}
		Parent = NewParent;
		if (Parent) {
			Parent->AddChild(this);
		}
	}

	void RemoveParent()
	{
		if (Parent) {
			Parent = nullptr;
			Parent->RemoveChild(this);
		}
	}

private:
	glm::vec3 Position{ 0.0f, 0.0f, 0.0f };
	glm::vec3 Rotation{ 0.0f, 0.0f, 0.0f };
	glm::vec3 Scale{ 1.0f, 1.0f, 1.0f };


	USTransformComponent* Parent = nullptr;
	USList<USTransformComponent*> Children;
};
