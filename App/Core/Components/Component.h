// ============================================================================
// Component.h - Classe de base pour les composants (pattern ECS)
// ============================================================================
// Un Component est un comportement qui peut être attaché à un GameObject.
// Cette architecture permet de composer des entités par combinaison de
// fonctionnalités plutôt que par héritage profond.
//
// Exemples de composants :
//   - USRenderComponent : rendu d'un mesh avec texture
//   - TransformComponent : position, rotation, échelle (TODO)
//   - PhysicsComponent   : simulation physique (TODO)
//
// Utilisation :
//   GameObject->AddComponent<USRenderComponent>();
//
// ============================================================================
#pragma once
#include "../BaseObject.h"

class USComponent : public USBaseObject {
	// Classe de base vide pour l'instant
	// Les méthodes Tick(), Draw(), Reset() sont héritées de USBaseObject
};
