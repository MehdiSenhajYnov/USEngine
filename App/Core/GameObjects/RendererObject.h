// ============================================================================
// RendererObject.h - GameObject prêt pour le rendu
// ============================================================================
// RendererObject est une spécialisation de USGameObject qui inclut
// automatiquement un USRenderComponent pour faciliter l'affichage d'objets.
//
// Cette classe simplifie la création d'objets rendus :
//   - Crée automatiquement un RenderComponent dans le constructeur
//   - Expose un pointeur direct vers le RenderComponent pour un accès rapide
//
// Utilisation :
//   RendererObject* obj = scene.CreateGameObject<RendererObject>();
//   obj->RenderComponent->Init(graphicsContext, "Quad", "Texture", pipeline);
//   obj->RenderComponent->Translate({1.0f, 0.0f, 0.0f});
//
// Le RenderComponent gère :
//   - Les descriptor sets (texture + matrice modèle)
//   - Le buffer uniform de la matrice de transformation
//   - L'enregistrement des commandes de dessin
//
// ============================================================================
#pragma once
#include "GameObject.h"

class USRenderComponent;

class RendererObject : public USGameObject {
public:
	RendererObject();

	// Accès direct au RenderComponent (créé automatiquement)
	USRenderComponent* RenderComponent;
};
