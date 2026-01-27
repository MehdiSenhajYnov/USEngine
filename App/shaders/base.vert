// =============================================================================
// VERTEX SHADER - Exécuté une fois par sommet (vertex)
// =============================================================================
// Rôle : Transformer les positions des sommets de l'espace local (objet)
//        vers l'espace écran (clip space) pour que le GPU sache où dessiner.
//
// Pipeline de transformation (dans l'ordre) :
//   Position locale (objet)
//       ↓ [Matrice Model] - place l'objet dans le monde (position, rotation, scale)
//   Position monde (world space)
//       ↓ [Matrice View] - positionne la caméra (où on regarde)
//   Position caméra (camera/view space)
//       ↓ [Matrice Projection] - applique la perspective 3D
//   Position écran (clip space) → gl_Position
//
// ATTENTION : En GLSL, on multiplie dans l'ordre INVERSE !
//   gl_Position = Projection * View * Model * position
//   (on lit de droite à gauche : position → model → view → projection)
// =============================================================================
#version 460

// -----------------------------------------------------------------------------
// ENTRÉES (in) - Données venant des Vertex Buffers (côté CPU)
// -----------------------------------------------------------------------------
// "layout(location = X)" doit correspondre à l'ordre des buffers dans le code C++
// Dans main.cpp : LoadRenderable("Quad", {vb, uvb}, ...)
//   → vb  = location 0 (positions)
//   → uvb = location 1 (UVs)

layout(location = 0) in vec3 iPosition;  // Position 3D du sommet (x, y, z)
layout(location = 1) in vec2 iTexCoord;  // Coordonnées de texture (u, v)

// -----------------------------------------------------------------------------
// SORTIES (out) - Données envoyées au Fragment Shader
// -----------------------------------------------------------------------------
// Le GPU interpole automatiquement ces valeurs entre les sommets
// Ex: un pixel au milieu d'un triangle aura les UVs moyennes des 3 sommets

layout(location = 0) out vec2 vTexCoord;

// -----------------------------------------------------------------------------
// PUSH CONSTANTS - Données rapides partagées par tous les objets
// -----------------------------------------------------------------------------
// Envoyées via : rendering->UpdatePushConstants(EShaderStage::Vertex, cameraDataStore)
// Avantage : très rapide à mettre à jour (pas besoin de descriptor set)
// Limite : petite taille (128-256 bytes selon le GPU)
//
// Ici on met view/projection car c'est la même caméra pour tous les objets

layout(push_constant) uniform constants
{
	mat4 view;        // Où est la caméra et où elle regarde
	mat4 projection;  // Perspective (FOV, near/far planes)
} PushConstants;

// -----------------------------------------------------------------------------
// UNIFORM BUFFER via DESCRIPTOR SET - Données par objet
// -----------------------------------------------------------------------------
// "set = 0, binding = 0" signifie :
//   - Descriptor Set numéro 0 (DescriptorSetModel dans RenderComponent)
//   - Binding numéro 0 dans ce set
//
// Chaque objet a son propre uniform buffer avec sa matrice modèle
// → permet de positionner chaque quad différemment

layout(set = 0, binding = 0) uniform ModelMatrix
{
	mat4 matrix;  // Position/rotation/scale de cet objet spécifique
} model;

// -----------------------------------------------------------------------------
// FONCTION PRINCIPALE - Exécutée pour chaque sommet
// -----------------------------------------------------------------------------
void main()
{
	// Transformation MVP complète
	// Ordre de lecture (droite → gauche) :
	//   1. iPosition : position locale du sommet
	//   2. model.matrix : place dans le monde
	//   3. PushConstants.view : vue depuis la caméra
	//   4. PushConstants.projection : perspective 3D
	//
	// vec4(iPosition, 1.0f) : le "1.0" est nécessaire pour les matrices 4x4
	// (1.0 = point dans l'espace, 0.0 = direction/vecteur)

	gl_Position = PushConstants.projection * PushConstants.view * model.matrix * vec4(iPosition, 1.0f);

	// Passe les UVs au fragment shader (interpolées automatiquement)
	vTexCoord = iTexCoord;
}
