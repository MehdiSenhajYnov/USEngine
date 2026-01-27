#pragma once
#include "../Components/CameraComponent.h"

class CameraManager
{
public:
	static CameraManager& GetInstance()
	{
		static CameraManager instance;
		return instance;
	}

	void SetMainCamera(USCameraComponent* Camera) { MainCamera = Camera; }
	USCameraComponent* GetMainCamera() const { return MainCamera; }

	void Update(float dt);

private:
	USCameraComponent* MainCamera = nullptr;
	float moveSpeed = 3.0f;
	float mouseSensitivity = 0.1f;
	CameraManager() = default;
};
