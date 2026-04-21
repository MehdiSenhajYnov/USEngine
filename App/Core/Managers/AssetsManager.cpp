#include "AssetsManager.h"

void AssetsManager::Reset()
{
	LoadedRenderables.clear();
	LoadedTextures.clear();
	LoadedMeshes.clear();
}

bool AssetsManager::GetRenderable(const std::string& RenderableName, Renderable& OutRenderable)
{
	if (LoadedRenderables.contains(RenderableName))
	{
		OutRenderable = LoadedRenderables[RenderableName];
		return true;
	}
	return false;
}

void AssetsManager::LoadRenderable(const std::string& RenderableName, const Renderable& RenderableToUse)
{
	LoadedRenderables[RenderableName] = {
		RenderableToUse.AllVertexBuffers,
		RenderableToUse.IndexBuffer,
		RenderableToUse.IndexCount
	};
}

void AssetsManager::LoadRenderable(const std::string& RenderableName,
                                   USList<vde::core::gpu::Buffer*> AllVertexBuffers,
                                   vde::core::gpu::Buffer* IndexBuffer,
                                   int IndexCount)
{
	LoadedRenderables[RenderableName] = {AllVertexBuffers, IndexBuffer, IndexCount};
}

bool AssetsManager::GetTexture(const std::string& TextureName,
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

bool AssetsManager::GetMesh(const std::string& MeshName,
                            vde::core::assets::Asset<vde::graphics::Mesh>*& OutMesh)
{
	if (LoadedMeshes.contains(MeshName))
	{
		OutMesh = LoadedMeshes[MeshName].get();
		return true;
	}
	return false;
}

vde::core::assets::Asset<vde::graphics::Mesh>* AssetsManager::LoadMesh(
	const std::string& MeshName,
	const std::string& MeshPath)
{
	using namespace vde::core::assets;
	using MeshAsset = Asset<vde::graphics::Mesh>;

	auto Mesh = std::make_unique<MeshAsset>(
		std::make_unique<FileAssetSource>(MeshPath)
	);

	LoadedMeshes[MeshName] = std::move(Mesh);
	return LoadedMeshes[MeshName].get();
}
