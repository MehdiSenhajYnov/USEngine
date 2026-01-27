// ============================================================================
// AllConcepts.h - Concepts C++23 pour la vérification de types à la compilation
// ============================================================================
// Ce fichier définit des concepts utilisés pour contraindre les templates.
// Les concepts permettent de vérifier les types à la compilation plutôt
// qu'à l'exécution, offrant de meilleurs messages d'erreur et une sécurité
// de type renforcée.
//
// Concepts définis :
//   - IsDerivedFrom<Derived, Base> : vérifie que Derived hérite de Base
//   - IsBaseOf<Base, Derived>      : vérifie que Base est parent de Derived
//
// Utilisation avec les templates :
//   template<typename T> requires IsDerivedFrom<T, USComponent>
//   T& AddComponent();  // T doit hériter de USComponent
//
// La fonction isBaseOf() gère également les types pointeurs en les
// "nettoyant" automatiquement (suppression de *, const, volatile, &).
//
// ============================================================================
#pragma once
#include <iostream>
#include <vector>

// Fonction constexpr qui vérifie la relation d'héritage à la compilation
// Gère automatiquement les types pointeurs en les déréférençant
template<typename Base, typename Derived>
constexpr bool isBaseOf()
{
	// Si Base est un pointeur, on retire le pointeur et on recommence
	if constexpr (std::is_pointer_v<Base>)
	{
		using CleanedBase = std::remove_cvref_t<std::remove_pointer_t<Base>>;
		return isBaseOf<CleanedBase, Derived>();
	}
	// Si Derived est un pointeur, on retire le pointeur et on recommence
	if constexpr (std::is_pointer_v<Derived>)
	{
		using CleanedDerived = std::remove_cvref_t<std::remove_pointer_t<Derived>>;
		return isBaseOf<Base, CleanedDerived>();
	}
	// Vérification finale avec std::is_base_of_v
	return std::is_base_of_v<Base, Derived>;
}

// Concept : Derived hérite de Base
// Exemple : IsDerivedFrom<USRenderComponent, USComponent> == true
template<typename Derived, typename Base>
concept IsDerivedFrom = isBaseOf<Base, Derived>();

// Concept : Base est parent de Derived (ordre inversé)
// Exemple : IsBaseOf<USComponent, USRenderComponent> == true
template<typename Base, typename Derived>
concept IsBaseOf = isBaseOf<Base, Derived>();