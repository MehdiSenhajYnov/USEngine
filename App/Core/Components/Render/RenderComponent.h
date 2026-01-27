// ============================================================================
// RenderComponent.h - Composant de rendu pour les GameObjects
// ============================================================================
// USRenderComponent gère tout ce qui est nécessaire pour dessiner un objet :
//   - Référence vers le mesh (via AssetsManager)
//   - Référence vers la texture
//   - Matrice de transformation (uniform buffer)
//   - Descriptor sets pour lier les ressources au shader
//
// Architecture Vulkan des descriptor sets :
// ┌──────────────────────────────────────────────────────────────┐
// │ Set 0, Binding 0 : Uniform Buffer (matrice modèle 4x4)       │
// │   -> DescriptorSetModel                                      │
// ├──────────────────────────────────────────────────────────────┤
// │ Set 1, Binding 0 : Sampler2D (texture de l'objet)            │
// │   -> DescriptorSet                                           │
// └──────────────────────────────────────────────────────────────┘
//
// Flux de données vers le GPU :
//   1. ModelStore["matrix"] = glm::translate(...)  // CPU : mise à jour
//   2. ModelBuffer->Upload(ModelStore)              // CPU -> GPU
//   3. BindDescriptorSets(...)                      // Lie les ressources
//   4. DrawIndexed(...)                             // Commande de dessin
//
// ============================================================================
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

	// Initialise le composant avec les ressources nécessaires
	// DrawableNameToUse : nom du mesh dans AssetsManager (ex: "Quad")
	// TextureNameToUse  : nom de la texture dans AssetsManager (ex: "AllMight")
	void Init(vde::core::GraphicsContext* GraphicsContextToUse,
	          std::string DrawableNameToUse,
	          std::string TexturePathToUse,
	          vde::core::gpu::Pipeline* Pipeline);

	void Reset() override;
	void Tick(float deltaTime) override;
	void Draw(vde::core::gpu::RenderingCommandEncoder& renderingToUse) override;

	// --- Données de rendu ---
	std::string DrawableName;                                     // Nom du mesh
	vde::core::assets::Asset<vde::core::gpu::Image>* Texture = nullptr;  // Texture
	std::string TextureName;

	// --- Matrice de transformation ---
	vde::core::gpu::ShaderDataStore ModelStore;                   // Stockage CPU de la matrice
	std::unique_ptr<vde::core::gpu::Buffer> ModelBuffer;          // Buffer GPU (uniform)

	// --- Descriptor Sets ---
	std::unique_ptr<vde::core::gpu::DescriptorSet> DescriptorSet;      // Set 1 : texture
	std::unique_ptr<vde::core::gpu::DescriptorSet> DescriptorSetModel; // Set 0 : matrice

	// Crée les ressources GPU (buffers, descriptor sets)
	void Load(vde::core::gpu::Pipeline* Pipeline, vde::core::GraphicsContext* GraphicsContext);

	// Applique une translation à la matrice modèle
	void Translate(glm::vec3 ToTranslate);

private:
	bool AlreadyReset = false;  // Évite un double reset
};
