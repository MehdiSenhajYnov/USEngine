#pragma once

#include "GameObject.h"

class USMeshRenderComponent;

class USMeshRendererObject : public USGameObject
{
public:
	USMeshRendererObject();
	USMeshRenderComponent* MeshRenderComponent = nullptr;
};
