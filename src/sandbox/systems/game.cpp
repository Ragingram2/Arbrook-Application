#include "sandbox/systems/game.hpp"

namespace fs = std::filesystem;
namespace ast = rythe::core::assets;
namespace rythe::game
{
	void Game::setup()
	{
		log::info("Initializing Game system");
		bindEvent<key_input<inputmap::method::NUM1>, &Game::reloadShaders>();
		bindEvent<key_input<inputmap::method::ESCAPE>, &Game::toggleMouseCapture>();
		//bindEvent<moveInput, &Game::move>();
		//bindEvent<mouse_input, &Game::mouselook>();

		input::InputSystem::registerWindow(gfx::Renderer::RI->getGlfwWindow());
		gfx::gui_stage::addGuiRender<Game, &Game::guiRender>(this);

		ast::AssetCache<gfx::texture>::registerImporter<gfx::TextureImporter>();
		ast::AssetCache<gfx::mesh>::registerImporter<gfx::MeshImporter>();
		ast::AssetCache<gfx::shader>::registerImporter<gfx::ShaderImporter>();

		ast::AssetCache<gfx::mesh>::loadAssets("resources/meshes/glb/", gfx::default_mesh_params);
		ast::AssetCache<gfx::texture>::loadAssets("resources/textures/", gfx::default_texture_params);
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
		cube.addComponent<core::examplecomp>({ .direction = 1, .angularSpeed = .00f });
		auto& transf = cube.addComponent<core::transform>();
		transf.scale = math::vec3::one;
		transf.position = math::vec3(0.0f, -1.0f, 0.f);
		cube.addComponent<gfx::mesh_renderer>({ .material = mat, .model = modelHandle });

		{
			auto& skyboxRenderer = registry->world.addComponent<gfx::skybox_renderer>();
			skyboxRenderer.skyboxTex = ast::AssetCache<gfx::texture>::getAsset("park");
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

		{
			auto camera = createEntity("Camera");
			//camera.addComponent<gfx::light>({ .type = gfx::LightType::POINT, .data.color = math::vec4(1.0f,1.0f,1.0f,1.0f), .data.intensity = 1.0f, .data.range = 10.f });
			auto& camTransf = camera.addComponent<core::transform>();
			camTransf.position = math::vec3(0.0f, 0.0f, 0.0f);
			camTransf.rotation = math::quat(math::lookAt(math::vec3::zero, camTransf.forward(), camTransf.up()));
			camera.addComponent<gfx::camera>({ .farZ = 10000.f, .nearZ = 1.0f, .fov = 90.f });
			camera.addComponent<camera_settings>({ .mode = CameraControlMode::FreeLook,.speed = 25.0f, .sensitivity = .9f });
		}
	}

	void Game::update()
	{
		ZoneScopedN("Game Update");
	}

	void Game::guiRender()
	{
		using namespace ImGui;
		//ShowDemoWindow();
		Begin("Docking Window");
		{
			static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
			static ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}End();
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

		ImGui::End();
	}

	void Game::reloadShaders(key_input<inputmap::method::NUM1>& input)
	{
		if (input.isPressed())
		{
			//gfx::ShaderCache::reloadShaders();
		}
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