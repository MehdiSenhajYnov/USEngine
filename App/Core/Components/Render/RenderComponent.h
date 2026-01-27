#pragma once
#include <memory>
#include <core/gpu/commandbuffer.h>
#include <core/gpu/descriptorset.h>
#include <core/assets/asset.h>

#include "Renderable.h"
#include "../Component.h"

namespace vde::core::gpu { class RenderingCommandEncoder; }


class USRenderComponent : public USComponent
{
public:
	USRenderComponent();
	~USRenderComponent() override;

	void Init(vde::core::GraphicsContext* GraphicsContextToUse, std::string DrawableNameToUse, std::string TexturePathToUse, vde::core::gpu::Pipeline* Pipeline);
	void Reset() override;
	void Tick(float deltaTime) override;

	void Draw(vde::core::gpu::RenderingCommandEncoder& renderingToUse) override;


	std::string DrawableName;
	vde::core::assets::Asset<vde::core::gpu::Image>* Texture = nullptr;
	std::string TextureName;
	vde::core::gpu::ShaderDataStore ModelStore;
	std::unique_ptr<vde::core::gpu::Buffer> ModelBuffer;
	std::unique_ptr<vde::core::gpu::DescriptorSet> DescriptorSet;
	std::unique_ptr<vde::core::gpu::DescriptorSet> DescriptorSetModel;
	
	void Load(vde::core::gpu::Pipeline* Pipeline, vde::core::GraphicsContext* GraphicsContext);
	void Translate(glm::vec3 ToTranslate);
	
	bool AlreadyReset = false;
};
