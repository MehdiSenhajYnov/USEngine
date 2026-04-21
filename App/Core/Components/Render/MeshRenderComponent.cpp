#include "MeshRenderComponent.h"

#include <stdexcept>
#include <utility>
#include <vector>

#include "../TransformComponent.h"
#include "../../GameObjects/GameObject.h"
#include "../../Managers/AssetsManager.h"

namespace
{
std::vector<vde::core::gpu::Buffer*> GetMeshBuffersForBaseShader(const vde::graphics::Mesh& mesh)
{
	if (!mesh.vertexBuffers[0])
		throw std::runtime_error("USMeshRenderComponent::Draw: Mesh is missing position buffer at slot 0.");

	if (!mesh.vertexBuffers[2])
		throw std::runtime_error("USMeshRenderComponent::Draw: Mesh is missing UV buffer at slot 2.");

	return {
		mesh.vertexBuffers[0].get(),
		mesh.vertexBuffers[2].get()
	};
}

std::vector<std::pair<uint32_t, uint32_t>> GetEnabledSubmeshes(const vde::graphics::Mesh& mesh)
{
	std::vector<std::pair<uint32_t, uint32_t>> submeshes;
	submeshes.reserve(mesh.primitives.size());

	for (const auto& primitive : mesh.primitives)
	{
		if (primitive.enabled)
			submeshes.emplace_back(primitive.startIndex, primitive.indexCount);
	}

	return submeshes;
}
}

void USMeshRenderComponent::Init(vde::core::GraphicsContext* GraphicsContextToUse,
                                 std::string MeshNameToUse,
                                 std::string TextureNameToUse,
                                 vde::core::gpu::Pipeline* Pipeline)
{
	MeshName = std::move(MeshNameToUse);
	TextureName = std::move(TextureNameToUse);

	if (!AssetsManager::GetInstance().GetMesh(MeshName, Mesh))
	{
		throw std::runtime_error("USMeshRenderComponent::Init: Mesh " + MeshName + " not found in AssetsManager.");
	}

	if (!AssetsManager::GetInstance().GetTexture(TextureName, Texture))
	{
		throw std::runtime_error("USMeshRenderComponent::Init: Texture " + TextureName + " not found in AssetsManager.");
	}

	Load(Pipeline, GraphicsContextToUse);
}

void USMeshRenderComponent::Draw(vde::core::gpu::RenderingCommandEncoder& Rendering)
{
	if (!Mesh)
	{
		throw std::runtime_error("USMeshRenderComponent::Draw: Mesh asset is not initialized.");
	}

	ModelStore["matrix"] = Owner->Transform->GetWorldTransform();
	ModelBuffer->Upload(ModelStore);

	Rendering.BindDescriptorSets(0, {
		DescriptorSetModel.get(),
		DescriptorSet.get()
	});

	const vde::graphics::Mesh& mesh = Mesh->Value();
	if (!mesh.indexBuffer)
	{
		throw std::runtime_error("USMeshRenderComponent::Draw: Mesh is missing its index buffer.");
	}

	const std::vector<vde::core::gpu::Buffer*> meshBuffers = GetMeshBuffersForBaseShader(mesh);
	const std::vector<std::pair<uint32_t, uint32_t>> submeshes = GetEnabledSubmeshes(mesh);
	if (submeshes.empty())
		return;

	Rendering.DrawIndexedSubmeshes(meshBuffers, mesh.indexBuffer.get(), submeshes);
}
