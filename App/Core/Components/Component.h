#pragma once
#include "../BaseObject.h"

class USGameObject;

class USComponent : public USBaseObject {
public:
	void SetOwner(USGameObject* OwnerToUse);
protected:
	USGameObject* Owner = nullptr;
};
