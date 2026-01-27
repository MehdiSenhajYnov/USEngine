// ============================================================================
// AssetsManager.cpp - Implémentation du gestionnaire de ressources
// ============================================================================
#include "AssetsManager.h"

void AssetsManager::Reset()
{
	// Libère toutes les textures GPU
	// Les unique_ptr appellent automatiquement le destructeur de Asset<Image>
	for (auto& [Name, Texture] : LoadedTextures)
	{
		Texture.reset();
	}
	// Note : Les Renderables ne sont pas reset ici car ils ne possèdent pas
	// les buffers (pointeurs bruts). Les buffers sont gérés par main.cpp.
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

	// Charge l'image depuis le fichier via le système d'assets du moteur
	// FileAssetSource lit le fichier, puis Asset<Image> le décode et l'upload au GPU
	auto Texture = std::make_unique<ImageAsset>(
		std::make_unique<FileAssetSource>(TexturePath)
	);

	// Stocke et retourne un pointeur vers la texture
	LoadedTextures[TextureName] = std::move(Texture);
	return LoadedTextures[TextureName].get();
}
