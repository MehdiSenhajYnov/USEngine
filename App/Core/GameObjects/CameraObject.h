#pragma once
#include "GameObject.h"
#include "../Components/CameraComponent.h"
#include "../Components/TransformComponent.h"

class CameraObject : public USGameObject
{
public:
	CameraObject();
	USCameraComponent* CameraComponent;
	USTransformComponent* Transform;
};
