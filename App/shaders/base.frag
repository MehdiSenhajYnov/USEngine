// ============================================================================
// base.frag - Fragment Shader principal du moteur VDE
// ============================================================================
// Ce shader est execute pour chaque pixel (fragment) genere par la rasterisation.
// Il echantillonne la texture aux coordonnees UV interpolees et retourne la
// couleur finale du pixel.
//
// Flux de donnees :
//   Vertex Shader -> Rasterizer -> Fragment Shader -> Framebuffer
//                   (interpole les UVs)  (calcule la couleur)
//
// ============================================================================
#version 460

// --- Entrees (depuis le Vertex Shader, interpolees par le rasterizer) ---
layout(location = 0) in vec2 vTexCoord;  // Coordonnees UV du pixel actuel

// --- Texture (via Descriptor Set 1) ---
// Lie par DescriptorSet->Bind(0, texture) dans RenderComponent
layout(set = 1, binding = 0) uniform sampler2D uTexture;

// --- Sortie (vers le framebuffer) ---
layout(location = 0) out vec4 oFragColor;  // Couleur RGBA finale du pixel

void main()
{
	// Echantillonne la texture aux coordonnees UV
	// texture() effectue le filtrage (bilineaire par defaut)
	oFragColor = texture(uTexture, vTexCoord);

	// Note : Pour ajouter des effets, on pourrait modifier oFragColor ici :
	// - Eclairage : oFragColor *= lightIntensity;
	// - Teinte    : oFragColor *= vec4(1.0, 0.8, 0.8, 1.0);
	// - Alpha     : oFragColor.a = 0.5; // Semi-transparent
}
