#pragma once
#include <map>
#include <memory>
#include <string>

#include "../Components/Render/Renderable.h"
#include "core/assets/asset.h"
#include "core/gpu/image.h"
#include "graphics/mesh.h"

// Singleton pour stocker les meshes et textures
class AssetsManager {
public:
	static AssetsManager& GetInstance()
	{
		static AssetsManager instance;
		return instance;
	}

	void Reset();

	bool GetRenderable(const std::string& RenderableName, Renderable& OutRenderable);
	void LoadRenderable(const std::string& RenderableName, const Renderable& Renderable);
	void LoadRenderable(const std::string& RenderableName,
	                    USList<vde::core::gpu::Buffer*> AllVertexBuffers,
	                    vde::core::gpu::Buffer* IndexBuffer,
	                    int IndexCount);

	bool GetTexture(const std::string& TextureName, vde::core::assets::Asset<vde::core::gpu::Image>*& OutTexture);
	vde::core::assets::Asset<vde::core::gpu::Image>* LoadTexture(const std::string& TextureName,
	                                                              const std::string& TexturePath);
	vde::core::assets::Asset<vde::core::gpu::Image>* StoreTexture(
		const std::string& TextureName,
		std::unique_ptr<vde::core::gpu::Image> Texture);

	bool GetMesh(const std::string& MeshName, vde::core::assets::Asset<vde::graphics::Mesh>*& OutMesh);
	vde::core::assets::Asset<vde::graphics::Mesh>* LoadMesh(const std::string& MeshName,
	                                                        const std::string& MeshPath);

private:
	std::map<std::string, Renderable> LoadedRenderables;
	std::map<std::string, std::unique_ptr<vde::core::assets::Asset<vde::core::gpu::Image>>> LoadedTextures;
	std::map<std::string, std::unique_ptr<vde::core::assets::Asset<vde::graphics::Mesh>>> LoadedMeshes;
	AssetsManager() = default;
};
