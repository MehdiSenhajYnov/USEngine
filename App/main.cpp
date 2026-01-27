#include <iostream>
#include <array>

#include <VkBootstrap.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <core/window.h>
#include <core/graphicscontext.h>
#include <core/gpu/commandbuffer.h>
#include <core/gpu/commandpool.h>
#include <core/gpu/descriptorpool.h>
#include <core/gpu/shader.h>
#include <core/gpu/pipeline.h>
#include <core/assets/asset.h>
#include <imgui/imgui_vde.h>

#include "OptRef.h"
#include "Core/Components/TransformComponent.h"
#include "Core/Components/Render/RenderComponent.h"
#include "Core/GameObjects/RendererObject.h"
#include "Core/Managers/AssetsManager.h"
#include "Core/Scene/Scene.h"

#include "Core/GameObjects/CameraObject.h"
#include "Core/Managers/CameraManager.h"
#include "Core/Managers/InputManager.h"

// Géométrie du quad (carré 1x1 centré sur l'origine)
//   0 (haut gauche)      3 (haut droit)
//        +------------------+
//        |                  |
//        +------------------+
//   1 (bas gauche)       2 (bas droit)
std::array<glm::vec3, 4> positions = {
	glm::vec3 { -0.5f,  0.5f, 0.0f },
	glm::vec3 { -0.5f, -0.5f, 0.0f },
	glm::vec3 {  0.5f, -0.5f, 0.0f },
	glm::vec3 {  0.5f,  0.5f, 0.0f }
};

std::array<glm::vec2, 4> texCoords = {
	glm::vec2 { 0.0f, 0.0f },
	glm::vec2 { 0.0f, 1.0f },
	glm::vec2 { 1.0f, 1.0f },
	glm::vec2 { 1.0f, 0.0f }
};

// 2 triangles : (0,1,2) et (0,2,3)
std::array<uint32_t, 6> indices = { 0, 1, 2, 0, 2, 3 };

int main(int argc, char** argv)
{
	// === INITIALISATION ===
	auto window = std::make_unique<vde::core::Window>(
		vde::core::WindowDescriptor{ {1600, 900}, "Hello VDE", false }
	);

	InputManager::GetInstance().Initialize(*window);

	// Contexte Vulkan (instance, device, swapchain, command pools)
	auto graphicsContext = std::make_shared<vde::core::GraphicsContext>(*window);

	// Plugins pour charger les formats de fichiers (JPG/PNG, OBJ)
	vde::util::PluginRegistry::Global().Context().graphicsContext = graphicsContext.get();
	vde::util::PluginRegistry::Global().LoadAllFromDirectory("engine/plugins");

	vde::imgui::Initialize(*window, *graphicsContext);

	// === RESSOURCES GPU ===
	// Vertex Buffer (positions) - location 0 dans le shader
	auto vb = graphicsContext->CreateBuffer<glm::vec3>(
		4, vde::core::gpu::EBufferUsageBits::VertexBuffer | vde::core::gpu::EBufferUsageBits::CanUpload
	);
	vb->Upload(positions);

	// UV Buffer - location 1 dans le shader
	auto uvb = graphicsContext->CreateBuffer<glm::vec2>(
		4, vde::core::gpu::EBufferUsageBits::VertexBuffer | vde::core::gpu::EBufferUsageBits::CanUpload
	);
	uvb->Upload(texCoords);

	// Index Buffer
	auto ib = graphicsContext->CreateBuffer<uint32_t>(
		6, vde::core::gpu::EBufferUsageBits::IndexBuffer | vde::core::gpu::EBufferUsageBits::CanUpload
	);
	ib->Upload(indices);

	AssetsManager::GetInstance().LoadRenderable("Quad", {vb.get(), uvb.get()}, ib.get(), 6);
	AssetsManager::GetInstance().LoadTexture("AllMight", "assets/AllMight.jpg");

	// === SHADERS ===
	// Fichiers .spv = SPIR-V (binaire Vulkan compilé avec glslc)
	vde::core::assets::FileAssetSource vsSrc("shaders/base.vert.spv");
	auto vs = graphicsContext->CreateShader({ vsSrc.Data().data(), vsSrc.Data().size() });

	vde::core::assets::FileAssetSource fsSrc("shaders/base.frag.spv");
	auto fs = graphicsContext->CreateShader({ fsSrc.Data().data(), fsSrc.Data().size() });

	// === PIPELINE ===
	// Vertex Shader → Rasterization → Fragment Shader → Backbuffer
	auto pipeline = graphicsContext->CreatePipeline(vde::core::gpu::Pipeline::GraphicsPipelineInfo{
		{ vs.get(), fs.get() },
		{ &graphicsContext->Backbuffer() }
	});

	OptRef<USGameObject> temp;

	// === SCENE ===
	// Disposition des 4 quads en croix :
	//              [GO1]           (y = +1)
	//                |
	//     [GO2] --- [O] --- [GO3]  (x = -1, 0, +1)
	//                |
	//              [GO4]           (y = -1)
	USScene Scene;
	USRendererObject* GameObject1 = Scene.CreateGameObject<USRendererObject>();
	USRendererObject* GameObject2 = Scene.CreateGameObject<USRendererObject>();
	USRendererObject* GameObject3 = Scene.CreateGameObject<USRendererObject>();
	USRendererObject* GameObject4 = Scene.CreateGameObject<USRendererObject>();

	GameObject1->RenderComponent->Init(graphicsContext.get(), "Quad", "AllMight", pipeline.get());
	GameObject2->RenderComponent->Init(graphicsContext.get(), "Quad", "AllMight", pipeline.get());
	GameObject3->RenderComponent->Init(graphicsContext.get(), "Quad", "AllMight", pipeline.get());
	GameObject4->RenderComponent->Init(graphicsContext.get(), "Quad", "AllMight", pipeline.get());

	CameraObject* CameraGO = Scene.CreateGameObject<CameraObject>();
	CameraGO->CameraComponent->SetMovable(true);
	CameraGO->CameraComponent->SetPosition({ 0.0f, 0.0f, -3.0f });

	// === Caméra TEST 2 ===
	CameraObject* CameraGO2 = Scene.CreateGameObject<CameraObject>();
	CameraGO2->CameraComponent->SetMovable(false);
	CameraGO2->CameraComponent->SetPosition({ 0.0f, 3.0f, -6.0f });
	CameraGO2->CameraComponent->SetRotation({ 0.0f, -20.0f }); 
	CameraManager::GetInstance().SetMainCamera(CameraGO->CameraComponent);

	static bool useCam1 = true;
	static bool cWasDown = false;

	// === Caméra TEST 2 ===

	// === CAMERA ===
	// Push constants = données rapides envoyées au shader (view/projection partagées)
	vde::core::gpu::ShaderDataStore cameraDataStore(vs->PushConstants());
	cameraDataStore["projection"] = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 100.0f);

	float t = 0.0f;

	using Clock = std::chrono::high_resolution_clock;
	auto lastTime = Clock::now();


	GameObject1->Transform->Translate({ 0.0f, 1.0f, 0.0f });
	GameObject2->Transform->Translate({ -1.0f, 0.0f, 0.0f });
	GameObject3->Transform->Translate({ 1.0f, 0.0f, 0.0f });
	GameObject4->Transform->Translate({ 0.0f, -1.0f, 0.0f });

	// === BOUCLE DE RENDU ===
	do
	{
		window->PollEvents();
		vde::imgui::BeginFrame();

		auto now = Clock::now();
		float deltaTime = std::chrono::duration<float>(now - lastTime).count();
		lastTime = now;

		InputManager::GetInstance().Update();
		if (InputManager::GetInstance().IsKeyDown(GLFW_KEY_ESCAPE))
		{
			window->RaiseShouldClose();
		}
		CameraManager::GetInstance().Update(deltaTime);

		// === Switch Caméra ===
		bool cDown = InputManager::GetInstance().IsKeyDown(GLFW_KEY_C);
		if (cDown && !cWasDown)
		{
			useCam1 = !useCam1;
			CameraManager::GetInstance().SetMainCamera(
				useCam1 ? CameraGO->CameraComponent : CameraGO2->CameraComponent
			);
		}
		cWasDown = cDown;
		// === Switch Caméra ===

		auto* cam = CameraManager::GetInstance().GetMainCamera();
		if (cam)
		{
			float aspect = static_cast<float>(graphicsContext->Backbuffer().Size().x) /
				static_cast<float>(graphicsContext->Backbuffer().Size().y);

			cameraDataStore["projection"] = cam->GetProjectionMatrix(aspect);
			cameraDataStore["view"] = cam->GetViewMatrix();

		}



		// Acquisition d'un command buffer pour enregistrer les commandes GPU
		auto& cmdBuffer = graphicsContext->CommandPool().Acquire();

		// CommandEncoder = RAII wrapper qui finalise automatiquement à la destruction
		if (std::unique_ptr<vde::core::gpu::CommandEncoder> encoder = cmdBuffer.Record(
			"Triangle", vde::core::gpu::ECommandBufferRecordType::OneTimeSubmit); encoder)
		{
			encoder->ClearImageColor(graphicsContext->Backbuffer(), { 0.1f, 0.2f, 0.3f, 1.0f });

			// Passe de rendu
			if (auto rendering = encoder->BeginRendering(
				*pipeline,
				{ &graphicsContext->Backbuffer() },
				nullptr); rendering)
			{
				// Push constants view/projection (partagées par tous les objets)
				rendering->UpdatePushConstants(vde::core::gpu::EShaderStage::Vertex, cameraDataStore);
				rendering->SetViewport({ 0, 0 }, { graphicsContext->Backbuffer().Size() });

				// TODO: remplacer 1/60 par le vrai deltaTime
				Scene.Tick(1/60);
				Scene.Draw(*rendering);
			}
		}

		graphicsContext->Submit(cmdBuffer);
		graphicsContext->CommandPool().Release(cmdBuffer);

		// ImGui
		if (ImGui::Begin("This is a VDE/ImGui window"))
		{
			if (ImGui::Button("Click me"))
				std::cout << "Pouet" << std::endl;
		}
		ImGui::End();

		vde::imgui::EndFrame(*graphicsContext);
		graphicsContext->Present();

	} while (!window->ShouldClose());

	// === NETTOYAGE ===
	// WaitForIdle() OBLIGATOIRE : le GPU peut encore utiliser les ressources
	// même si le CPU a fini de soumettre les commandes
	graphicsContext->WaitForIdle();

	// Ordre de destruction important (LIFO par rapport aux dépendances)
	Scene.Reset();
	AssetsManager::GetInstance().Reset();
	ib.reset();
	uvb.reset();
	vb.reset();
	pipeline.reset();
	fs.reset();
	vs.reset();
	vde::imgui::Shutdown();
	graphicsContext.reset();  // Détruit tout Vulkan (device, swapchain, pools...)
	window.reset();

	return 0;
}

// =============================================================================
// RÉSUMÉ VULKAN - Rappel rapide du flux de rendu
// =============================================================================
//
// DESCRIPTOR SETS (ressources liées aux shaders) :
//   Set 0, Binding 0 : Uniform Buffer (matrice modèle)  -> par objet
//   Set 1, Binding 0 : Sampler2D (texture)              -> par objet
//
// PUSH CONSTANTS (rapides, partagées) :
//   view       : matrice de la caméra
//   projection : matrice de perspective
//
// PIPELINE MVP :
//   gl_Position = projection * view * model * vec4(position, 1.0)
//
// FRAME :
//   1. Acquire command buffer
//   2. Record : Clear -> BeginRendering -> UpdatePushConstants -> Draw -> End
//   3. Submit command buffer
//   4. Present (swap buffers)
//
// NETTOYAGE :
//   WaitForIdle() -> Reset ressources -> Destroy dans l'ordre inverse
// =============================================================================
