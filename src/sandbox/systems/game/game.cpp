#include "sandbox/systems/game/game.hpp"

namespace fs = std::filesystem;
namespace ast = rythe::core::assets;
namespace rythe::game
{
	void Game::setup()
	{
		log::info("Initializing Game system");
		bindEvent<key_input<inputmap::method::NUM1>, &Game::reloadShaders>();
		bindEvent<key_input<inputmap::method::ESCAPE>, &Game::toggleMouseCapture>();

		input::InputSystem::registerWindow(gfx::Renderer::RI->getGlfwWindow());
		gfx::gui_stage::addGuiRender<Game, &Game::guiRender>(this);

		ast::AssetCache<gfx::texture_source>::registerImporter<gfx::TextureImporter>();
		ast::AssetCache<gfx::mesh>::registerImporter<gfx::MeshImporter>();
		ast::AssetCache<gfx::shader_source>::registerImporter<gfx::ShaderImporter>();

		gfx::MaterialCache::loadMaterialFromFile("error", "resources/shaders/error.shader");

		ast::AssetCache<gfx::mesh>::loadAssets("resources/meshes/glb/", gfx::default_mesh_params);
		ast::AssetCache<gfx::texture_source>::loadAssets("resources/textures/", gfx::default_texture_import_params);
		ast::AssetCache<gfx::shader_source>::loadAssets("resources/shaders/", gfx::default_shader_params);
		gfx::ShaderCache::createShaders(ast::AssetCache<gfx::shader_source>::getAssets());
		gfx::ModelCache::loadModels(ast::AssetCache<gfx::mesh>::getAssets());

		gfx::TextureCache::createTexture2D("park", ast::AssetCache<gfx::texture_source>::getAsset("park"), gfx::texture_parameters
			{
				.format = gfx::FormatType::RGB,
				.usage = gfx::UsageType::IMMUTABLE
			});

		modelHandle = gfx::ModelCache::getModel("cube");

		mat = gfx::MaterialCache::loadMaterialFromFile("default", "resources/shaders/lit.shader");
		mat->texture0 = gfx::TextureCache::createTexture2D("container_diffuse", ast::AssetCache<gfx::texture_source>::getAsset("container_diffuse"));
		mat->texture1 = gfx::TextureCache::createTexture2D("container_specular", ast::AssetCache<gfx::texture_source>::getAsset("container_specular"));

		colorMat = gfx::MaterialCache::loadMaterialFromFile("color", "resources/shaders/color.shader");
		colorMat->shader->addBuffer(gfx::BufferCache::createConstantBuffer<math::vec4>("Color", 3, gfx::UsageType::STATICDRAW));
		math::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		colorMat->shader->setUniform("Color", &color);

		gfx::MaterialCache::loadMaterial("white", gfx::ShaderCache::getShader("white"));

		{
			auto& skyboxRenderer = registry->world.addComponent<gfx::skybox_renderer>();
			skyboxRenderer.skyboxTex = gfx::TextureCache::getTexture("park");
		}

		{
			auto ent = createEntity("Floor");
			auto& transf = ent.addComponent<core::transform>();
			transf.scale = math::vec3(10, .5f, 10);
			transf.position = math::vec3(0.0f, -4.0f, 0.0f);
			ent.addComponent<gfx::mesh_renderer>({ .material = mat, .model = gfx::ModelCache::getModel("cube"), .castShadows = false });
		}

		{
			cube = createEntity("Cube");
			cube.addComponent<core::examplecomp>({ .direction = 1, .angularSpeed = .0f });
			auto& transf = cube.addComponent<core::transform>();
			transf.scale = math::vec3::one;
			transf.position = math::vec3(0.0f, -1.0f, 0.f);
			cube.addComponent<gfx::mesh_renderer>({ .material = mat, .model = modelHandle });
		}

		{
			dirLight = createEntity("Light");
			auto& transf = dirLight.addComponent<core::transform>();
			transf.scale = math::vec3::one;
			transf.position = math::vec3::one;
			transf.rotation = math::toQuat(math::vec3(-45.0f, 45.0f, 0.0f));
			dirLight.addComponent<gfx::light>({ .type = gfx::LightType::DIRECTIONAL, .data.color = math::vec4(1.0f) });
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
		if (CollapsingHeader("DirectionalLight"))
		{
			static math::vec3 rot = math::toEuler(dirLight.getComponent<core::transform>().rotation);
			if (InputFloat3("Light Rotation", rot.data))
			{
				dirLight.getComponent<core::transform>().rotation = math::toQuat(rot);
			}

			ColorEdit4("Color Picker", dirLight.getComponent<gfx::light>().data.color.data);
		}

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
			ast::AssetCache<gfx::shader_source>::loadAssets("resources/shaders/", gfx::default_shader_params, true);
			gfx::ShaderCache::compileShaders();
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