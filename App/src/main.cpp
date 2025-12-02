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
#include "../RenderObject.h"

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

	// ====================================================================
	// Chargement des textures
	// ====================================================================

	RenderObject RenderObject1(graphicsContext.get());
	RenderObject1.LoadTexture("assets/AllMight.jpg");

	RenderObject RenderObject2(graphicsContext.get());
	RenderObject2.LoadTexture("assets/AllMight.jpg");

	RenderObject RenderObject3(graphicsContext.get());
	RenderObject3.LoadTexture("assets/AllMight.jpg");

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

	RenderObject1.Load(pipeline.get());
	RenderObject2.Load(pipeline.get());
	RenderObject3.Load(pipeline.get());

	RenderObject1.Translate({0.0f, 1.0f, 0.0f});
	RenderObject2.Translate({-1.0f, 0.0f, 0.0f});
	RenderObject3.Translate({1.0f, 0.0f, 0.0f});

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


				RenderObject2.Draw(rendering.get(), vb.get(), uvb.get(), ib.get());
				RenderObject3.Draw(rendering.get(), vb.get(), uvb.get(), ib.get());
				RenderObject1.Draw(rendering.get(), vb.get(), uvb.get(), ib.get());


				// ================================================================
				// Note importante : Instancing vs Multiple Draw Calls
				// ================================================================
				// Ici on fait 2 draw calls distincts, ce qui n'est pas optimal.
				// Pour un grand nombre d'objets identiques, il vaudrait mieux
				// utiliser l'instancing (glDrawElementsInstanced en OpenGL).
				// Mais pour 2 objets, c'est négligeable.
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

	RenderObject1.Reset();
	RenderObject2.Reset();
	RenderObject3.Reset();

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
// • Création de la fenêtre (GLFW) 1600x900
// • Initialisation du contexte Vulkan (device, swapchain, pools)
// • Chargement des plugins (FileFormat_stbimage, FileFormat_obj)
// • Setup de l'interface ImGui
//
// PHASE 2 : Création des ressources GPU
// ──────────────────────────────────────
// • Vertex Buffer (positions 3D des sommets)
// • UV Buffer (coordonnées de texture)
// • Index Buffer (ordre de dessin des triangles)
// • Texture 1 : AllMight.jpg (image JPG décodée par stb_image)
// • Texture 2 : cage2.png (image PNG décodée par stb_image)
//
// PHASE 3 : Chargement des shaders
// ─────────────────────────────────
// • Vertex Shader (base.vert.spv) - Traite chaque sommet
// • Fragment Shader (base.frag.spv) - Applique les textures
//
// PHASE 4 : Création du pipeline graphique
// ─────────────────────────────────────────
// • Configuration du pipeline Vulkan
// • Setup des descriptor sets :
//   - Set 0 : Uniform buffers (matrices modèle)
//   - Set 1 : Samplers (textures)
// • Création des uniform buffers pour 2 matrices modèle :
//   - Objet 1 : Translation (-1, 0, 0) → Quad gauche
//   - Objet 2 : Translation (+1, 0, 0) → Quad droit
//
// PHASE 5 : Configuration des matrices de caméra
// ───────────────────────────────────────────────
// • Matrice de projection (perspective 45°, aspect 16:9)
// • Matrice de vue (caméra orbitale autour de l'origine)
//
// PHASE 6 : Boucle de rendu (60 FPS)
// ───────────────────────────────────
// Pour chaque frame :
//   1. Traiter les événements fenêtre (clavier, souris, fermeture)
//   2. Mettre à jour la caméra (rotation orbitale)
//   3. Acquérir un command buffer
//   4. Enregistrer les commandes de dessin :
//      a. Clear du backbuffer (couleur de fond)
//      b. Début de la passe de rendu
//      c. Update des push constants (view/projection)
//      d. Dessiner le quad gauche (texture AllMight)
//      e. Dessiner le quad droit (texture cage)
//      f. Fin de la passe de rendu
//   5. Soumettre le command buffer au GPU
//   6. Rendre l'interface ImGui
//   7. Présenter à l'écran (swap des buffers)
//
// PHASE 7 : Nettoyage (ordre LIFO)
// ─────────────────────────────────
// • WaitForIdle() - Attente de la fin des opérations GPU
// • Destruction des uniform buffers (matrices modèle)
// • Destruction des descriptor sets
// • Destruction des buffers de géométrie (vertex, UV, index)
// • Destruction du pipeline et des shaders
// • Arrêt d'ImGui
// • Destruction des textures
// • Destruction du contexte Vulkan
// • Fermeture de la fenêtre
//
// ╔══════════════════════════════════════════════════════════════════════════╗
// ║                        CONCEPTS CLÉS UTILISÉS                            ║
// ╚══════════════════════════════════════════════════════════════════════════╝
//
// 🏗️ ARCHITECTURE
// • Pattern pImpl : Cache les détails Vulkan dans GraphicsContext::Impl
// • RAII (Resource Acquisition Is Initialization) : unique_ptr partout
// • Command Pattern : Enregistrement des commandes GPU
// • Descriptor Sets : Liaison dynamique des ressources aux shaders
//
// 🎨 RENDU
// • Frame Overlap : 2 frames en parallèle pour maximiser les performances
// • Double Buffering : Swapchain avec backbuffer/frontbuffer
// • Indexed Drawing : Réutilisation des sommets via index buffer
// • Push Constants : Données légères envoyées rapidement au shader
// • Uniform Buffers : Données volumineuses (matrices) partagées
//
// 📐 MATHÉMATIQUES
// • Matrices de transformation (Model-View-Projection)
// • Translation : glm::translate() pour positionner les objets
// • Projection perspective : glm::perspective() pour la 3D
// • LookAt : glm::lookAt() pour positionner/orienter la caméra
//
// 🔧 VULKAN
// • Device : Représente la carte graphique
// • Swapchain : Gère l'affichage (double/triple buffering)
// • Command Buffers : Séquence de commandes GPU
// • Descriptor Sets : Lient les ressources (textures, buffers) aux shaders
// • Pipeline : Configuration complète du rendu (shaders, états, formats)
//
// 📊 RÉSULTAT VISUEL
// ──────────────────
// L'application affiche 2 quads texturés côte à côte :
// • Quad GAUCHE : Texture AllMight (JPG)
// • Quad DROITE : Texture cage (PNG)
// La caméra tourne autour des deux objets en cercle.
//
// ============================================================================
