#include "DefaultSceneLayer.h"

// GLM math library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/gtx/common.hpp> // for fmod (floating modulus)

#include <filesystem>

// Graphics
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/VertexArrayObject.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Textures/TextureCube.h"
#include "Graphics/VertexTypes.h"
#include "Graphics/Font.h"
#include "Graphics/GuiBatcher.h"
#include "Graphics/Framebuffer.h"

// Utilities
#include "Utils/MeshBuilder.h"
#include "Utils/MeshFactory.h"
#include "Utils/ObjLoader.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/FileHelpers.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/StringUtils.h"
#include "Utils/GlmDefines.h"

// Gameplay
#include "Gameplay/Material.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"

// Components
#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Components/Camera.h"
#include "Gameplay/Components/RotatingBehaviour.h"
#include "Gameplay/Components/JumpBehaviour.h"
#include "Gameplay/Components/RenderComponent.h"
#include "Gameplay/Components/MaterialSwapBehaviour.h"
#include "Gameplay/Components/TriggerVolumeEnterBehaviour.h"
#include "Gameplay/Components/SimpleCameraControl.h"
#include "Gameplay/Components/CharacterMovement.h"


// Physics
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Physics/Colliders/BoxCollider.h"
#include "Gameplay/Physics/Colliders/PlaneCollider.h"
#include "Gameplay/Physics/Colliders/SphereCollider.h"
#include "Gameplay/Physics/Colliders/ConvexMeshCollider.h"
#include "Gameplay/Physics/Colliders/CylinderCollider.h"
#include "Gameplay/Physics/TriggerVolume.h"
#include "Graphics/DebugDraw.h"

// GUI
#include "Gameplay/Components/GUI/RectTransform.h"
#include "Gameplay/Components/GUI/GuiPanel.h"
#include "Gameplay/Components/GUI/GuiText.h"
#include "Gameplay/InputEngine.h"

#include "Application/Application.h"
#include "Gameplay/Components/ParticleSystem.h"
#include "Graphics/Textures/Texture3D.h"
#include "Graphics/Textures/Texture1D.h"


DefaultSceneLayer::DefaultSceneLayer() :
	ApplicationLayer()
{
	Name = "Default Scene";
	Overrides = AppLayerFunctions::OnAppLoad;
}

DefaultSceneLayer::~DefaultSceneLayer() = default;

void DefaultSceneLayer::OnAppLoad(const nlohmann::json& config) {
	_CreateScene();
}

void DefaultSceneLayer::_CreateScene()
{
	using namespace Gameplay;
	using namespace Gameplay::Physics;

	Application& app = Application::Get();

	bool loadScene = false;
	// For now we can use a toggle to generate our scene vs load from file
	if (loadScene && std::filesystem::exists("scene.json")) {
		app.LoadScene("scene.json");
	} else {

		ShaderProgram::Sptr reflectiveShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_environment_reflective.glsl" }
		});
		reflectiveShader->SetDebugName("Reflective");

		// This shader handles our basic materials without reflections (cause they expensive)
		ShaderProgram::Sptr basicShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_blinn_phong_textured.glsl" }
		});
		basicShader->SetDebugName("Blinn-phong");

		ShaderProgram::Sptr AShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_ambient.glsl" }
		});
		AShader->SetDebugName("Ambience");

		ShaderProgram::Sptr DShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{ 
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_diffuse.glsl" }
		});
		
		DShader->SetDebugName("Diffuse"); 

		// This shader handles our basic materials without reflections (cause they expensive)
		ShaderProgram::Sptr specShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/textured_specular.glsl" }
		});


		specShader->SetDebugName("Textured-Specular");

		// This shader handles our foliage vertex shader example
		ShaderProgram::Sptr foliageShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/foliage.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/screendoor_transparency.glsl" }
		});
		foliageShader->SetDebugName("Foliage");

		// This shader handles our cel shading example
		ShaderProgram::Sptr toonShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/toon_shading.glsl" }
		});
		toonShader->SetDebugName("Toon Shader");

		// This shader handles our displacement mapping example
		ShaderProgram::Sptr displacementShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/displacement_mapping.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_tangentspace_normal_maps.glsl" }
		});
		displacementShader->SetDebugName("Displacement Mapping");

		// This shader handles our tangent space normal mapping
		ShaderProgram::Sptr tangentSpaceMapping = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_tangentspace_normal_maps.glsl" }
		});
		tangentSpaceMapping->SetDebugName("Tangent Space Mapping");

		// This shader handles our multitexturing example
		ShaderProgram::Sptr multiTextureShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/vert_multitextured.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_multitextured.glsl" }
		});
		multiTextureShader->SetDebugName("Multitexturing");

		// Load in the meshes
		MeshResource::Sptr monkeyMesh = ResourceManager::CreateAsset<MeshResource>("Monkey.obj");
		
		//Objects for my scene
		//Used in GDW Game
		MeshResource::Sptr CharacterMesh = ResourceManager::CreateAsset<MeshResource>("CharacterFinal.obj");
		MeshResource::Sptr magemesh = ResourceManager::CreateAsset<MeshResource>("MageEnemy.obj");
		MeshResource::Sptr wallMesh = ResourceManager::CreateAsset<MeshResource>("Wall.obj");
		MeshResource::Sptr WallGrateMesh = ResourceManager::CreateAsset<MeshResource>("WallGrate.obj");
		MeshResource::Sptr SwordMesh = ResourceManager::CreateAsset<MeshResource>("Sword.obj");
		MeshResource::Sptr RockMesh = ResourceManager::CreateAsset<MeshResource>("Rock.obj");
		MeshResource::Sptr spikeMesh = ResourceManager::CreateAsset<MeshResource>("SpikeTrap.obj");
		MeshResource::Sptr LeverMesh = ResourceManager::CreateAsset<MeshResource>("Lever.obj");


		//Textures for my scene
		Texture2D::Sptr    characterTex = ResourceManager::CreateAsset<Texture2D>("textures/CharacterTexture.png");
		Texture2D::Sptr    mageTex = ResourceManager::CreateAsset<Texture2D>("textures/MageEnemy.png"); 
		Texture2D::Sptr    swordTex = ResourceManager::CreateAsset<Texture2D>("textures/SwordTexture.png");
		Texture2D::Sptr    wallTex = ResourceManager::CreateAsset<Texture2D>("textures/Wall.png");
		Texture2D::Sptr    rockTex = ResourceManager::CreateAsset<Texture2D>("textures/RockTexture.png");
		Texture2D::Sptr    grateTex = ResourceManager::CreateAsset<Texture2D>("textures/WallGrateUVS.png");
		Texture2D::Sptr    floorTex = ResourceManager::CreateAsset<Texture2D>("textures/StoneTexture.png");
		Texture2D::Sptr    spikeTex = ResourceManager::CreateAsset<Texture2D>("textures/SpikeTexture.png");
		Texture2D::Sptr    LeverTex = ResourceManager::CreateAsset<Texture2D>("textures/LeverTextures.png");

		// Loading in a 1D LUT
		Texture1D::Sptr toonLut = ResourceManager::CreateAsset<Texture1D>("luts/toon-1D.png"); 
		toonLut->SetWrap(WrapMode::ClampToEdge);

		// Here we'll load in the cubemap, as well as a special shader to handle drawing the skybox
		TextureCube::Sptr testCubemap = ResourceManager::CreateAsset<TextureCube>("cubemaps/ocean/ocean.jpg");
		ShaderProgram::Sptr      skyboxShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/skybox_vert.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/skybox_frag.glsl" }
		});

		// Create an empty scene
		Scene::Sptr scene = std::make_shared<Scene>(); 

		// Setting up our enviroment map
		scene->SetSkyboxTexture(testCubemap); 
		scene->SetSkyboxShader(skyboxShader);
		// Since the skybox I used was for Y-up, we need to rotate it 90 deg around the X-axis to convert it to z-up 
		scene->SetSkyboxRotation(glm::rotate(MAT4_IDENTITY, glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)));

		// Loading My Color LookUp Tables created in Photoshop
		Texture3D::Sptr CoolLut = ResourceManager::CreateAsset<Texture3D>("luts/Cool.CUBE");   
		Texture3D::Sptr WarmLut = ResourceManager::CreateAsset<Texture3D>("luts/Warm.CUBE");  
		Texture3D::Sptr CustomLut = ResourceManager::CreateAsset<Texture3D>("luts/CustomFix.CUBE");
	
		Texture2D::Sptr displacementMap = ResourceManager::CreateAsset<Texture2D>("textures/displacement_map.png"); 
		Texture2D::Sptr normalMap = ResourceManager::CreateAsset<Texture2D>("textures/normal_map.png"); 
		Texture2D::Sptr diffuseMap = ResourceManager::CreateAsset<Texture2D>("textures/bricks_diffuse.png"); 

		ComponentManager::RegisterType<CharacterMovement>();


		scene->SetColorLUT(CustomLut);

		// Create our materials
		// This will be our box material, with no environment reflections

		Material::Sptr stoneMat = ResourceManager::CreateAsset<Material>(basicShader); //Blinn-phong Shader
		{
			stoneMat->Name = "Box";
			stoneMat->Set("u_Material.Diffuse", floorTex);
			stoneMat->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr characterMat = ResourceManager::CreateAsset<Material>(basicShader);     
		{
			characterMat->Name = "Character"; 
			characterMat->Set("u_Material.Diffuse", characterTex);
			characterMat->Set("u_Material.Shininess", 0.3f); 

		}

		Material::Sptr mageMat = ResourceManager::CreateAsset<Material>(AShader); //Ambience Shader
		{
			mageMat->Name = "M";
			mageMat->Set("u_Material.Diffuse", mageTex);
			mageMat->Set("u_Material.Shininess", 0.3f);
		}

		Material::Sptr wallMat = ResourceManager::CreateAsset<Material>(basicShader); 
		{
			wallMat->Name = "W";
			wallMat->Set("u_Material.Diffuse", wallTex);
			wallMat->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr grateMat = ResourceManager::CreateAsset<Material>(specShader); //Specular Shader
		{
			grateMat->Name = "G";
			grateMat->Set("u_Material.Diffuse", grateTex);
			grateMat->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr swordMat = ResourceManager::CreateAsset<Material>(basicShader); 
		{
			swordMat->Name = "Sword";
			swordMat->Set("u_Material.Diffuse", swordTex);
			swordMat->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr spikeMat = ResourceManager::CreateAsset<Material>(reflectiveShader); //Reflective Metal Shader
		{
			spikeMat->Name = "spike";
			spikeMat->Set("u_Material.Diffuse", spikeTex); 
			spikeMat->Set("u_Material.Shininess", 0.5f);
		}

		Material::Sptr leverMat = ResourceManager::CreateAsset<Material>(specShader); //Specular Shader
		{
			leverMat->Name = "G";
			leverMat->Set("u_Material.Diffuse", LeverTex);
			leverMat->Set("u_Material.Shininess", 0.1f); 
		}
	
		Material::Sptr rockMat = ResourceManager::CreateAsset<Material>(toonShader); //Toon Shader 
		{
			rockMat->Name = "Toon";
			rockMat->Set("u_Material.Diffuse", rockTex);
			rockMat->Set("s_ToonTerm", toonLut);
			rockMat->Set("u_Material.Shininess", 0.1f);
			rockMat->Set("u_Material.Steps", 8);
		}

		// Create some lights for our scene
		scene->Lights.resize(3);
		scene->Lights[0].Position = glm::vec3(0.0f, 1.0f, 3.0f);
		scene->Lights[0].Color = glm::vec3(1.0f, 1.0f, 1.0f);
		scene->Lights[0].Range = 2000.0f;

		scene->Lights[1].Position = glm::vec3(1.0f, 0.0f, 3.0f);
		scene->Lights[1].Color = glm::vec3(0.2f, 0.8f, 0.1f);

		scene->Lights[2].Position = glm::vec3(0.0f, 1.0f, 3.0f);
		scene->Lights[2].Color = glm::vec3(1.0f, 0.2f, 0.1f);

		// Tileable Plane
		MeshResource::Sptr planeMesh = ResourceManager::CreateAsset<MeshResource>(); 
		planeMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(1.0f)));
		planeMesh->GenerateMesh();

		MeshResource::Sptr sphere = ResourceManager::CreateAsset<MeshResource>();
		sphere->AddParam(MeshBuilderParam::CreateIcoSphere(ZERO, ONE, 5));
		sphere->GenerateMesh();

		// Set up the scene's camera
		GameObject::Sptr camera = scene->MainCamera->GetGameObject()->SelfRef();
		{
			camera->SetPostion({ -8, 0, 10 });
			camera->SetRotation({ 45, -6, -90 });
			camera->LookAt(glm::vec3(0.0f));

			camera->Add<SimpleCameraControl>();

			// This is now handled by scene itself!
			//Camera::Sptr cam = camera->Add<Camera>();
			// Make sure that the camera is set as the scene's main camera!
			//scene->MainCamera = cam;
		}

		// Set up all our sample objects
		GameObject::Sptr plane = scene->CreateGameObject("Plane");
		{
			// Make a big tiled mesh
			MeshResource::Sptr tiledMesh = ResourceManager::CreateAsset<MeshResource>();
			tiledMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(60.0f), glm::vec2(20.0f)));
			tiledMesh->GenerateMesh();

			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = plane->Add<RenderComponent>();
			renderer->SetMesh(tiledMesh);
			renderer->SetMaterial(stoneMat);

			// Attach a plane collider that extends infinitely along the X/Y axis
			RigidBody::Sptr physics = plane->Add<RigidBody>(/*static by default*/);
			physics->AddCollider(BoxCollider::Create(glm::vec3(50.0f, 50.0f, 1.0f)))->SetPosition({ 0,0,-1 });
		}

		GameObject::Sptr character = scene->CreateGameObject("Character");
		{
			character->SetPostion(glm::vec3(-5.0f, 0.0f, 0.0f));
			character->SetRotation(glm::vec3(-90.0f, 180.0f, 180.0f));
			character->SetScale(glm::vec3(0.2));

			// Character Jumping
			character->Add<JumpBehaviour>();

			//Character Movemennt
			character->Add<CharacterMovement>();
			
			RenderComponent::Sptr renderer = character->Add<RenderComponent>();
			renderer->SetMesh(CharacterMesh);
			renderer->SetMaterial(characterMat);

			RigidBody::Sptr physics = character->Add<RigidBody>(RigidBodyType::Dynamic);
			ICollider::Sptr charactercollider = physics->AddCollider(ConvexMeshCollider::Create());
			charactercollider->SetScale(glm::vec3(0.2));  
		}

		GameObject::Sptr enemyMage = scene->CreateGameObject("Enemy");
		{
			enemyMage->SetPostion(glm::vec3(4.0f, 0.0f, 3.0f));
			enemyMage->SetRotation(glm::vec3(-90.0f, 180.0f, 0.0f));
			enemyMage->SetScale(glm::vec3(0.1)); 
;

			RenderComponent::Sptr renderer = enemyMage->Add<RenderComponent>();
			renderer->SetMesh(magemesh);
			renderer->SetMaterial(mageMat);

			// Example of a trigger that interacts with static and kinematic bodies as well as dynamic bodies
			RigidBody::Sptr physics = enemyMage->Add<RigidBody>(RigidBodyType::Static);
			ICollider::Sptr enemycollider = physics->AddCollider(ConvexMeshCollider::Create()); 
			enemycollider->SetScale(glm::vec3(0.1));
		}


		GameObject::Sptr rock = scene->CreateGameObject("Rock");
		{
			rock->SetPostion(glm::vec3(0.0f, 0.0f, 0.0f));
			rock->SetRotation(glm::vec3(-90.0f, 180.0f, 0.0f));
			rock->SetScale(glm::vec3(0.5));
			

			RenderComponent::Sptr renderer = rock->Add<RenderComponent>();
			renderer->SetMesh(RockMesh);
			renderer->SetMaterial(rockMat);

			// Example of a trigger that interacts with static and kinematic bodies as well as dynamic bodies
			RigidBody::Sptr physics = rock->Add<RigidBody>(RigidBodyType::Static);
			ICollider::Sptr rockcollider = physics->AddCollider(ConvexMeshCollider::Create());
			rockcollider->SetScale(glm::vec3(0.5));
		}
		
		GameObject::Sptr sword = scene->CreateGameObject("sword");
		{
			sword->SetPostion(glm::vec3(0.0f, -0.4f, 4.0f));
			sword->SetRotation(glm::vec3(80.0f, 180.0f, 0.0f));
			sword->SetScale(glm::vec3(0.3));


			RenderComponent::Sptr renderer = sword->Add<RenderComponent>(); 
			renderer->SetMesh(SwordMesh);
			renderer->SetMaterial(swordMat);

			RigidBody::Sptr physics = sword->Add<RigidBody>(RigidBodyType::Static); 
			ICollider::Sptr swordcollider = physics->AddCollider(ConvexMeshCollider::Create());
			swordcollider->SetScale(glm::vec3(0.3));
		}
		
		GameObject::Sptr grate = scene->CreateGameObject("Grate");
		{
			grate->SetPostion(glm::vec3(-7.0f, 0.0f, 0.0f));
			grate->SetRotation(glm::vec3(-90.0f, -180.0f, 180.0f));
			grate->SetScale(glm::vec3(0.5));


			RenderComponent::Sptr renderer = grate->Add<RenderComponent>();
			renderer->SetMesh(WallGrateMesh);
			renderer->SetMaterial(grateMat);

			RigidBody::Sptr physics = grate->Add<RigidBody>(RigidBodyType::Static);
			ICollider::Sptr gratecollider = physics->AddCollider(ConvexMeshCollider::Create());
			gratecollider->SetScale(glm::vec3(0.5));
		}

		GameObject::Sptr lever = scene->CreateGameObject("Lever");
		{
			lever->SetPostion(glm::vec3(-4.0f, -5.0f, 0.0f));
			lever->SetRotation(glm::vec3(-90.0f, -180.0f, 90.0f));
			lever->SetScale(glm::vec3(0.5));


			RenderComponent::Sptr renderer = lever->Add<RenderComponent>();
			renderer->SetMesh(LeverMesh);
			renderer->SetMaterial(leverMat);

			RigidBody::Sptr physics = lever->Add<RigidBody>(RigidBodyType::Static);
			ICollider::Sptr levercollider = physics->AddCollider(ConvexMeshCollider::Create());
			levercollider->SetScale(glm::vec3(0.5)); 
		}

		GameObject::Sptr spike1 = scene->CreateGameObject("spike");
		{
			spike1->SetPostion(glm::vec3(4.0f, -6.0f, 0.0f)); 
			spike1->SetRotation(glm::vec3(-90.0f, -180.0f, 90.0f)); 
			spike1->SetScale(glm::vec3(0.5)); 
			 

			RenderComponent::Sptr renderer = spike1->Add<RenderComponent>();
			renderer->SetMesh(spikeMesh);
			renderer->SetMaterial(spikeMat); 

			RigidBody::Sptr physics = spike1->Add<RigidBody>(RigidBodyType::Static); 
			ICollider::Sptr spikecollider = physics->AddCollider(ConvexMeshCollider::Create());
			spikecollider->SetScale(glm::vec3(0.5));
		}

		GameObject::Sptr spike2 = scene->CreateGameObject("spike 2");
		{
			spike2->SetPostion(glm::vec3(1.0f, 6.0f, 0.0f));
			spike2->SetRotation(glm::vec3(-90.0f, -180.0f, 90.0f));
			spike2->SetScale(glm::vec3(0.5));


			RenderComponent::Sptr renderer = spike2->Add<RenderComponent>();
			renderer->SetMesh(spikeMesh);
			renderer->SetMaterial(spikeMat);

			RigidBody::Sptr physics = spike2->Add<RigidBody>(RigidBodyType::Static);
			ICollider::Sptr spike2collider = physics->AddCollider(ConvexMeshCollider::Create());
			spike2collider->SetScale(glm::vec3(0.5));
		}

		GameObject::Sptr wall1 = scene->CreateGameObject("Wall 1");
		{
			wall1->SetPostion(glm::vec3(-7.0f, 3.5f, 0.0f));
			wall1->SetRotation(glm::vec3(-90.0f, -180.0f, 180.0f));
			wall1->SetScale(glm::vec3(0.8));

			RenderComponent::Sptr renderer = wall1->Add<RenderComponent>();
			renderer->SetMesh(wallMesh);
			renderer->SetMaterial(wallMat);
		}

		GameObject::Sptr wall2 = scene->CreateGameObject("Wall 2");
		{
			wall2->SetPostion(glm::vec3(-7.0f, -11.5f, 0.0f));
			wall2->SetRotation(glm::vec3(-90.0f, -180.0f, 180.0f));
			wall2->SetScale(glm::vec3(0.8));

			RenderComponent::Sptr renderer = wall2->Add<RenderComponent>();
			renderer->SetMesh(wallMesh);
			renderer->SetMaterial(wallMat);

		}

		GameObject::Sptr wall3 = scene->CreateGameObject("Wall 3");
		{
			wall3->SetPostion(glm::vec3(-6.5f, -11.5f, 0.0f));
			wall3->SetRotation(glm::vec3(-90.0f, -180.0f, 90.0f));
			wall3->SetScale(glm::vec3(0.8));

			RenderComponent::Sptr renderer = wall3->Add<RenderComponent>();
			renderer->SetMesh(wallMesh);
			renderer->SetMaterial(wallMat);

		}

		GameObject::Sptr wall4 = scene->CreateGameObject("Wall 4");
		{
			wall4->SetPostion(glm::vec3(-6.5f, 11.5f, 0.0f));
			wall4->SetRotation(glm::vec3(-90.0f, -180.0f, 90.0f));
			wall4->SetScale(glm::vec3(0.8));

			RenderComponent::Sptr renderer = wall4->Add<RenderComponent>();
			renderer->SetMesh(wallMesh);
			renderer->SetMaterial(wallMat);

		}

		GameObject::Sptr wall5 = scene->CreateGameObject("Wall 5");
		{
			wall5->SetPostion(glm::vec3(3, 11.5f, 0.0f));
			wall5->SetRotation(glm::vec3(-90.0f, -180.0f, 90.0f));
			wall5->SetScale(glm::vec3(0.8));

			RenderComponent::Sptr renderer = wall5->Add<RenderComponent>();
			renderer->SetMesh(wallMesh);
			renderer->SetMaterial(wallMat);

		}

		GameObject::Sptr wall6 = scene->CreateGameObject("Wall 6");
		{
			wall6->SetPostion(glm::vec3(3, -11.5f, 0.0f));
			wall6->SetRotation(glm::vec3(-90.0f, -180.0f, 90.0f));
			wall6->SetScale(glm::vec3(0.8));

			RenderComponent::Sptr renderer = wall6->Add<RenderComponent>();
			renderer->SetMesh(wallMesh);
			renderer->SetMaterial(wallMat);

		}

		GameObject::Sptr wall7 = scene->CreateGameObject("Wall 7");
		{
			wall7->SetPostion(glm::vec3(12.0f, 3.0f, 0.0f));
			wall7->SetRotation(glm::vec3(-90.0f, -180.0f, 180.0f));
			wall7->SetScale(glm::vec3(0.8));

			RenderComponent::Sptr renderer = wall7->Add<RenderComponent>();
			renderer->SetMesh(wallMesh);
			renderer->SetMaterial(wallMat);
		}

		GameObject::Sptr wall8 = scene->CreateGameObject("Wall 8");
		{
			wall8->SetPostion(glm::vec3(12.0f, -11.0f, 0.0f));
			wall8->SetRotation(glm::vec3(-90.0f, -180.0f, 180.0f));
			wall8->SetScale(glm::vec3(0.8));

			RenderComponent::Sptr renderer = wall8->Add<RenderComponent>();
			renderer->SetMesh(wallMesh);
			renderer->SetMaterial(wallMat);

		}



		//GameObject::Sptr particles = scene->CreateGameObject("Particles");
		//{
		//	ParticleSystem::Sptr particleManager = particles->Add<ParticleSystem>();  
		//	particleManager->AddEmitter(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 10.0f), 10.0f, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)); 
		//}

		GuiBatcher::SetDefaultTexture(ResourceManager::CreateAsset<Texture2D>("textures/ui-sprite.png"));
		GuiBatcher::SetDefaultBorderRadius(8);

		// Save the asset manifest for all the resources we just loaded
		ResourceManager::SaveManifest("scene-manifest.json");
		// Save the scene to a JSON file
		scene->Save("scene.json");

		// Send the scene to the application
		app.LoadScene(scene);
	}
}
