#pragma once
#include "GameObject.h"
#include "../Components/CameraComponent.h"

class CameraObject : public USGameObject
{
public:
	CameraObject();
	USCameraComponent* CameraComponent;
};
