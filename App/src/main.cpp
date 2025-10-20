// ============================================================================
// Includes standard et librairies externes
// ============================================================================
#include <iostream>
#include <array>

// Vulkan Bootstrap - Simplifie l'initialisation de Vulkan
#include <VkBootstrap.h>

// GLM - Bibliothèque mathématique pour les graphiques (vecteurs, matrices)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// ============================================================================
// Includes du moteur VDE
// ============================================================================
// Gestion de la fenêtre (GLFW)
#include <core/window.h>

// Contexte graphique Vulkan principal
#include <core/graphicscontext.h>

// Abstraction GPU : buffers, shaders, pipelines, commandes
#include <core/gpu/commandbuffer.h>
#include <core/gpu/commandpool.h>
#include <core/gpu/descriptorpool.h>
#include <core/gpu/shader.h>
#include <core/gpu/pipeline.h>

// Système de gestion des assets (textures, modèles, shaders)
#include <core/assets/asset.h>

// Interface utilisateur ImGui
#include <imgui/imgui_vde.h>

// ============================================================================
// Données de géométrie : Quad (carré) texturé
// ============================================================================

// Positions des 4 sommets du quad dans l'espace 3D
// Forme un carré centré sur l'origine, de taille 1x1
//
//   0 (haut gauche)      3 (haut droit)
//        +------------------+
//        |                  |
//        |                  |
//        |                  |
//        +------------------+
//   1 (bas gauche)       2 (bas droit)
std::array<glm::vec3, 4> positions = {
	glm::vec3 { -0.5f,  0.5f, 0.0f },  // Sommet 0 : Haut gauche
	glm::vec3 { -0.5f, -0.5f, 0.0f },  // Sommet 1 : Bas gauche
	glm::vec3 {  0.5f, -0.5f, 0.0f },  // Sommet 2 : Bas droit
	glm::vec3 {  0.5f,  0.5f, 0.0f }   // Sommet 3 : Haut droit
};

// Coordonnées de texture (UV) pour chaque sommet
// (0,0) = coin haut-gauche de la texture
// (1,1) = coin bas-droit de la texture
std::array<glm::vec2, 4> texCoords = {
	glm::vec2 { 0.0f, 0.0f },  // Sommet 0 : UV haut gauche
	glm::vec2 { 0.0f, 1.0f },  // Sommet 1 : UV bas gauche
	glm::vec2 { 1.0f, 1.0f },  // Sommet 2 : UV bas droit
	glm::vec2 { 1.0f, 0.0f }   // Sommet 3 : UV haut droit
};

// Indices pour former 2 triangles (6 indices = 2 triangles * 3 sommets)
// Triangle 1 : sommets 0, 1, 2
// Triangle 2 : sommets 0, 2, 3
std::array<uint32_t, 6> indices = { 0, 1, 2, 0, 2, 3 };

// ============================================================================
// Point d'entrée principal de l'application
// ============================================================================
int main(int argc, char** argv)
{
	// ------------------------------------------------------------------------
	// PHASE 1 : Initialisation du système
	// ------------------------------------------------------------------------

	// Création de la fenêtre de l'application
	// - Taille : 1600x900 pixels
	// - Titre : "Hello VDE"
	// - Mode plein écran : désactivé (false)
	auto window = std::make_unique<vde::core::Window>(
		vde::core::WindowDescriptor{ {1600, 900}, "Hello VDE", false }
	);

	// Création du contexte graphique Vulkan
	// Initialise : instance Vulkan, device, swapchain, command pools, etc.
	auto graphicsContext = std::make_unique<vde::core::GraphicsContext>(*window);

	// Configuration du système de plugins
	// Les plugins permettent de charger différents formats de fichiers
	// (textures JPG/PNG, modèles OBJ, etc.)
	vde::util::PluginRegistry::Global().Context().graphicsContext = graphicsContext.get();
	vde::util::PluginRegistry::Global().LoadAllFromDirectory("engine/plugins");

	// Initialisation de l'interface utilisateur ImGui
	// ImGui permet de créer des fenêtres, boutons, sliders, etc.
	vde::imgui::Initialize(*window, *graphicsContext);

	// ------------------------------------------------------------------------
	// PHASE 2 : Création des ressources GPU
	// ------------------------------------------------------------------------

	// Création du buffer de positions (Vertex Buffer)
	// Contient les coordonnées XYZ des 4 sommets du quad
	auto vb = graphicsContext->CreateBuffer<glm::vec3>(
		4, // Nombre de sommets
		vde::core::gpu::EBufferUsageBits::VertexBuffer | vde::core::gpu::EBufferUsageBits::CanUpload
	);
	vb->Upload(positions); // Upload des données CPU vers GPU

	// Création du buffer de coordonnées de texture (UV Buffer)
	// Contient les coordonnées UV pour mapper la texture sur le quad
	auto uvb = graphicsContext->CreateBuffer<glm::vec2>(
		4, // Nombre de coordonnées UV (une par sommet)
		vde::core::gpu::EBufferUsageBits::VertexBuffer | vde::core::gpu::EBufferUsageBits::CanUpload
	);
	uvb->Upload(texCoords); // Upload des UVs vers GPU

	// Création du buffer d'indices (Index Buffer)
	// Définit l'ordre dans lequel les sommets forment des triangles
	auto ib = graphicsContext->CreateBuffer<uint32_t>(
		6, // 6 indices pour 2 triangles (3 indices par triangle)
		vde::core::gpu::EBufferUsageBits::IndexBuffer | vde::core::gpu::EBufferUsageBits::CanUpload
	);
	ib->Upload(indices); // Upload des indices vers GPU

	// Chargement de la texture
	// Le système de plugins (FileFormat_stbimage) décode automatiquement le JPG
	// et crée une image GPU utilisable pour le rendu
	vde::core::assets::Asset<vde::core::gpu::Image> texture(
		std::make_unique<vde::core::assets::FileAssetSource>("assets/AllMight.jpg")
	);

	// ------------------------------------------------------------------------
	// PHASE 3 : Chargement et compilation des shaders
	// ------------------------------------------------------------------------

	// Chargement du Vertex Shader (traite chaque sommet)
	// Le fichier .spv est le shader compilé en SPIR-V (format binaire Vulkan)
	vde::core::assets::FileAssetSource vsSrc("shaders/base.vert.spv");
	auto vs = graphicsContext->CreateShader({ vsSrc.Data().data(), vsSrc.Data().size() });

	// Chargement du Fragment Shader (traite chaque pixel)
	// Applique la texture sur les pixels du quad
	vde::core::assets::FileAssetSource fsSrc("shaders/base.frag.spv");
	auto fs = graphicsContext->CreateShader({ fsSrc.Data().data(), fsSrc.Data().size() });

	// ------------------------------------------------------------------------
	// PHASE 4 : Création du pipeline graphique
	// ------------------------------------------------------------------------

	// Le pipeline définit comment les données sont traitées du début à la fin :
	// Vertex Shader → Rasterization → Fragment Shader → Framebuffer
	auto pipeline = graphicsContext->CreatePipeline(vde::core::gpu::Pipeline::GraphicsPipelineInfo{
		{ vs.get(), fs.get() },                // Shaders à utiliser
		{ &graphicsContext->Backbuffer() }     // Cible de rendu (écran)
	});

	// Configuration du descriptor set pour la texture
	// Les descriptors permettent aux shaders d'accéder aux ressources GPU
	// Ici, on lie la texture au binding 0 du shader fragment
	auto& descriptorSetLayout = pipeline->GetDescriptorSetLayout(0);
	auto descriptorSet = graphicsContext->DescriptorPool().Acquire(descriptorSetLayout);
	descriptorSet->Bind(0, texture.Value()); // Binding 0 = uniform sampler2D uTexture

	// ------------------------------------------------------------------------
	// PHASE 5 : Configuration des matrices de transformation
	// ------------------------------------------------------------------------

	// Création d'un store pour les push constants (données envoyées au shader)
	// Les push constants sont des petites données rapides à updater
	vde::core::gpu::ShaderDataStore cameraDataStore(vs->PushConstants());

	// Configuration de la matrice de projection (perspective 3D)
	// - FOV : 45 degrés
	// - Aspect ratio : 16:9
	// - Near plane : 0.1
	// - Far plane : 100.0
	cameraDataStore["projection"] = glm::perspective(
		glm::radians(45.0f), // Field of View
		16.0f / 9.0f,        // Aspect ratio
		0.1f,                // Near clipping plane
		100.0f               // Far clipping plane
	);

	// Variable de temps pour l'animation de la caméra
	float t = 0.0f;

	// ------------------------------------------------------------------------
	// PHASE 6 : Boucle de rendu principale
	// ------------------------------------------------------------------------
	// Cette boucle s'exécute à chaque frame jusqu'à la fermeture de la fenêtre
	do
	{
		// Traitement des événements fenêtre (clavier, souris, redimensionnement)
		window->PollEvents();

		// Début de frame ImGui (interface utilisateur)
		vde::imgui::BeginFrame();

		// ====================================================================
		// Mise à jour de la caméra (animation orbitale)
		// ====================================================================
		// Création d'une matrice de vue qui fait tourner la caméra
		// autour de l'origine en cercle
		cameraDataStore["view"] = glm::lookAt(
			glm::vec3(glm::cos(t), 0.5, glm::sin(t)) * 3.0f, // Position de la caméra (orbite)
			glm::vec3(0.0f),                                  // Point regardé (origine)
			glm::vec3(0.0f, 1.0f, 0.0f)                       // Vecteur "haut"
		);
		t += 1.0f / 60.0f; // Incrémentation du temps (FIXME: devrait utiliser deltaTime réel)

		// ====================================================================
		// Enregistrement des commandes de rendu
		// ====================================================================

		// Acquisition d'un command buffer depuis le pool
		// Les command buffers contiennent toutes les instructions GPU
		auto& cmdBuffer = graphicsContext->CommandPool().Acquire();

		// Début de l'enregistrement des commandes
		// Le CommandEncoder est un RAII wrapper qui finalise automatiquement
		if (std::unique_ptr<vde::core::gpu::CommandEncoder> encoder = cmdBuffer.Record(
			"Triangle", // Nom pour le debugging
			vde::core::gpu::ECommandBufferRecordType::OneTimeSubmit); encoder)
		{
			// Effacement du backbuffer avec une couleur de fond (bleu foncé)
			encoder->ClearImageColor(graphicsContext->Backbuffer(),
				{ 0.1f, 0.2f, 0.3f, 1.0f } // RGBA
			);

			// Début d'une passe de rendu
			if (auto rendering = encoder->BeginRendering(
				*pipeline,
				{ &graphicsContext->Backbuffer() }, // Color attachment
				nullptr); rendering)                // Depth attachment (none)
			{
				// Envoi des matrices view/projection au vertex shader
				rendering->UpdatePushConstants(
					vde::core::gpu::EShaderStage::Vertex,
					cameraDataStore
				);

				// Liaison du descriptor set contenant la texture
				rendering->BindDescriptorSets(0, { descriptorSet.get() });

				// Configuration du viewport (zone de rendu)
				rendering->SetViewport(
					{ 0, 0 },                                  // Origine
					{ graphicsContext->Backbuffer().Size() }   // Taille
				);

				// Commande de dessin final
				// DrawIndexed utilise l'index buffer pour dessiner 6 indices (2 triangles)
				rendering->DrawIndexed(
					{ vb.get(), uvb.get() }, // Vertex buffers (positions + UVs)
					ib.get(),                // Index buffer
					6                        // Nombre d'indices à dessiner
				);
			}
		} // Le CommandEncoder se ferme automatiquement ici

		// Soumission du command buffer à la queue GPU
		graphicsContext->Submit(cmdBuffer);

		// Libération du command buffer pour réutilisation future
		graphicsContext->CommandPool().Release(cmdBuffer);

		// ====================================================================
		// Interface utilisateur ImGui
		// ====================================================================
		if (ImGui::Begin("This is a VDE/ImGui window"))
		{
			if (ImGui::Button("Click me"))
				std::cout << "Pouet" << std::endl;
		}
		ImGui::End();

		// Rendu de l'interface ImGui par-dessus la scène 3D
		vde::imgui::EndFrame(*graphicsContext);

		// Présentation du backbuffer à l'écran (swap des buffers)
		graphicsContext->Present();

	} while (!window->ShouldClose()); // Continue tant que la fenêtre est ouverte

	// ------------------------------------------------------------------------
	// PHASE 7 : Nettoyage et libération des ressources
	// ------------------------------------------------------------------------

	// Attente que toutes les opérations GPU soient terminées
	// Crucial avant de détruire les ressources pour éviter les crashes
	graphicsContext->WaitForIdle();

	// Destruction des ressources dans l'ordre inverse de leur création
	// C'est important pour respecter les dépendances entre objets

	// Libération des resources de rendu
	descriptorSet.reset();  // Descriptor set (lie la texture au shader)
	ib.reset();             // Index buffer
	uvb.reset();            // UV buffer (coordonnées de texture)
	vb.reset();             // Vertex buffer (positions)

	// Libération du pipeline et des shaders
	pipeline.reset();       // Pipeline graphique
	fs.reset();             // Fragment shader
	vs.reset();             // Vertex shader

	// Arrêt de l'interface ImGui
	vde::imgui::Shutdown();

	// Libération de la texture
	texture.Reset();

	// Destruction du contexte graphique (détruit tout Vulkan)
	graphicsContext.reset();

	// Fermeture de la fenêtre
	window.reset();

	return 0; // Fin du programme
}

// ============================================================================
// Fin du fichier main.cpp
// ============================================================================
//
// Résumé du flux d'exécution :
// 1. Initialisation : Fenêtre + Contexte Vulkan + Plugins + ImGui
// 2. Création des ressources GPU : Buffers, Texture, Shaders, Pipeline
// 3. Configuration : Matrices de projection/vue, Descriptor sets
// 4. Boucle de rendu : Mise à jour caméra → Enregistrement commandes → Rendu
// 5. Nettoyage : Destruction de toutes les ressources dans le bon ordre
//
// Architecture :
// - Le moteur utilise le pattern pImpl pour cacher les détails Vulkan
// - Les ressources GPU sont gérées par des unique_ptr (RAII)
// - Les command buffers utilisent des encoders RAII pour l'enregistrement
// - La texture est chargée automatiquement via le système de plugins
// ============================================================================
