#include "sandbox/systems/game/game.hpp"

namespace fs = std::filesystem;
namespace ast = rythe::core::assets;
namespace rythe::game
{
	void Game::setup()
	{
		log::info("Initializing Game system");
		bindEvent<key_input<inputmap::method::F1>, &Game::reloadShaders>();
		bindEvent<key_input<inputmap::method::MOUSE_RIGHT>, &Game::toggleMouseCapture>();

		input::InputSystem::registerWindow(gfx::Renderer::RI->getGlfwWindow());

		ast::AssetCache<gfx::texture_source>::registerImporter<gfx::TextureImporter>();
		ast::AssetCache<gfx::mesh>::registerImporter<gfx::MeshImporter>();
		ast::AssetCache<gfx::shader_source>::registerImporter<gfx::ShaderImporter>();
		ast::AssetCache<gfx::material_source>::registerImporter<gfx::MaterialImporter>();

		ast::AssetCache<gfx::texture_source>::loadAssets("resources/textures/", gfx::default_texture_import_params);
		ast::AssetCache<gfx::shader_source>::loadAssets("resources/shaders/", gfx::default_shader_params);
		gfx::ShaderCache::createShaders(ast::AssetCache<gfx::shader_source>::getAssets());
		ast::AssetCache<gfx::mesh>::loadAssets("resources/meshes/glb/", gfx::default_mesh_params);
		ast::AssetCache<gfx::material_source>::loadAssets("resources/materials/", gfx::default_material_params);
		gfx::MaterialCache::loadMaterial("error");
		gfx::ModelCache::loadModels(ast::AssetCache<gfx::mesh>::getAssets());
		gfx::MaterialCache::loadMaterials(ast::AssetCache<gfx::material_source>::getAssets());

		gfx::TextureCache::createTexture2D("park", ast::AssetCache<gfx::texture_source>::getAsset("park"), gfx::texture_parameters
			{
				.usage = gfx::UsageType::IMMUTABLE
			});

		gfx::TextureCache::createTexture2D("bricks_normal", ast::AssetCache<gfx::texture_source>::getAsset("bricks_normal"), gfx::texture_parameters
			{
				.wrapModeS = gfx::WrapMode::REPEAT,
				.wrapModeT = gfx::WrapMode::REPEAT,
				.minFilterMode = gfx::FilterMode::LINEAR_MIPMAP_LINEAR,
				.magFilterMode = gfx::FilterMode::LINEAR
			});


		mat = gfx::MaterialCache::getMaterial("default");

		colorMat = gfx::MaterialCache::getMaterial("color");
		colorMat->getShader()->addBuffer(gfx::BufferCache::createConstantBuffer<math::vec4>("Color", 3, gfx::UsageType::STATICDRAW));
		math::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		colorMat->setUniform("Color", &color, 3);

		{
			auto& skyboxRenderer = registry->world.addComponent<gfx::skybox_renderer>();
			skyboxRenderer.skyboxTex = gfx::TextureCache::getTexture("park");
		}

		//{
		//	auto ent = createEntity("Floor");
		//	auto& transf = ent.addComponent<core::transform>();
		//	transf.scale = math::vec3(20, .5f, 20);
		//	transf.position = math::vec3(0.0f, 0.0f, 0.0f);
		//	ent.addComponent<gfx::mesh_renderer>({ .mainMaterial = mat, .model = gfx::ModelCache::getModel("cube") ,.castShadows = false });
		//}

		//{
		//	auto ent = createEntity("Sponza");
		//	auto& transf = ent.addComponent<core::transform>();
		//	transf.scale = math::vec3::one * 10.0f;
		//	transf.position = math::vec3(0.0f, 10.0f, 0.0f);
		//	ent.addComponent<gfx::mesh_renderer>({ .mainMaterial = gfx::MaterialCache::getMaterial("sponza-material"), .model = gfx::ModelCache::getModel("sponza"), .castShadows = true });
		//}

		{
			auto ent = createEntity("Entity");
			auto& transf = ent.addComponent<core::transform>();
			transf.scale = math::vec3::one;
			transf.position = math::vec3(0.0f, 10.0f, 0.0f);
			transf.rotation = math::toQuat(math::vec3(-90.0f,0.0f,0.0f));
			ent.addComponent<gfx::mesh_renderer>({ .mainMaterial = gfx::MaterialCache::getMaterial("bog"), .model = gfx::ModelCache::getModel("cube"), .castShadows = true });
		}

		{
			auto ent = createEntity("Directional Light");
			auto& transf = ent.addComponent<core::transform>();
			transf.scale = math::vec3::one;
			transf.position = math::vec3(0.0f, 25.f, 0.0f);
			transf.rotation = math::toQuat(math::vec3(-90.0f, 0.0f, 0.0f));
			ent.addComponent<gfx::light>({ .type = gfx::LightType::DIRECTIONAL, .dir_data.color = math::vec4(1.0f), .dir_data.intensity = 1.0f });
			ent.addComponent<gfx::mesh_renderer>({ .mainMaterial = colorMat, .model = gfx::ModelCache::getModel("cone"), .castShadows = false });
		}

		{
			auto ent = createEntity("Point Light");
			ent.addComponent<core::transform>({ .scale = math::vec3(.1f, .1f, .1f), .position = math::vec3(0.0f, 10.0f, 0.0f) });
			ent.addComponent<gfx::light>({ .type = gfx::LightType::POINT, .point_data.color = math::vec4(1.0f,1.0f,1.0f,1.0f), .point_data.intensity = 1.0f, .point_data.range = 100.f });
			ent.addComponent<core::examplecomp>({ .direction = math::vec3::up, .range = 10.0f, .speed = 20.0f });
			ent.addComponent<gfx::mesh_renderer>({ .mainMaterial = gfx::MaterialCache::getMaterial("red"), .model = gfx::ModelCache::getModel("icosphere") ,.castShadows = false });
		}

		{
			auto camera = createEntity("Camera");
			auto& camTransf = camera.addComponent<core::transform>();
			camTransf.position = math::vec3(0.0f, 10.0f, -10.0f);
			camTransf.rotation = math::quat(math::lookAt(math::vec3::zero, camTransf.forward(), camTransf.up()));
			camera.addComponent<gfx::camera>({ .farZ = 10000.f, .nearZ = 0.1f, .fov = 90.f });
			camera.addComponent<camera_settings>({ .mode = CameraControlMode::FreeLook, .speed = 10.0f, .sensitivity = .9f });
		}

		//future = TimerAsync(30s, []() {log::debug("Timer Finished"); });
	}

	void Game::update()
	{
		ZoneScopedN("Game Update");

		//auto deltaTime = static_cast<std::chrono::duration<float, std::milli>>(core::Time::deltaTime);
		//auto status = future.wait_for(deltaTime);
		//if (status != std::future_status::ready)
		//{
		//	log::debug("Processing....");
		//}
	}
}