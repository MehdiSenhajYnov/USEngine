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

#include "Core/Components/Render/RenderComponent.h" 
#include "Core/GameObjects/RendererObject.h"
#include "Core/Managers/AssetsManager.h"
#include "Core/Scene/Scene.h"

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
	auto graphicsContext = std::make_shared<vde::core::GraphicsContext>(*window);

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

	AssetsManager::GetInstance().LoadRenderable("Quad", {vb.get(), uvb.get()}, ib.get(), 6);
	AssetsManager::GetInstance().LoadTexture("AllMight", "assets/AllMight.jpg");
	
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

	
	// ------------------------------------------------------------------------
	// PHASE 4.5 : Création de la scène et des GameObjects
	// ------------------------------------------------------------------------
	// Le système Scene/GameObject/Component suit une architecture ECS simplifiée :
	// - Scene : conteneur principal qui gère le cycle de vie des GameObjects
	// - GameObject : entité de base pouvant contenir plusieurs Components
	// - Component : comportement attachable (rendu, physique, etc.)
	//
	// Disposition des 4 quads en croix :
	//
	//              [GO1]           (y = +1)
	//                |
	//     [GO2] --- [O] --- [GO3]  (x = -1, 0, +1)
	//                |
	//              [GO4]           (y = -1)
	//
	USScene Scene;
	RendererObject* GameObject1 = Scene.CreateGameObject<RendererObject>();
	RendererObject* GameObject2 = Scene.CreateGameObject<RendererObject>();
	RendererObject* GameObject3 = Scene.CreateGameObject<RendererObject>();
	RendererObject* GameObject4 = Scene.CreateGameObject<RendererObject>();

	// Initialisation des RenderComponents avec le mesh "Quad" et la texture "AllMight"
	// Chaque GameObject a son propre RenderComponent avec sa matrice de transformation
	GameObject1->RenderComponent->Init(graphicsContext.get(), "Quad", "AllMight", pipeline.get());
	GameObject2->RenderComponent->Init(graphicsContext.get(), "Quad", "AllMight", pipeline.get());
	GameObject3->RenderComponent->Init(graphicsContext.get(), "Quad", "AllMight", pipeline.get());
	GameObject4->RenderComponent->Init(graphicsContext.get(), "Quad", "AllMight", pipeline.get());

	// Positionnement de chaque quad pour former une croix autour de l'origine
	GameObject1->RenderComponent->Translate({0.0f, 1.0f, 0.0f});   // Haut
	GameObject2->RenderComponent->Translate({-1.0f, 0.0f, 0.0f});  // Gauche
	GameObject3->RenderComponent->Translate({1.0f, 0.0f, 0.0f});   // Droite
	GameObject4->RenderComponent->Translate({0.0f, -1.0f, 0.0f});  // Bas
	
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
				// ================================================================
				// Configuration commune pour tous les objets de la scène
				// ================================================================

				// Envoi des matrices view/projection au vertex shader via push constants
				// Ces matrices sont partagées par tous les objets (même caméra)
				rendering->UpdatePushConstants(
					vde::core::gpu::EShaderStage::Vertex,
					cameraDataStore
				);

				// Configuration du viewport (zone de rendu sur l'écran)
				rendering->SetViewport(
					{ 0, 0 },                                  // Origine (coin haut-gauche)
					{ graphicsContext->Backbuffer().Size() }   // Taille (plein écran)
				);

				// TODO a changer 1/60 avec le vrai delta time
				Scene.Tick(1/60);
				Scene.Draw(*rendering);
				
				// ================================================================
				// Note : Instancing vs Multiple Draw Calls
				// ================================================================
				// Actuellement, chaque RenderComponent fait un draw call.
				// Pour un grand nombre d'objets identiques, l'instancing
				// (DrawIndexedInstanced) serait plus performant.
				// Avec 4 objets, l'impact est négligeable.
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
	// CRUCIAL : Le GPU peut encore utiliser les ressources même si le CPU
	// a fini de soumettre les commandes. WaitForIdle() garantit que tout
	// est terminé avant de détruire les ressources.
	graphicsContext->WaitForIdle();

	Scene.Reset();
	AssetsManager::GetInstance().Reset();
	// --- Buffers de géométrie ---
	ib.reset();   // Index buffer (indices des triangles)
	uvb.reset();  // UV buffer (coordonnées de texture)
	vb.reset();   // Vertex buffer (positions des sommets)

	// --- Pipeline et Shaders ---
	// Le pipeline dépend des shaders, donc on le détruit en premier
	pipeline.reset();  // Pipeline graphique (state machine Vulkan)
	fs.reset();        // Fragment shader
	vs.reset();        // Vertex shader

	// --- Interface utilisateur ---
	// ImGui doit être arrêté avant la destruction du contexte graphique
	vde::imgui::Shutdown();

	// --- Contexte graphique ---
	// Détruit TOUT Vulkan : device, swapchain, allocator, pools, etc.
	// Doit être détruit en dernier (après toutes les ressources GPU)
	graphicsContext.reset();

	// --- Fenêtre ---
	// La fenêtre peut être fermée en tout dernier
	window.reset();

	// ====================================================================
	// Fin propre du programme
	// ====================================================================
	return 0;
}

// ============================================================================
// Fin du fichier main.cpp
// ============================================================================
//
// ╔══════════════════════════════════════════════════════════════════════════╗
// ║                    RÉSUMÉ DU FLUX D'EXÉCUTION                            ║
// ╚══════════════════════════════════════════════════════════════════════════╝
//
// PHASE 1 : Initialisation du système
// ────────────────────────────────────
// - Création de la fenêtre (GLFW) 1600x900
// - Initialisation du contexte Vulkan (device, swapchain, pools)
// - Chargement des plugins (FileFormat_stbimage, FileFormat_obj)
// - Setup de l'interface ImGui
//
// PHASE 2 : Création des ressources GPU
// ──────────────────────────────────────
// - Vertex Buffer (positions 3D des sommets du quad)
// - UV Buffer (coordonnées de texture)
// - Index Buffer (6 indices pour 2 triangles)
// - Enregistrement dans l'AssetsManager comme "Quad"
// - Chargement de la texture "AllMight" (JPG via stb_image)
//
// PHASE 3 : Chargement des shaders
// ─────────────────────────────────
// - Vertex Shader (base.vert.spv) : transformations MVP
// - Fragment Shader (base.frag.spv) : échantillonnage de texture
//
// PHASE 4 : Création du pipeline graphique
// ─────────────────────────────────────────
// - Configuration du pipeline Vulkan
// - Layout des descriptor sets :
//   - Set 0, Binding 0 : Uniform buffer (matrice modèle)
//   - Set 1, Binding 0 : Sampler2D (texture)
//
// PHASE 4.5 : Création de la scène (architecture ECS)
// ────────────────────────────────────────────────────
// - USScene : conteneur principal des GameObjects
// - 4 RendererObjects créés, chacun avec un RenderComponent
// - Disposition en croix autour de l'origine :
//   - GO1 (y=+1), GO2 (x=-1), GO3 (x=+1), GO4 (y=-1)
// - Chaque RenderComponent possède :
//   - Sa propre matrice modèle (uniform buffer)
//   - Ses propres descriptor sets (texture + matrice)
//
// PHASE 5 : Configuration des matrices de caméra
// ───────────────────────────────────────────────
// - Matrice de projection (perspective 45 deg, aspect 16:9)
// - Matrice de vue (caméra orbitale autour de l'origine)
// - Stockées dans ShaderDataStore pour les push constants
//
// PHASE 6 : Boucle de rendu
// ─────────────────────────
// Pour chaque frame :
//   1. PollEvents() - traitement des événements fenêtre
//   2. Mise à jour de la caméra (rotation orbitale via lookAt)
//   3. Acquisition d'un command buffer
//   4. Enregistrement des commandes :
//      a. ClearImageColor (fond bleu foncé)
//      b. BeginRendering avec le pipeline
//      c. UpdatePushConstants (view/projection)
//      d. Scene.Tick() et Scene.Draw() -> dessine les 4 quads
//   5. Submit() + Present()
//   6. Rendu ImGui par-dessus
//
// PHASE 7 : Nettoyage (ordre LIFO)
// ─────────────────────────────────
// - WaitForIdle() - synchronisation GPU
// - Scene.Reset() - libère les ressources des GameObjects
// - AssetsManager.Reset() - libère les textures
// - Buffers de géométrie (ib, uvb, vb)
// - Pipeline et shaders
// - ImGui::Shutdown()
// - GraphicsContext (détruit Vulkan)
// - Window (ferme GLFW)
//
// ╔══════════════════════════════════════════════════════════════════════════╗
// ║                        ARCHITECTURE DU PROJET                            ║
// ╚══════════════════════════════════════════════════════════════════════════╝
//
// PATTERNS UTILISES
// -----------------
// - pImpl : Cache les détails Vulkan dans GraphicsContext::Impl
// - RAII : unique_ptr/shared_ptr pour la gestion mémoire automatique
// - ECS simplifié : Scene > GameObject > Component
// - Singleton : AssetsManager pour centraliser les ressources
// - Command Pattern : Enregistrement des commandes GPU
//
// SYSTEME DE RENDU
// ----------------
// - Frame Overlap : 2 frames en parallèle (GPU/CPU)
// - Double Buffering : Swapchain avec backbuffer/frontbuffer
// - Indexed Drawing : Réutilisation des sommets via index buffer
// - Push Constants : Matrices view/projection (rapide, partagées)
// - Uniform Buffers : Matrices modèle (par objet, via descriptor set 0)
// - Sampler2D : Textures (via descriptor set 1)
//
// PIPELINE MVP (Model-View-Projection)
// ------------------------------------
//   Vertex Shader :
//     gl_Position = projection * view * model * vec4(position, 1.0)
//
//   - model      : matrice de transformation de l'objet (uniform buffer)
//   - view       : position/orientation de la caméra (push constant)
//   - projection : perspective 3D (push constant)
//
// RESULTAT VISUEL
// ---------------
// 4 quads texturés disposés en croix, vus par une caméra orbitale :
//
//              [Quad]
//                |
//     [Quad] -- [O] -- [Quad]
//                |
//              [Quad]
//
// Tous les quads utilisent la même texture (AllMight.jpg).
// La caméra tourne autour de l'origine en cercle.
//
// ============================================================================
