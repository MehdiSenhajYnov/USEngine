#include "RendererObject.h"

#include "../Components/Render/RenderComponent.h"


USRendererObject::USRendererObject() : USGameObject()
{
	RenderComponent = &AddComponent<USRenderComponent>();
}
