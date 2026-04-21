#include "MeshRendererObject.h"

#include "../Components/Render/MeshRenderComponent.h"

USMeshRendererObject::USMeshRendererObject()
	: USGameObject()
{
	MeshRenderComponent = &AddComponent<USMeshRenderComponent>();
}
