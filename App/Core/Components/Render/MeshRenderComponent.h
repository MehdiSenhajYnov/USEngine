#pragma once

#include <string>

#include <graphics/mesh.h>

#include "RenderComponent.h"

class USMeshRenderComponent : public USRenderComponent
{
public:
	void Init(vde::core::GraphicsContext* GraphicsContextToUse,
	          std::string MeshNameToUse,
	          std::string TextureNameToUse,
	          vde::core::gpu::Pipeline* Pipeline);

	void Draw(vde::core::gpu::RenderingCommandEncoder& renderingToUse) override;

	std::string MeshName;
	vde::core::assets::Asset<vde::graphics::Mesh>* Mesh = nullptr;
};
