#pragma once

namespace vde::core::gpu { class RenderingCommandEncoder; }

class USBaseObject {
public:
	int Id = -1;
	virtual ~USBaseObject() = default;
	virtual void Tick(float deltaTime) {}
	virtual void Draw(vde::core::gpu::RenderingCommandEncoder& Rendering) {}
	// Reset() libère les ressources GPU (doit être appelé avant destruction)
	virtual void Reset() {}
};
