// ============================================================================
// Renderable.h - Structure de données pour un mesh dessinable
// ============================================================================
// Un Renderable regroupe toutes les données géométriques nécessaires
// pour dessiner un objet avec DrawIndexed() :
//
//   AllVertexBuffers : Liste des vertex buffers dans l'ordre du pipeline
//                      Exemple : {PositionBuffer, UVBuffer}
//                      L'ordre doit correspondre aux layout(location = N) du shader
//
//   IndexBuffer      : Buffer contenant les indices des triangles
//
//   IndexCount       : Nombre d'indices à dessiner (ex: 6 pour un quad = 2 triangles)
//
// Ces données sont stockées dans l'AssetsManager et référencées par nom.
//
// ============================================================================
#pragma once
#include "../../List.h"

namespace vde::core::gpu
{
	class Buffer;
}

struct Renderable {
	Renderable() = default;

	Renderable(const USList<vde::core::gpu::Buffer*>& all_vertex_buffers,
	           vde::core::gpu::Buffer* index_buffer,
	           int index_count)
		: AllVertexBuffers(all_vertex_buffers),
		  IndexBuffer(index_buffer),
		  IndexCount(index_count)
	{
	}

	// Vertex buffers dans l'ordre des locations du shader :
	// location 0 = positions (vec3), location 1 = UVs (vec2), etc.
	USList<vde::core::gpu::Buffer*> AllVertexBuffers;

	// Index buffer pour le dessin indexé (réutilisation des sommets)
	vde::core::gpu::Buffer* IndexBuffer;

	// Nombre d'indices à dessiner
	int IndexCount;
};
