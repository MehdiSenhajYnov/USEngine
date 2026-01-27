#include "RendererObject.h"

#include "../Components/Render/RenderComponent.h"


RendererObject::RendererObject() : USGameObject()
{
	RenderComponent = &AddComponent<USRenderComponent>();
}
