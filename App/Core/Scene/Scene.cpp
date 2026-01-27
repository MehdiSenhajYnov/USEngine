#include "Scene.h"
#include "../GameObjects/GameObject.h"

void USScene::Tick(float deltaTime)
{
	for(std::unique_ptr<USGameObject>& gameObject : GameObjects)
	{
		gameObject->Tick(deltaTime);
	}
}

void USScene::Draw(vde::core::gpu::RenderingCommandEncoder& Rendering)
{
	for(std::unique_ptr<USGameObject>& gameObject : GameObjects)
	{
		gameObject->Draw(Rendering);
	}
}

bool USScene::DestroyGameObject(USGameObject* ToDestroy)
{
	if (!ToDestroy) return false;

	return GameObjects.RemoveFirstWith([ToDestroy](const std::unique_ptr<USGameObject>& ptr) {
		return ptr.get() == ToDestroy;
	});
	
	// Rappel : Le unique_ptr libère automatiquement la mémoire, pas besoin de delete

	
}

void USScene::Reset()
{
	USBaseObject::Reset();
	for(std::unique_ptr<USGameObject>& gameObject : GameObjects)
	{
		gameObject->Reset();
	}
}

