#pragma once
#include "GameObject.h"

class USRenderComponent;

// GameObject avec un RenderComponent créé automatiquement
class USRendererObject : public USGameObject {
public:
	USRendererObject();
	USRenderComponent* RenderComponent;
};
