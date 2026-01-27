// Descriptor Sets Vulkan :
//   Set 0, Binding 0 : Uniform Buffer (matrice modèle) -> DescriptorSetModel
//   Set 1, Binding 0 : Sampler2D (texture)             -> DescriptorSet
#pragma once
#include <memory>
#include <core/gpu/commandbuffer.h>
#include <core/gpu/descriptorset.h>
#include <core/assets/asset.h>

#include "Renderable.h"
#include "../Component.h"

class USGameObject;

namespace vde::core::gpu { class RenderingCommandEncoder; }

class USRenderComponent : public USComponent
{
public:
	USRenderComponent();
	~USRenderComponent() override;

	void Init(vde::core::GraphicsContext* GraphicsContextToUse,
	          std::string DrawableNameToUse,
	          std::string TexturePathToUse,
	          vde::core::gpu::Pipeline* Pipeline);

	void Reset() override;
	void Tick(float deltaTime) override;
	void Draw(vde::core::gpu::RenderingCommandEncoder& renderingToUse) override;

	std::string DrawableName;
	vde::core::assets::Asset<vde::core::gpu::Image>* Texture = nullptr;
	std::string TextureName;

	// Matrice de transformation (CPU -> GPU via Upload)
	vde::core::gpu::ShaderDataStore ModelStore;
	std::unique_ptr<vde::core::gpu::Buffer> ModelBuffer;

	std::unique_ptr<vde::core::gpu::DescriptorSet> DescriptorSet;      // Set 1 : texture
	std::unique_ptr<vde::core::gpu::DescriptorSet> DescriptorSetModel; // Set 0 : matrice

	void Load(vde::core::gpu::Pipeline* Pipeline, vde::core::GraphicsContext* GraphicsContext);

protected:
	bool AlreadyReset = false;
};
