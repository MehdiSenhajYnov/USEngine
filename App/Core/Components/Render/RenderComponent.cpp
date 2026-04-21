#include "RenderComponent.h"
#include "core/graphicscontext.h"
#include "core/gpu/descriptorpool.h"
#include <glm/ext/matrix_transform.hpp>
#include <core/gpu/commandbuffer.h>

#include "../TransformComponent.h"
#include "../../GameObjects/GameObject.h"
#include "../../Managers/AssetsManager.h"


USRenderComponent::USRenderComponent() : USComponent(), ModelStore()
{

}

USRenderComponent::~USRenderComponent()
{
	
}

void USRenderComponent::Init(vde::core::GraphicsContext* GraphicsContextToUse, std::string DrawableNameToUse, std::string TextureNameToUse, vde::core::gpu::Pipeline* Pipeline)
{
	DrawableName = DrawableNameToUse;
	TextureName = TextureNameToUse;
	if (!AssetsManager::GetInstance().GetTexture(TextureName, Texture))
	{
		throw std::runtime_error("USRenderComponent::Init: Texture " + TextureName + " not found in AssetsManager.");
	}
	Load(Pipeline, GraphicsContextToUse);
}

void USRenderComponent::Load(vde::core::gpu::Pipeline* Pipeline, vde::core::GraphicsContext* GraphicsContext)
{
	// ====================================================================
	// Configuration des Descriptor Sets pour les TEXTURES
	// ====================================================================
	// Les descriptor sets sont des "pointeurs" vers les ressources GPU
	// utilisées par les shaders. Ils permettent de changer de texture
	// sans recompiler le shader.
	//
	// Architecture des descriptor sets dans ce shader :
	// - Set 0 : Matrices modèle (uniform buffer)
	// - Set 1 : Textures (sampler2D)

	// Récupération du layout pour les textures (Set 1)
	
	vde::core::gpu::DescriptorSetLayout& DescriptorSetLayout = Pipeline->GetDescriptorSetLayout(1);
	DescriptorSet = GraphicsContext->DescriptorPool().Acquire(DescriptorSetLayout);
	DescriptorSet->Bind(0, Texture->Value()); // Binding 0 = uniform sampler2D uTexture

	// ====================================================================
	// Configuration des Descriptor Sets pour les MATRICES MODÈLE
	// ====================================================================
	// Chaque objet dessiné peut avoir sa propre matrice de transformation
	// (position, rotation, échelle). Ces matrices sont stockées dans des
	// uniform buffers et accessibles via le descriptor set 0.

	// Création d'un store pour la matrice modèle de l'objet
	// Ce store utilise la réflexion du shader pour connaître la structure

	ModelStore = vde::core::gpu::ShaderDataStore(
		Pipeline->GetDescriptorSetLayout(0).GetBinding<vde::core::gpu::ShaderDataType>(0)
	);

	// Création du buffer GPU pour stocker la matrice
	ModelBuffer = GraphicsContext->CreateBuffer(
		ModelStore,
		vde::core::gpu::EBufferUsageBits::UniformBuffer | vde::core::gpu::EBufferUsageBits::CanUpload
	);

	// Configuration de la matrice : Translation vers la gauche (-1 sur l'axe X)
	ModelStore["matrix"] = glm::mat4(1.0f);


	// --- Création des descriptor sets pour les matrices modèle ---
	// Descriptor Set Model 1 : lie le buffer de matrice 1 au shader
	DescriptorSetModel = GraphicsContext->DescriptorPool().Acquire(
		Pipeline->GetDescriptorSetLayout(0) // Set 0 = matrices
	);
	DescriptorSetModel->Bind(0, *ModelBuffer); // Binding 0 = uniform buffer
}

//void USRenderComponent::Translate(glm::vec3 ToTranslate)
//{
//	ModelStore["matrix"] = glm::translate(glm::mat4(1.0f), ToTranslate);
//}

void USRenderComponent::Reset()
{
	if (AlreadyReset) return;
	AlreadyReset = true;
	// ====================================================================
	// Destruction dans l'ORDRE INVERSE de la création
	// ====================================================================
	// Principe de RAII : Last In, First Out (LIFO)
	// Les ressources qui dépendent d'autres doivent être détruites en premier

	// Doivent être détruits avant les ressources qu'ils référencent
	DescriptorSet.reset();
	DescriptorSetModel.reset();
	ModelBuffer.reset();
}

void USRenderComponent::Tick(float deltaTime)
{
	// ModelStore["matrix"] = scaleMatrix * rotationMatrix * translationMatrix*
}

void USRenderComponent::Draw(vde::core::gpu::RenderingCommandEncoder& Rendering)
{
	ModelStore["matrix"] = Owner->Transform->GetWorldTransform();
	// Upload de la matrice vers le buffer GPU
	ModelBuffer->Upload(ModelStore);

	// ================================================================
	// DESSIN DU QUAD
	// ================================================================

	// Liaison des descriptor sets pour le premier objet :
	// - Set 0 : descriptorSetModel1 → matrice de translation
	// - Set 1 : descriptorSet → texture 

	Rendering.BindDescriptorSets(0, {
		DescriptorSetModel.get(),
		DescriptorSet.get()
		});

	// Commande de dessin pour le quad gauche
	// Utilise les mêmes vertex/index buffers, mais avec des descriptor sets différents
	Renderable RenderableData;
	bool result = AssetsManager::GetInstance().GetRenderable(DrawableName, RenderableData);
	if (!result)
	{
		throw std::runtime_error("USRenderComponent::Draw: Renderable " + DrawableName + " not found in AssetsManager.");
	}
	
	Rendering.DrawIndexed(
		RenderableData.AllVertexBuffers,	// Vertex buffers (positions + coordonnées UV)
		RenderableData.IndexBuffer,			// Index buffer (ordre des sommets)
		RenderableData.IndexCount			// 6 indices = 2 triangles = 1 quad
	);

}
