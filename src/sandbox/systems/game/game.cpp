#include "sandbox/systems/game/game.hpp"

namespace fs = std::filesystem;
namespace ast = rythe::core::assets;
namespace rythe::game
{
	void Game::setup()
	{
		log::info("Initializing Game system");
		bindEvent<key_input<inputmap::method::F1>, &Game::reloadShaders>();
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
		mat->addTexture(gfx::TextureSlot::TEXTURE3,gfx::TextureCache::createTexture2D("container_diffuse", ast::AssetCache<gfx::texture_source>::getAsset("container_diffuse")));
		mat->addTexture(gfx::TextureSlot::TEXTURE4, gfx::TextureCache::createTexture2D("container_specular", ast::AssetCache<gfx::texture_source>::getAsset("container_specular")));

		colorMat = gfx::MaterialCache::loadMaterialFromFile("color", "resources/shaders/color.shader");
		colorMat->getShader()->addBuffer(gfx::BufferCache::createConstantBuffer<math::vec4>("Color", 3, gfx::UsageType::STATICDRAW));
		math::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		colorMat->getShader()->setUniform("Color", &color);

		gfx::MaterialCache::loadMaterial("white", gfx::ShaderCache::getShader("white"));

		{
			auto& skyboxRenderer = registry->world.addComponent<gfx::skybox_renderer>();
			skyboxRenderer.skyboxTex = gfx::TextureCache::getTexture("park");
		}

		{
			auto ent = createEntity("Floor");
			auto& transf = ent.addComponent<core::transform>();
			transf.scale = math::vec3(20, .5f, 20);
			transf.position = math::vec3(0.0f, -21.0f, 0.0f);
			ent.addComponent<gfx::mesh_renderer>({ .material = mat, .model = gfx::ModelCache::getModel("cube")});
		}

		//{
		//	auto ent = createEntity("Ceiling");
		//	auto& transf = ent.addComponent<core::transform>();
		//	transf.scale = math::vec3(20, .5f, 20);
		//	transf.position = math::vec3(0.0f, 21.0f, 0.0f);
		//	ent.addComponent<gfx::mesh_renderer>({ .material = mat, .model = gfx::ModelCache::getModel("cube")});
		//}

		{
			auto ent = createEntity("WallX+");
			auto& transf = ent.addComponent<core::transform>();
			transf.scale = math::vec3(.5f, 20.0f, 20.0f);
			transf.position = math::vec3(21.0f, 0.0f, 0.0f);
			ent.addComponent<gfx::mesh_renderer>({ .material = mat, .model = gfx::ModelCache::getModel("cube")});
		}

		//{
		//	auto ent = createEntity("WallX-");
		//	auto& transf = ent.addComponent<core::transform>();
		//	transf.scale = math::vec3(.5f, 20.0f, 20.0f);
		//	transf.position = math::vec3(-21.0f, 0.0f, 0.0f);
		//	ent.addComponent<gfx::mesh_renderer>({ .material = mat, .model = gfx::ModelCache::getModel("cube")});
		//}

		{
			auto ent = createEntity("WallZ+");
			auto& transf = ent.addComponent<core::transform>();
			transf.scale = math::vec3(20.0f, 20.0f, .5f);
			transf.position = math::vec3(0.0f, 0.0f, 21.0f);
			ent.addComponent<gfx::mesh_renderer>({ .material = mat, .model = gfx::ModelCache::getModel("cube") });
		}

		//{
		//	auto ent = createEntity("WallZ-");
		//	auto& transf = ent.addComponent<core::transform>();
		//	transf.scale = math::vec3(20.0f, 20.0f, .5f);
		//	transf.position = math::vec3(0.0f, 0.0f, -21.0f);
		//	ent.addComponent<gfx::mesh_renderer>({ .material = mat, .model = gfx::ModelCache::getModel("cube"), .castShadows = false });
		//}

		{
			cube = createEntity("Cube");
			auto& transf = cube.addComponent<core::transform>();
			transf.scale = math::vec3::one;
			transf.position = math::vec3(0.0f, 10.0f, 0.f);
			cube.addComponent<gfx::mesh_renderer>({ .material = mat, .model = modelHandle });
		}

		{
			auto ent = createEntity("Bunny");
			auto& transf = ent.addComponent<core::transform>();
			transf.scale = math::vec3::one * 3.0f;
			transf.position = math::vec3(10.0f, 0.0f, 10.0f);
			ent.addComponent<gfx::mesh_renderer>({ .material = mat, .model = gfx::ModelCache::getModel("bunny")});
		}

		{
			auto ent = createEntity("Suzanne");
			auto& transf = ent.addComponent<core::transform>();
			transf.scale = math::vec3::one * 3.0f;
			transf.position = math::vec3(-10.0f, 0.0f, -10.0f);
			ent.addComponent<gfx::mesh_renderer>({ .material = mat, .model = gfx::ModelCache::getModel("suzanne") });
		}

		{
			auto ent = createEntity("Cone");
			auto& transf = ent.addComponent<core::transform>();
			transf.scale = math::vec3::one * 3.0f;
			transf.position = math::vec3(-10.0f, 0.0f, 10.0f);
			ent.addComponent<gfx::mesh_renderer>({ .material = mat, .model = gfx::ModelCache::getModel("cone") });
		}

		{
			auto ent = createEntity("Sphere");
			auto& transf = ent.addComponent<core::transform>();
			transf.scale = math::vec3::one*3.0f;
			transf.position = math::vec3(10.0f, 0.0f, -10.0f);
			ent.addComponent<gfx::mesh_renderer>({ .material = mat, .model = gfx::ModelCache::getModel("sphere") });
		}

		//{
		//	dirLight = createEntity("Light");
		//	auto& transf = dirLight.addComponent<core::transform>();
		//	transf.scale = math::vec3::one;
		//	transf.position = math::vec3::one;
		//	transf.rotation = math::toQuat(math::vec3(-45.0f, 45.0f, 0.0f));
		//	dirLight.addComponent<gfx::light>({ .type = gfx::LightType::DIRECTIONAL, .dir_data.color = math::vec4(1.0f) });
		//}

		{
			pointLight = createEntity("PointLight");
			pointLight.addComponent<core::transform>({ .scale = math::vec3(.1f, .1f, .1f), .position = math::vec3(0.0f, 0.0f,0.0f) });
			pointLight.addComponent<gfx::light>({ .type = gfx::LightType::POINT, .point_data.color = math::vec4(1.0f,1.0f,1.0f,1.0f), .point_data.intensity = 1.0f, .point_data.range = 50.f });
			pointLight.addComponent<core::examplecomp>({ .direction = math::vec3::forward, .range = 5.0f, .speed = .02f});
			pointLight.addComponent<gfx::mesh_renderer>({ .material = mat, .model = gfx::ModelCache::getModel("icosphere") ,.castShadows = false });
		}

		{
			pointLight2 = createEntity("PointLight2");
			pointLight2.addComponent<core::transform>({ .scale = math::vec3(.1f, .1f, .1f), .position = math::vec3(0.0f,0.0f,0.0f) });
			pointLight2.addComponent<gfx::light>({ .type = gfx::LightType::POINT, .point_data.color = math::vec4(1.0f,1.0f,1.0f,1.0f), .point_data.intensity = 1.0f, .point_data.range = 50.f });
			pointLight2.addComponent<core::examplecomp>({ .direction = math::vec3::right, .range = 10.0f, .speed = .02f });
			pointLight2.addComponent<gfx::mesh_renderer>({ .material = mat, .model = gfx::ModelCache::getModel("icosphere") ,.castShadows = false});
		}

		{
			pointLight3 = createEntity("PointLight3");
			pointLight3.addComponent<core::transform>({ .scale = math::vec3(.1f, .1f, .1f), .position = math::vec3(0.0f,0.0f,0.0f) });
			pointLight3.addComponent<gfx::light>({ .type = gfx::LightType::POINT, .point_data.color = math::vec4(1.0f,1.0f,1.0f,1.0f), .point_data.intensity = 1.0f, .point_data.range = 50.f });
			pointLight3.addComponent<core::examplecomp>({ .direction = math::vec3::up, .range = 10.0f, .speed = .02f });
			pointLight3.addComponent<gfx::mesh_renderer>({ .material = mat, .model = gfx::ModelCache::getModel("icosphere") ,.castShadows = false });
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
		Begin("Docking Window");
		{
			static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
			static ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}End();
		Begin("Inspector");
		if (cube != invalid_id && CollapsingHeader(cube->name.c_str()))
		{
			Indent();
			transformEditor(cube);
			meshrendererEditor(cube);
			Unindent();
		}

		if (dirLight != invalid_id && CollapsingHeader(dirLight->name.c_str()))
		{
			Indent();
			transformEditor(dirLight);
			directionalLightEditor(dirLight);
			Unindent();
		}
		if (pointLight != invalid_id && CollapsingHeader(pointLight->name.c_str()))
		{
			Indent();
			transformEditor(pointLight);
			pointLightEditor(pointLight);
			exampleCompEditor(pointLight);
			Unindent();
		}
		//if (pointLight2 != invalid_id && CollapsingHeader(pointLight2->name.c_str()))
		//{
		//	Indent();
		//	transformEditor(pointLight2);
		//	pointLightEditor(pointLight2);
		//	exampleCompEditor(pointLight2);
		//	Unindent();
		//}
		//if (pointLight3 != invalid_id && CollapsingHeader(pointLight3->name.c_str()))
		//{
		//	Indent();
		//	transformEditor(pointLight3);
		//	pointLightEditor(pointLight3);
		//	exampleCompEditor(pointLight3);
		//	Unindent();
		//}

		ImGui::End();
	}

	void Game::directionalLightEditor(core::ecs::entity ent)
	{
		using namespace ImGui;
		if (ent.hasComponent<gfx::light>())
		{
			if (CollapsingHeader("Directional Light"))
			{
				auto& comp = ent.getComponent<gfx::light>();
				static math::color color = comp.dir_data.color;
				if (ColorEdit4("Directional Light Color", color.data))
				{
					comp.dir_data.color = color;
				}
			}
		}
	}

	void Game::pointLightEditor(core::ecs::entity ent)
	{
		using namespace ImGui;
		if (ent.hasComponent<gfx::light>())
		{
			if (CollapsingHeader("Point Light"))
			{
				auto& comp = ent.getComponent<gfx::light>();
				static math::color color = comp.point_data.color;
				if (ColorEdit4("Point Light Color", color.data))
				{
					comp.point_data.color = color;
				}
			}
		}
	}

	void Game::exampleCompEditor(core::ecs::entity ent)
	{
		using namespace ImGui;
		if (ent.hasComponent<core::examplecomp>())
		{
			if (CollapsingHeader("Example Component"))
			{
				auto& comp = ent.getComponent<core::examplecomp>();
				static float range = comp.range;
				static float speed = comp.speed;
				static float angularSpeed = comp.angularSpeed;
				static math::vec3 direction = comp.direction;

				if (InputFloat("Range", &range))
				{
					comp.range = range;
				}
				if (InputFloat("Speed", &speed))
				{
					comp.speed = speed;
				}
				if (InputFloat("Angular Speed", &angularSpeed))
				{
					comp.angularSpeed = angularSpeed;
				}
				if (InputFloat3("Direction", direction.data))
				{
					comp.direction = direction;
				}
			}
		}
	}

	void Game::meshrendererEditor(core::ecs::entity ent)
	{
		using namespace ImGui;
		if (ent.hasComponent<gfx::mesh_renderer>())
		{
			if (CollapsingHeader("Mesh Renderer"))
			{
				{
					auto models = gfx::ModelCache::getModels();
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
		}
	}

	void Game::transformEditor(core::ecs::entity ent)
	{
		using namespace ImGui;
		if (ent.hasComponent<core::transform>())
		{
			if (CollapsingHeader("Transform"))
			{
				auto& transf = ent.getComponent<core::transform>();
				static math::vec3 pos = transf.position;
				static math::vec3 rot = math::toEuler(transf.rotation);
				static math::vec3 scale = transf.scale;
				if (InputFloat3("Position", pos.data))
				{
					transf.position = pos;
				}
				if (InputFloat3("Rotation", rot.data))
				{
					transf.rotation = math::toQuat(rot);
				}
				if (InputFloat3("Scale", scale.data))
				{
					transf.scale = scale;
				}
			}
		}
	}

	void Game::reloadShaders(key_input<inputmap::method::F1>& input)
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