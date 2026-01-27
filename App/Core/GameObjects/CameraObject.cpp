#include "CameraObject.h"

CameraObject::CameraObject()
{
	CameraComponent = &AddComponent<USCameraComponent>();
}
