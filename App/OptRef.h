#pragma once
#include <optional>
#include <stdexcept>

template<typename T>
class OptRef
{
	std::optional<std::reference_wrapper<T>> Obj;
public:
	OptRef() : Obj(std::nullopt) {}
	OptRef(T& Ref) : Obj(Ref) {}
	bool HasValue() const {
		return Obj.has_value();
	}
	T& Get() {
		if (!Obj.has_value()) {
			throw std::runtime_error("OptRef: No value present.");
		}
		return Obj->get();
	}
	const T& Get() const {
		if (!Obj.has_value()) {
			throw std::runtime_error("OptRef: No value present.");
		}
		return Obj->get();
	}
	void Set(T& Ref) {
		Obj = Ref;
	}
	void Reset() {
		Obj = std::nullopt;
	}

	OptRef operator*() {
		if (HasValue())
		{
			return Get();
		}
		return nullptr;
	}
};

