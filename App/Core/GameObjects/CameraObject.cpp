#include "CameraObject.h"

CameraObject::CameraObject()
{
	Transform = &AddComponent<USTransformComponent>();
	Transform->SetPosition(glm::vec3(0.0f, 0.0f, 3.0f));

	CameraComponent = &AddComponent<USCameraComponent>();
	CameraComponent->SetTransform(Transform);
}
