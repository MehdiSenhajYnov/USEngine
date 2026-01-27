// ============================================================================
// Scene.h - Conteneur principal des GameObjects
// ============================================================================
// Une Scene gère le cycle de vie d'un ensemble de GameObjects.
// C'est le point d'entrée pour créer, mettre à jour et dessiner les objets.
//
// Responsabilités :
//   - Création de GameObjects via CreateGameObject<T>()
//   - Propagation des appels Tick() et Draw() à tous les objets
//   - Destruction propre des objets via DestroyGameObject() ou Reset()
//
// Utilisation typique :
//   USScene scene;
//   RendererObject* obj = scene.CreateGameObject<RendererObject>();
//   obj->RenderComponent->Init(...);
//
//   // Dans la boucle de rendu :
//   scene.Tick(deltaTime);
//   scene.Draw(rendering);
//
//   // À la fin :
//   scene.Reset();  // Libère toutes les ressources GPU
//
// ============================================================================
#pragma once
#include <map>
#include <memory>

#include "../BaseObject.h"
#include "../List.h"
#include "../AllConcepts.h"

class USGameObject;

class USScene : public USBaseObject {
public:
	// Appelle Tick() sur tous les GameObjects
	void Tick(float deltaTime) override;

	// Appelle Draw() sur tous les GameObjects
	void Draw(vde::core::gpu::RenderingCommandEncoder& Rendering) override;

	// Liste des GameObjects de la scène (ownership via unique_ptr)
	USList<std::unique_ptr<USGameObject>> GameObjects;

	// Crée un GameObject de type T et retourne un pointeur vers lui
	// T doit hériter de USGameObject (vérifié par concept C++23)
	template<typename T> requires IsDerivedFrom<T, USGameObject>
	T* CreateGameObject();

	// Supprime un GameObject de la scène (libère automatiquement la mémoire)
	bool DestroyGameObject(USGameObject* ToDestroy);

	// Libère les ressources GPU de tous les GameObjects
	void Reset() override;
};

// ----------------------------------------------------------------------------
// Implémentation template (doit être dans le header)
// ----------------------------------------------------------------------------
template <typename T> requires IsDerivedFrom<T, USGameObject>
T* USScene::CreateGameObject()
{
	GameObjects.push_back(std::make_unique<T>());
	return static_cast<T*>(GameObjects.back().get());
}
