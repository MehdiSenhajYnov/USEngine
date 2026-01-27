#include "GameObject.h"

#include "../Components/Component.h"
#include "core/gpu/buffer.h"


void USGameObject::Tick(float deltaTime)
{
	USBaseObject::Tick(deltaTime);
	for (std::unique_ptr<USComponent>& component : Components)
	{
		component->Tick(deltaTime);
	}
}

void USGameObject::Draw(vde::core::gpu::RenderingCommandEncoder& Rendering)
{
	for (std::unique_ptr<USComponent>& component : Components)
	{
		component->Draw(Rendering);
	}
}

void USGameObject::Reset()
{
	for(std::unique_ptr<USComponent>& component : Components)
	{
		component->Reset();
	}
}
