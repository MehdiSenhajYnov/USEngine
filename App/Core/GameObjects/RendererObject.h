#pragma once
#include "GameObject.h"

class USRenderComponent;

// GameObject avec un RenderComponent créé automatiquement
class RendererObject : public USGameObject {
public:
	RendererObject();
	USRenderComponent* RenderComponent;
};
