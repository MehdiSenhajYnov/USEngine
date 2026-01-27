// ============================================================================
// List.h - Container personnalisé étendant std::vector
// ============================================================================
// USList est une extension de std::vector qui ajoute des méthodes utilitaires
// pour manipuler les collections de manière plus expressive.
//
// Méthodes ajoutées :
//   Suppression :
//     - RemoveElement(element)          : supprime par valeur
//     - RemoveFirstWith(predicate)      : supprime le 1er élément qui match
//     - RemoveAllWith(predicate)        : supprime tous les éléments qui match
//
//   Recherche :
//     - Contains(element)               : vérifie la présence
//     - Contains(predicate)             : vérifie si un élément match
//     - GetFirstWith(predicate)         : retourne le 1er élément qui match
//     - GetAllWith(predicate)           : retourne tous les éléments qui match
//
//   Utilitaires :
//     - AppendVector(vector)            : ajoute les éléments d'un autre vecteur
//
// Les prédicats sont des std::function<bool(const T&)> permettant des
// recherches/suppressions conditionnelles.
//
// ============================================================================
#pragma once
#include <vector>
#include <algorithm>
#include <functional>

template<typename T>
class USList : public std::vector<T> {
public:
	USList(std::initializer_list<T> List) : std::vector<T>(List) {}
	USList() : std::vector<T>() {}

	// Supprime la première occurrence de l'élément
	void RemoveElement(const T& element) {
		this->erase(std::remove(this->begin(), this->end(), element));
	}

	// Supprime le premier élément qui satisfait le prédicat
	// Retourne true si un élément a été supprimé
	bool RemoveFirstWith(const std::function<bool(const T&)>& _predicate)
	{
		auto it = std::find_if(this->begin(), this->end(), _predicate);
		if (it != this->end()) {
			this->erase(it);
			return true;
		}
		return false;
	}

	// Variante qui retourne aussi l'élément supprimé
	bool RemoveFirstWith(const std::function<bool(const T&)>& _predicate, T& RemovedValue)
	{
		auto it = std::find_if(this->begin(), this->end(), _predicate);
		if (it != this->end()) {
			RemovedValue = *it;
			this->erase(it);
			return true;
		}
		return false;
	}

	// Supprime tous les éléments qui satisfont le prédicat
	void RemoveAllWith(const std::function<bool(const T&)>& _predicate)
	{
		this->erase(std::remove_if(this->begin(), this->end(), _predicate), this->end());
	}

	// Vérifie si l'élément est présent dans la liste
	bool Contains(const T& element)
	{
		return std::count(this->begin(), this->end(), element) > 0;
	}

	// Vérifie si au moins un élément satisfait le prédicat
	bool Contains(const std::function<bool(const T&)>& _predicate)
	{
		return std::find_if(this->begin(), this->end(), _predicate) != this->end();
	}

	// Retourne le premier élément qui satisfait le prédicat (ou nullptr)
	T GetFirstWith(const std::function<bool(const T&)>& _predicate)
	{
		auto it = std::find_if(this->begin(), this->end(), _predicate);
		if (it != this->end()) {
			return *it;
		}
		return nullptr;
	}

	// Retourne une nouvelle liste avec tous les éléments qui satisfont le prédicat
	USList<T> GetAllWith(const std::function<bool(const T&)>& _predicate)
	{
		USList<T> result;
		for (const T& element : *this) {
			if (_predicate(element)) {
				result.push_back(element);
			}
		}
		return result;
	}

	// Ajoute tous les éléments d'un autre vecteur à la fin de cette liste
	void AppendVector(const std::vector<T>& _list)
	{
		this->insert(this->end(), _list.begin(), _list.end());
	}
};
