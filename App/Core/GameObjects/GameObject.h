#pragma once
#include "../BaseObject.h"
#include "../List.h"
#include <memory>
#include <expected>

#include "../AllConcepts.h"
#include "../Components/Component.h"

class USTransformComponent;
class USComponent;

class USGameObject : public USBaseObject {
public:
	USGameObject();
	USList<std::unique_ptr<USComponent>> Components;

	template<typename T> requires IsDerivedFrom<T, USComponent>
	T& AddComponent();

	// A refaire sans cast a chaque fois
	//template<typename T> requires IsDerivedFrom<T, USComponent>
	//bool GetComponent(T& OutComponent)
	//{
	//	for (std::unique_ptr<USComponent>& component : Components)
	//	{
	//		if (T* casted = dynamic_cast<T*>(component.get()))
	//		{
	//			OutComponent = *casted;
	//			return true;
	//		}
	//	}
	//	return false;
	//}

	void Tick(float deltaTime) override;
	void Draw(vde::core::gpu::RenderingCommandEncoder& Rendering) override;
	void Reset() override;

	USTransformComponent* Transform = nullptr;
};

template <typename T> requires IsDerivedFrom<T, USComponent>
T& USGameObject::AddComponent()
{
	Components.push_back(std::make_unique<T>());
	Components.back()->SetOwner(this);
	return static_cast<T&>(*Components.back());
}
