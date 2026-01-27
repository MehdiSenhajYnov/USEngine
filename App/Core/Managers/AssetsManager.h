// ============================================================================
// AssetsManager.h - Gestionnaire centralisé des ressources (Singleton)
// ============================================================================
// L'AssetsManager centralise le chargement et le stockage des ressources
// graphiques. Il utilise le pattern Singleton pour garantir une instance
// unique accessible depuis n'importe où dans l'application.
//
// Ressources gérées :
//   1. Renderables (meshes) : combinaisons de vertex/index buffers
//   2. Textures : images chargées sur le GPU
//
// Utilisation typique :
//   // Chargement (au démarrage)
//   AssetsManager::GetInstance().LoadRenderable("Quad", {vb, uvb}, ib, 6);
//   AssetsManager::GetInstance().LoadTexture("AllMight", "assets/AllMight.jpg");
//
//   // Récupération (dans RenderComponent)
//   Renderable data;
//   AssetsManager::GetInstance().GetRenderable("Quad", data);
//
//   // Nettoyage (à la fin)
//   AssetsManager::GetInstance().Reset();
//
// Note : Les Renderables stockent des pointeurs bruts vers les buffers.
//        L'appelant doit s'assurer que les buffers restent valides.
//
// ============================================================================
#pragma once
#include <map>
#include <memory>
#include <string>

#include "../Components/Render/Renderable.h"
#include "core/assets/asset.h"
#include "core/gpu/image.h"

class AssetsManager {
public:
	// Accès à l'instance unique (pattern Singleton)
	static AssetsManager& GetInstance()
	{
		static AssetsManager instance;
		return instance;
	}

	// Libère toutes les textures chargées
	void Reset();

	// --- Gestion des Renderables (meshes) ---

	// Récupère un Renderable par son nom. Retourne false si non trouvé.
	bool GetRenderable(std::string RenderableName, Renderable& OutRenderable);

	// Enregistre un Renderable avec un nom donné
	void LoadRenderable(std::string RenderableName, const Renderable& Renderable);

	// Variante avec les buffers séparés
	void LoadRenderable(std::string RenderableName,
	                    USList<vde::core::gpu::Buffer*> AllVertexBuffers,
	                    vde::core::gpu::Buffer* IndexBuffer,
	                    int IndexCount);

	// --- Gestion des Textures ---

	// Récupère une texture par son nom. Retourne false si non trouvée.
	bool GetTexture(std::string TextureName, vde::core::assets::Asset<vde::core::gpu::Image>*& OutTexture);

	// Charge une texture depuis un fichier et l'enregistre sous un nom
	vde::core::assets::Asset<vde::core::gpu::Image>* LoadTexture(const std::string& TextureName,
	                                                              const std::string& TexturePath);

private:
	// Stockage des ressources (clé = nom, valeur = données)
	std::map<std::string, Renderable> LoadedRenderables;
	std::map<std::string, std::unique_ptr<vde::core::assets::Asset<vde::core::gpu::Image>>> LoadedTextures;

	// Constructeur privé (Singleton)
	AssetsManager() = default;
};
