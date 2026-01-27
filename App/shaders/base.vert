// ============================================================================
// base.vert - Vertex Shader principal du moteur VDE
// ============================================================================
// Ce shader transforme chaque sommet de l'espace local (objet) vers l'espace
// clip (écran) en appliquant les matrices Model-View-Projection (MVP).
//
// Pipeline de transformation :
//   Local Space  --[Model]-->  World Space
//   World Space  --[View]-->   Camera Space
//   Camera Space --[Projection]--> Clip Space
//
// ============================================================================
#version 460

// --- Entrées (depuis les Vertex Buffers) ---
layout(location = 0) in vec3 iPosition;  // Position 3D du sommet (Vertex Buffer 0)
layout(location = 1) in vec2 iTexCoord;  // Coordonnées UV (Vertex Buffer 1)

// --- Sorties (vers le Fragment Shader) ---
layout(location = 0) out vec2 vTexCoord; // UV interpolées pour chaque pixel

// --- Push Constants (rapides, partagées par tous les objets) ---
// Envoyées via UpdatePushConstants() dans la boucle de rendu
layout(push_constant) uniform constants
{
	mat4 view;        // Matrice de vue (position/orientation de la camera)
	mat4 projection;  // Matrice de projection (perspective 3D)
} PushConstants;

// --- Uniform Buffer (par objet, via Descriptor Set 0) ---
// Chaque objet a sa propre matrice modele dans un buffer separe
layout(set = 0, binding = 0) uniform ModelMatrix
{
	mat4 matrix;      // Matrice modele (position/rotation/echelle de l'objet)
} model;

void main()
{
	// Transformation MVP complete : P * V * M * position
	// L'ordre est inverse car on lit de droite a gauche en maths
	gl_Position = PushConstants.projection * PushConstants.view * model.matrix * vec4(iPosition, 1.0f);

	// Les UVs sont passees directement au fragment shader
	// Le GPU les interpolera automatiquement entre les sommets
	vTexCoord = iTexCoord;
}
