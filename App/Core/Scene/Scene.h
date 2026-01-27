#pragma once
#include <map>
#include <memory>

#include "../BaseObject.h"
#include "../List.h"
#include "../AllConcepts.h"

class USGameObject;

class USScene : public USBaseObject {
public:
	void Tick(float deltaTime) override;
	void Draw(vde::core::gpu::RenderingCommandEncoder& Rendering) override;
	
	USList<std::unique_ptr<USGameObject>> GameObjects;
	
	template<typename T> requires IsDerivedFrom<T, USGameObject>
	T* CreateGameObject();

	bool DestroyGameObject(USGameObject* ToDestroy);

	void Reset() override;
};

template <typename T> requires IsDerivedFrom<T, USGameObject>
T* USScene::CreateGameObject()
{
	GameObjects.push_back(std::make_unique<T>());
	return static_cast<T*>(GameObjects.back().get());
}
