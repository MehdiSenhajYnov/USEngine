// =============================================================================
// FRAGMENT SHADER - Exécuté une fois par pixel (fragment)
// =============================================================================
// Rôle : Déterminer la couleur finale de chaque pixel affiché à l'écran.
//
// Flux du rendu :
//   Vertex Shader (par sommet)
//       ↓
//   Rasterization (le GPU détermine quels pixels sont couverts par le triangle)
//       ↓
//   Fragment Shader (par pixel) ← ON EST ICI
//       ↓
//   Framebuffer (image finale affichée)
//
// Le fragment shader reçoit les données du vertex shader INTERPOLÉES :
// Si un pixel est au milieu d'un triangle, ses UVs seront la moyenne
// pondérée des UVs des 3 sommets du triangle.
// =============================================================================
#version 460

// -----------------------------------------------------------------------------
// ENTRÉES (in) - Données venant du Vertex Shader (interpolées)
// -----------------------------------------------------------------------------
// "layout(location = 0)" doit correspondre au "out" du vertex shader

layout(location = 0) in vec2 vTexCoord;  // Coordonnées UV interpolées pour ce pixel

// -----------------------------------------------------------------------------
// TEXTURE via DESCRIPTOR SET
// -----------------------------------------------------------------------------
// "set = 1, binding = 0" signifie :
//   - Descriptor Set numéro 1 (DescriptorSet dans RenderComponent)
//   - Binding numéro 0 dans ce set
//
// "sampler2D" = texture 2D + sampler (comment lire la texture : filtrage, wrap mode...)
//
// En C++ (RenderComponent::Load) :
//   DescriptorSet->Bind(0, *Texture);  // Lie la texture au binding 0 du set 1

layout(set = 1, binding = 0) uniform sampler2D uTexture;

// -----------------------------------------------------------------------------
// SORTIES (out) - Couleur finale du pixel
// -----------------------------------------------------------------------------
// vec4 = (Rouge, Vert, Bleu, Alpha)
// Valeurs de 0.0 à 1.0
// Ex: vec4(1.0, 0.0, 0.0, 1.0) = rouge opaque

layout(location = 0) out vec4 oFragColor;

// -----------------------------------------------------------------------------
// FONCTION PRINCIPALE - Exécutée pour chaque pixel
// -----------------------------------------------------------------------------
void main()
{
	// texture(sampler, uv) : lit la couleur de la texture aux coordonnées UV
	//
	// Comment ça marche :
	//   - vTexCoord.x (u) : position horizontale dans la texture (0 = gauche, 1 = droite)
	//   - vTexCoord.y (v) : position verticale dans la texture (0 = haut, 1 = bas)
	//   - Le sampler applique le filtrage (bilinéaire par défaut = lissage entre pixels)
	//
	// La couleur retournée est directement celle de la texture

	oFragColor = texture(uTexture, vTexCoord);

	// -------------------------------------------------------------------------
	// EXEMPLES DE MODIFICATIONS POSSIBLES :
	// -------------------------------------------------------------------------
	//
	// Teinte rouge :
	//   oFragColor = texture(uTexture, vTexCoord) * vec4(1.0, 0.5, 0.5, 1.0);
	//
	// Niveaux de gris :
	//   vec4 color = texture(uTexture, vTexCoord);
	//   float gray = (color.r + color.g + color.b) / 3.0;
	//   oFragColor = vec4(gray, gray, gray, color.a);
	//
	// Semi-transparent :
	//   oFragColor = texture(uTexture, vTexCoord);
	//   oFragColor.a = 0.5;
	//
	// Inverser les couleurs :
	//   vec4 color = texture(uTexture, vTexCoord);
	//   oFragColor = vec4(1.0 - color.rgb, color.a);
	//
	// -------------------------------------------------------------------------
}
