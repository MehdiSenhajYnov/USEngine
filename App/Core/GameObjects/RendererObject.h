#pragma once
#include "GameObject.h"

class USRenderComponent;

class RendererObject : public USGameObject{
public:
	RendererObject();

	USRenderComponent* RenderComponent;
};
