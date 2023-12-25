#include "sandbox/systems/game.hpp"

namespace rythe::game
{
	void Game::setup()
	{
		log::debug("Initializing Game system");
		bindEvent<key_input<inputmap::method::NUM1>, &Game::reloadShaders>();
		bindEvent<moveInput, &Game::move>();
		bindEvent<key_input<inputmap::method::ESCAPE>, &Game::toggleMouseCapture>();
		bindEvent<mouse_input, &Game::mouselook>();

		input::InputSystem::registerWindow(gfx::Renderer::RI->getGlfwWindow());

		gfx::gui_stage::addGuiRender<Game, &Game::guiRender>(this);

		gfx::TextureCache::createTexture2D("park", "resources/textures/park.png",
			gfx::texture_parameters
			{
				.format = gfx::FormatType::RGB
			});
		gfx::ModelCache::loadModels("resources/meshes/glb/");
		gfx::TextureCache::loadTextures("resources/textures/");
		gfx::ShaderCache::loadShaders("resources/shaders/");
		modelHandle = gfx::ModelCache::getModel("cube");

		mat = gfx::MaterialCache::loadMaterialFromFile("default", "resources/shaders/lit.shader");
		mat.diffuse = gfx::TextureCache::getTexture2D("container_diffuse");
		mat.specular = gfx::TextureCache::getTexture2D("container_specular");

		color = gfx::MaterialCache::loadMaterialFromFile("color", "resources/shaders/color.shader");

		cube = createEntity("Cube");
		cube.addComponent<core::examplecomp>({ .direction = 1, .angularSpeed = .02f });
		auto& transf = cube.addComponent<core::transform>();
		transf.scale = math::vec3::one;
		transf.position = math::vec3(0.0f, -1.0f, 0.f);
		cube.addComponent<gfx::mesh_renderer>({ .material = mat, .model = modelHandle });

		{
			auto& skyboxRenderer = registry->world.addComponent<gfx::skybox_renderer>();
			skyboxRenderer.skyboxTex = gfx::TextureCache::getTexture2D("park");
		}

		{
			auto ent = createEntity("Light");
			auto& transf = ent.addComponent<core::transform>();
			transf.scale = math::vec3::one;
			transf.position = math::vec3(0.0f, 0.0f, 0.0f);
			transf.rotation = math::toQuat(math::vec3(0, math::radians(45.0f), math::radians(-45.0f)));
			ent.addComponent<gfx::light>({ .type = gfx::LightType::DIRECTIONAL, .data.color = math::vec4(1.0f) });
		}

		{
			auto ent = createEntity("PointLight");
			ent.addComponent<core::transform>({ .scale = math::vec3(.1f, .1f, .1f), .position = math::vec3(0.0f,1.0f,0.0f) });
			ent.addComponent<gfx::light>({ .type = gfx::LightType::POINT, .data.color = math::vec4(1.0f,0.0f,0.0f,1.0f), .data.intensity = 1.0f, .data.range = 50.f });
			ent.addComponent<core::examplecomp>({ .direction = 1, .range = 10.0f, .speed = .02f });
			ent.addComponent<gfx::mesh_renderer>({ .material = mat, .model = gfx::ModelCache::getModel("icosphere") });
		}

		{
			auto ent = createEntity("PointLight2");
			ent.addComponent<core::transform>({ .scale = math::vec3(.1f, .1f, .1f), .position = math::vec3(0.0f,1.0f,0.0f) });
			ent.addComponent<gfx::light>({ .type = gfx::LightType::POINT, .data.color = math::vec4(0.0f,0.0f,1.0f,1.0f), .data.intensity = 1.0f, .data.range = 50.f });
			ent.addComponent<core::examplecomp>({ .direction = -1, .range = 10.0f, .speed = .02f });
			ent.addComponent<gfx::mesh_renderer>({ .material = mat, .model = gfx::ModelCache::getModel("icosphere") });
		}

		camera = createEntity("Camera");
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
				//auto modelNames = gfx::ModelCache::getModelNamesC();
				Text("Here is where you can select which model to render");
				static gfx::model_handle currentSelected = modelHandle;
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
				//auto matNames = gfx::MaterialCache::getMaterialNamesC();
				Text("Here is where you can select which material to use");
				static gfx::material_handle currentSelected = mat;
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
			gfx::ShaderCache::reloadShaders();
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

	void Game::setModel(gfx::model_handle handle)
	{
		auto& renderer = cube.getComponent<gfx::mesh_renderer>();
		if (renderer.model != handle)
		{
			renderer.model = handle;
			renderer.dirty = true;
		}
	}

	void Game::setMaterial(gfx::material_handle handle)
	{
		auto& renderer = cube.getComponent<gfx::mesh_renderer>();
		if (renderer.material != handle)
		{
			renderer.material = handle;
			renderer.dirty = true;
		}
	}
}