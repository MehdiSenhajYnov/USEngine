// ============================================================================
// BaseObject.h - Classe de base pour tous les objets du moteur
// ============================================================================
// USBaseObject est la racine de la hiérarchie d'objets de l'application.
// Elle définit l'interface commune pour le cycle de vie des objets :
//
//   - Tick(deltaTime)  : appelée chaque frame pour la logique/mise à jour
//   - Draw(Rendering)  : appelée chaque frame pour le rendu
//   - Reset()          : libération des ressources (destruction propre)
//
// Hiérarchie :
//   USBaseObject
//     ├── USComponent      (comportements attachables)
//     │     └── USRenderComponent
//     ├── USGameObject     (entités avec composants)
//     │     └── RendererObject
//     └── USScene          (conteneur de GameObjects)
//
// ============================================================================
#pragma once

namespace vde::core::gpu { class RenderingCommandEncoder; }

class USBaseObject {
public:
	int Id = -1;  // Identifiant unique de l'objet (-1 = non assigné)

	virtual ~USBaseObject() = default;

	// Appelée chaque frame pour la mise à jour logique (animation, physique, etc.)
	virtual void Tick(float deltaTime) {}

	// Appelée chaque frame pour le rendu (enregistrement des commandes GPU)
	virtual void Draw(vde::core::gpu::RenderingCommandEncoder& Rendering) {}

	// Libération explicite des ressources GPU avant destruction
	// Nécessaire car Vulkan requiert un ordre de destruction spécifique
	virtual void Reset() {}
};
