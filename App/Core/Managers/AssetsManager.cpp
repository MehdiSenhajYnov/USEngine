#include "AssetsManager.h"

void AssetsManager::Reset()
{
	for (auto& [Name, Texture] : LoadedTextures)
	{
		Texture.reset();
	}
}

bool AssetsManager::GetRenderable(std::string RenderableName, Renderable& OutRenderable)
{
	if (LoadedRenderables.contains(RenderableName))
	{
		OutRenderable = LoadedRenderables[RenderableName];
		return true;
	}
	return false;
}

void AssetsManager::LoadRenderable(std::string RenderableName, const Renderable& RenderableToUse)
{
	LoadedRenderables[RenderableName] = {
		RenderableToUse.AllVertexBuffers,
		RenderableToUse.IndexBuffer,
		RenderableToUse.IndexCount
	};
}

void AssetsManager::LoadRenderable(std::string RenderableName,
                                   USList<vde::core::gpu::Buffer*> AllVertexBuffers,
                                   vde::core::gpu::Buffer* IndexBuffer,
                                   int IndexCount)
{
	LoadedRenderables[RenderableName] = {AllVertexBuffers, IndexBuffer, IndexCount};
}

bool AssetsManager::GetTexture(std::string TextureName,
                               vde::core::assets::Asset<vde::core::gpu::Image>*& OutTexture)
{
	if (LoadedTextures.contains(TextureName))
	{
		OutTexture = LoadedTextures[TextureName].get();
		return true;
	}
	return false;
}

vde::core::assets::Asset<vde::core::gpu::Image>* AssetsManager::LoadTexture(
	const std::string& TextureName,
	const std::string& TexturePath)
{
	using namespace vde::core::assets;
	using ImageAsset = Asset<vde::core::gpu::Image>;

	auto Texture = std::make_unique<ImageAsset>(
		std::make_unique<FileAssetSource>(TexturePath)
	);

	LoadedTextures[TextureName] = std::move(Texture);
	return LoadedTextures[TextureName].get();
}

vde::core::assets::Asset<vde::core::gpu::Image>* AssetsManager::StoreTexture(
	const std::string& TextureName,
	std::unique_ptr<vde::core::gpu::Image> Texture)
{
	using ImageAsset = vde::core::assets::Asset<vde::core::gpu::Image>;

	LoadedTextures[TextureName] = std::make_unique<ImageAsset>(std::move(Texture));
	return LoadedTextures[TextureName].get();
}
