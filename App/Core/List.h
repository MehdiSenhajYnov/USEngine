#pragma once
#include <vector>
#include <algorithm>
#include <functional>

template<typename T>
class USList : public std::vector<T> {
public:
	USList(std::initializer_list<T> List) : std::vector<T>(List){}
	USList() : std::vector<T>(){}

	void RemoveElement(const T& element) {
		this->erase(std::remove(this->begin(), this->end(), element));
	}
    
	bool RemoveFirstWith(const std::function<bool(const T&)>& _predicate)
	{
		auto it = std::find_if(this->begin(), this->end(), _predicate);
		if (it != this->end()) {
			this->erase(it);
			return true;
		}
		return false;
	}

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

	void RemoveAllWith(const std::function<bool(const T&)>& _predicate)
	{
		this->erase(std::remove_if(this->begin(), this->end(), _predicate), this->end());
	}

	bool Contains(const T& element)
	{
		if (std::count(this->begin(), this->end(), element)) {
			return true;
		}
		return false;
	}
    
	bool Contains(const std::function<bool(const T&)>& _predicate)
	{
		if (std::count(this->begin(), this->end(), _predicate)) {
			return true;
		}
		else {
			return false;
		}
	}

	T GetFirstWith(const std::function<bool(const T&)>& _predicate)
	{
		auto it = std::find_if(this->begin(), this->end(), _predicate);
		if (it != this->end()) {
			return *it;
		}
		return nullptr;
	}
    
	USList<T> GetAllWith(const std::function<bool(const T&)>& _predicate)
	{
		std::vector<T> result;
		for (const T& element : *this) {
			if (_predicate(element)) {
				result.push_back(element);
			}
		}
		return result;
	}

	void AppendVector(const std::vector<T>& _list)
	{
		this->insert(this->end(), _list.begin(), _list.end());
	}

};
