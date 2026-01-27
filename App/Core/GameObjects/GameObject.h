// ============================================================================
// GameObject.h - Entité de base du système ECS
// ============================================================================
// Un GameObject est une entité dans la scène qui peut contenir plusieurs
// Components. C'est le conteneur principal pour les objets du jeu.
//
// Fonctionnement :
//   - Possède une liste de Components (USList<unique_ptr<USComponent>>)
//   - AddComponent<T>() ajoute un composant avec vérification de type (C++23 concepts)
//   - Tick() propage l'appel à tous les composants
//   - Draw() propage l'appel à tous les composants
//   - Reset() libère les ressources de tous les composants
//
// Exemple :
//   USGameObject* go = scene.CreateGameObject<USGameObject>();
//   USRenderComponent& render = go->AddComponent<USRenderComponent>();
//   render.Init(graphicsContext, "Quad", "Texture", pipeline);
//
// ============================================================================
#pragma once
#include "../BaseObject.h"
#include "../List.h"
#include <memory>

#include "../AllConcepts.h"

class USComponent;

class USGameObject : public USBaseObject {
public:
	// Liste des composants attachés à ce GameObject
	USList<std::unique_ptr<USComponent>> Components;

	// Ajoute un composant de type T (doit hériter de USComponent)
	// Retourne une référence vers le composant créé
	template<typename T> requires IsDerivedFrom<T, USComponent>
	T& AddComponent();

	// Propage Tick à tous les composants
	void Tick(float deltaTime) override;

	// Propage Draw à tous les composants
	void Draw(vde::core::gpu::RenderingCommandEncoder& Rendering) override;

	// Libère les ressources de tous les composants
	void Reset() override;
};

// ----------------------------------------------------------------------------
// Implémentation template (doit être dans le header)
// ----------------------------------------------------------------------------
template <typename T> requires IsDerivedFrom<T, USComponent>
T& USGameObject::AddComponent()
{
	Components.push_back(std::make_unique<T>());
	return static_cast<T&>(*Components.back());
}
