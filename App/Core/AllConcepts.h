#pragma once
#include <iostream>
#include <vector>

// Vérifie la relation d'héritage (gère aussi les types pointeurs)
template<typename Base, typename Derived>
constexpr bool isBaseOf()
{
	if constexpr (std::is_pointer_v<Base>)
	{
		using CleanedBase = std::remove_cvref_t<std::remove_pointer_t<Base>>;
		return isBaseOf<CleanedBase, Derived>();
	}
	if constexpr (std::is_pointer_v<Derived>)
	{
		using CleanedDerived = std::remove_cvref_t<std::remove_pointer_t<Derived>>;
		return isBaseOf<Base, CleanedDerived>();
	}
	return std::is_base_of_v<Base, Derived>;
}

template<typename Derived, typename Base>
concept IsDerivedFrom = isBaseOf<Base, Derived>();

template<typename Base, typename Derived>
concept IsBaseOf = isBaseOf<Base, Derived>();