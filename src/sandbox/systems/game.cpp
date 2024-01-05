#include "sandbox/systems/game.hpp"

namespace fs = std::filesystem;
namespace ast = rythe::core::assets;
namespace rythe::game
{
	void Game::setup()
	{
		log::info("Initializing Game system");
		bindEvent<key_input<inputmap::method::NUM1>, &Game::reloadShaders>();
		bindEvent<moveInput, &Game::move>();
		bindEvent<key_input<inputmap::method::ESCAPE>, &Game::toggleMouseCapture>();
		bindEvent<mouse_input, &Game::mouselook>();

		input::InputSystem::registerWindow(gfx::Renderer::RI->getGlfwWindow());

		gfx::gui_stage::addGuiRender<Game, &Game::guiRender>(this);

		ast::AssetCache<gfx::texture>::registerImporter<gfx::TextureImporter>();
		ast::AssetCache<gfx::mesh>::registerImporter<gfx::MeshImporter>();
		ast::AssetCache<gfx::shader>::registerImporter<gfx::ShaderImporter>();

		ast::AssetCache<gfx::mesh>::loadAssets("resources/meshes/glb/", gfx::default_mesh_params);
		ast::AssetCache<gfx::texture>::loadAssets("resources/textures/", gfx::default_params);
		ast::AssetCache<gfx::shader>::loadAssets("resources/shaders/", gfx::default_shader_params);
		gfx::ModelCache::loadModels(ast::AssetCache<gfx::mesh>::getAssets());

		ast::AssetCache<gfx::texture>::createAsset("park", "resources/textures/park.png", gfx::texture_parameters
			{
				.format = gfx::FormatType::RGB
			}, true);

		modelHandle = gfx::ModelCache::getModel("cube");

		mat = gfx::MaterialCache::loadMaterialFromFile("default", "resources/shaders/lit.shader");
		mat->diffuse = ast::AssetCache<gfx::texture>::getAsset("container_diffuse");
		mat->specular = ast::AssetCache<gfx::texture>::getAsset("container_specular");

		color = gfx::MaterialCache::loadMaterialFromFile("color", "resources/shaders/color.shader");

		cube = createEntity("Cube");
		cube.addComponent<core::examplecomp>({ .direction = 1, .angularSpeed = .02f });
		auto& transf = cube.addComponent<core::transform>();
		transf.scale = math::vec3::one;
		transf.position = math::vec3(0.0f, -1.0f, 0.f);
		cube.addComponent<gfx::mesh_renderer>({ .material = mat, .model = modelHandle });

		{
			auto& skyboxRenderer = registry->world.addComponent<gfx::skybox_renderer>();
			skyboxRenderer.skyboxTex = ast::AssetCache<gfx::texture>::getAsset("park");
		}

		//{
		//	auto ent = createEntity("Light");
		//	auto& transf = ent.addComponent<core::transform>();
		//	transf.scale = math::vec3::one;
		//	transf.position = math::vec3(0.0f, 0.0f, 0.0f);
		//	transf.rotation = math::toQuat(math::vec3(0, math::radians(45.0f), math::radians(-45.0f)));
		//	ent.addComponent<gfx::light>({ .type = gfx::LightType::DIRECTIONAL, .data.color = math::vec4(1.0f) });
		//}

		//{
		//	auto ent = createEntity("PointLight");
		//	ent.addComponent<core::transform>({ .scale = math::vec3(.1f, .1f, .1f), .position = math::vec3(0.0f,1.0f,0.0f) });
		//	ent.addComponent<gfx::light>({ .type = gfx::LightType::POINT, .data.color = math::vec4(1.0f,0.0f,0.0f,1.0f), .data.intensity = 1.0f, .data.range = 50.f });
		//	ent.addComponent<core::examplecomp>({ .direction = 1, .range = 10.0f, .speed = .02f });
		//	ent.addComponent<gfx::mesh_renderer>({ .material = mat, .model = gfx::ModelCache::getModel("icosphere") });
		//}

		//{
		//	auto ent = createEntity("PointLight2");
		//	ent.addComponent<core::transform>({ .scale = math::vec3(.1f, .1f, .1f), .position = math::vec3(0.0f,1.0f,0.0f) });
		//	ent.addComponent<gfx::light>({ .type = gfx::LightType::POINT, .data.color = math::vec4(0.0f,0.0f,1.0f,1.0f), .data.intensity = 1.0f, .data.range = 50.f });
		//	ent.addComponent<core::examplecomp>({ .direction = -1, .range = 10.0f, .speed = .02f });
		//	ent.addComponent<gfx::mesh_renderer>({ .material = mat, .model = gfx::ModelCache::getModel("icosphere") });
		//}

		camera = createEntity("Camera");
		camera.addComponent<gfx::light>({ .type = gfx::LightType::POINT, .data.color = math::vec4(1.0f,1.0f,1.0f,1.0f), .data.intensity = 1.0f, .data.range = 10.f });
		auto& camTransf = camera.addComponent<core::transform>();
		camTransf.position = math::vec3(0.0f, 0.0f, 0.0f);
		camTransf.rotation = math::quat(math::lookAt(math::vec3::zero, camTransf.forward(), camTransf.up()));
		camera.addComponent<gfx::camera>({ .farZ = 10000.f, .nearZ = 1.0f, .fov = 90.f });
	}

	void Game::update()
	{
		ZoneScopedN("Game Update");
		currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		auto& camTransf = camera.getComponent<core::transform>();

		camTransf.position += velocity;
		velocity = math::vec3::zero;

		camTransf.rotation = math::conjugate(math::toQuat(math::lookAt(math::vec3::zero, front, up)));
	}

	void Game::guiRender()
	{
		using namespace ImGui;
		ShowDemoWindow();
		Begin("Inspector");
		if (CollapsingHeader("MeshRenderer"))
		{
			{
				auto models = gfx::ModelCache::getModels();
				Text("Here is where you can select which model to render");
				static ast::asset_handle<gfx::model> currentSelected = modelHandle;
				if (BeginCombo("Model Dropdown", currentSelected->name.c_str()))
				{
					for (auto handle : models)
					{
						const bool is_selected = (currentSelected == handle);
						if (Selectable(handle->name.c_str(), is_selected))
							currentSelected = handle;

						if (is_selected)
						{
							SetItemDefaultFocus();
						}
					}
					setModel(currentSelected);
					EndCombo();
				}
			}

			{
				auto mats = gfx::MaterialCache::getMaterials();
				Text("Here is where you can select which material to use");
				static ast::asset_handle<gfx::material> currentSelected = mat;
				if (BeginCombo("Material Dropdown", currentSelected->name.c_str()))
				{
					for (auto handle : mats)
					{
						const bool is_selected = (currentSelected == handle);
						if (Selectable(handle->name.c_str(), is_selected))
							currentSelected = handle;

						if (is_selected)
						{
							SetItemDefaultFocus();
						}
					}
					setMaterial(currentSelected);
					EndCombo();
				}
			}
		}

		//auto& cam = camera.getComponent<gfx::camera>();
		auto& transf = camera.getComponent<core::transform>();
		if (CollapsingHeader("DebugInfo", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
		{
			Text("Mouse Attributes");
			InputScalarN("Mouse Position", ImGuiDataType_Float, mousePos.data, 2, 0, 0, 0, ImGuiInputTextFlags_ReadOnly);

			Text("Camera Transform");
			InputScalarN("Position", ImGuiDataType_Float, transf.position.data, 3, 0, 0, 0, ImGuiInputTextFlags_ReadOnly);
			InputScalarN("Rotation", ImGuiDataType_Float, transf.rotation.data, 4, 0, 0, 0, ImGuiInputTextFlags_ReadOnly);
			InputScalarN("Scale", ImGuiDataType_Float, transf.scale.data, 3, 0, 0, 0, ImGuiInputTextFlags_ReadOnly);
		}

		ImGui::End();
	}

	void Game::reloadShaders(key_input<inputmap::method::NUM1>& input)
	{
		if (input.isPressed())
		{
			//gfx::ShaderCache::reloadShaders();
		}
	}

	void Game::move(moveInput& input)
	{
		auto& transf = camera.getComponent<core::transform>();

		auto leftRight = input.m_values["Left/Right"];
		auto forwardBackward = input.m_values["Forward/Backward"];
		auto upDown = input.m_values["Up/Down"];
		velocity += transf.right() * leftRight;
		velocity += transf.forward() * forwardBackward;
		velocity += transf.up() * upDown;
		if (velocity.length() > 0.00001f)
			velocity = math::normalize(velocity) * speed * deltaTime;
	}

	void Game::mouselook(mouse_input& input)
	{
		if (!input::InputSystem::mouseCaptured) return;

		static bool firstMouse = true;

		if (firstMouse)
		{
			lastMousePos = input.lastPosition;
			firstMouse = false;
		}

		mousePos = input.position;
		mouseDelta = input.positionDelta;
		lastMousePos = input.lastPosition;

		rotationDelta = math::vec2(mouseDelta.x * sensitivity, mouseDelta.y * sensitivity);

		pitch = math::clamp(pitch + rotationDelta.y, -89.99f, 89.99);
		yaw += -rotationDelta.x;

		front.x = cos(math::radians(yaw)) * cos(math::radians(pitch));
		front.y = sin(math::radians(pitch));
		front.z = sin(math::radians(yaw)) * cos(math::radians(pitch));
		front = math::normalize(front);
		right = math::normalize(math::cross(front, math::vec3::up));
		up = math::normalize(math::cross(right, front));
	}

	void Game::setModel(ast::asset_handle<gfx::model> handle)
	{
		auto& renderer = cube.getComponent<gfx::mesh_renderer>();
		if (renderer.model != handle)
		{
			renderer.model = handle;
			renderer.dirty = true;
		}
	}

	void Game::setMaterial(ast::asset_handle<gfx::material> handle)
	{
		auto& renderer = cube.getComponent<gfx::mesh_renderer>();
		if (renderer.material != handle)
		{
			renderer.material = handle;
			renderer.dirty = true;
		}
	}
}