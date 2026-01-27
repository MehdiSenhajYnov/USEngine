#pragma once
#include <map>
#include <memory>
#include <string>

#include "../Components/Render/Renderable.h"
#include "core/assets/asset.h"
#include "core/gpu/image.h"

class AssetsManager {
public:

	static AssetsManager& GetInstance()
	{
		static AssetsManager instance;
		return instance;
	}

	void Reset();
	
	bool GetRenderable(std::string RenderableName, Renderable& OutRenderable);
	void LoadRenderable(std::string RenderableName, const Renderable& Renderable);
	void LoadRenderable(std::string RenderableName, USList<vde::core::gpu::Buffer*> AllVertexBuffers,
		vde::core::gpu::Buffer* IndexBuffer, int IndexCount);


	bool GetTexture(std::string TextureName, vde::core::assets::Asset<vde::core::gpu::Image>*& OutTexture);
	vde::core::assets::Asset<vde::core::gpu::Image>* LoadTexture(const std::string& TextureName, const std::string& TexturePath);
	
private:
	std::map<std::string, Renderable> LoadedRenderables;
	std::map<std::string, std::unique_ptr<vde::core::assets::Asset<vde::core::gpu::Image>>> LoadedTextures;
	AssetsManager() = default;
	
};
