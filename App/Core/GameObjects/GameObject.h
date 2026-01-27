#pragma once
#include "../BaseObject.h"
#include "../List.h"
#include <memory>

#include "../AllConcepts.h"

class USComponent;

class USGameObject : public USBaseObject {
public:
	USList<std::unique_ptr<USComponent>> Components;
	template<typename T> requires IsDerivedFrom<T, USComponent>
	T& AddComponent();

	void Tick(float deltaTime) override;
	void Draw(vde::core::gpu::RenderingCommandEncoder& Rendering) override;

	void Reset() override;
};

template <typename T> requires IsDerivedFrom<T, USComponent>
T& USGameObject::AddComponent()
{
	Components.push_back(std::make_unique<T>());
	return static_cast<T&>(*Components.back());
}


// Affichage mesh
// Transform component
