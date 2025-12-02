#pragma once
#include <memory>
#include <core/gpu/commandbuffer.h>
#include <core/gpu/descriptorset.h>
#include <core/assets/asset.h>
// ============================================================================
// Includes standard et librairies externes
// ============================================================================

namespace vde::core::gpu { class RenderingCommandEncoder; }

class RenderObject
{
public:
	RenderObject(vde::core::GraphicsContext* GraphicsContextToUse);
	~RenderObject();

	void Reset();

	void Draw(vde::core::gpu::RenderingCommandEncoder* rendering, vde::core::gpu::Buffer* vb, vde::core::gpu::Buffer* uvb, vde::core::gpu::Buffer* ib);

	std::unique_ptr<vde::core::assets::Asset<vde::core::gpu::Image>> Texture;
	std::string TexturePath;
	vde::core::GraphicsContext* GraphicsContext;
	vde::core::gpu::ShaderDataStore ModelStore;
	std::unique_ptr<vde::core::gpu::Buffer> ModelBuffer;
	std::unique_ptr<vde::core::gpu::DescriptorSet> DescriptorSet;
	std::unique_ptr<vde::core::gpu::DescriptorSet> DescriptorSetModel;

	void LoadTexture(std::string TexturePath);
	void Load(vde::core::gpu::Pipeline* Pipeline);
	void Translate(glm::vec3 ToTranslate);

};
