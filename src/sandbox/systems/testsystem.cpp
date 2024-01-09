#include "sandbox/systems/testsystem.hpp"

namespace rythe::testing
{
	void TestSystem::setup()
	{
		log::info("Initializing Test System");

		bindEvent<key_input<inputmap::method::ESCAPE>, &TestSystem::toggleMouseCapture>();

		input::InputSystem::registerWindow(gfx::Renderer::RI->getGlfwWindow());

		gfx::render_stage::addRender<TestSystem, &TestSystem::onRender>(this);
		gfx::gui_stage::addGuiRender<TestSystem, &TestSystem::guiRender>(this);
		testing::bgfx_render_stage::addRender<TestSystem, &TestSystem::BGFXRender>(this);

		ast::AssetCache<gfx::texture>::registerImporter<gfx::TextureImporter>();
		ast::AssetCache<gfx::mesh>::registerImporter<gfx::MeshImporter>();
		ast::AssetCache<gfx::shader>::registerImporter<gfx::ShaderImporter>();

		ast::AssetCache<gfx::mesh>::loadAssets("resources/meshes/glb/", gfx::default_mesh_params);
		ast::AssetCache<gfx::texture>::loadAssets("resources/textures/", gfx::default_params);
		ast::AssetCache<gfx::shader>::loadAssets("resources/shaders/", gfx::default_shader_params);
		gfx::ModelCache::loadModels(ast::AssetCache<gfx::mesh>::getAssets());

		m_testScenes.emplace(APIType::Arbrook, std::vector<std::unique_ptr<rendering_test>>());
		m_testScenes[APIType::Arbrook].emplace_back(std::make_unique<DrawIndexedInstancedTest<APIType::Arbrook>>());
		m_testScenes[APIType::Arbrook].emplace_back(std::make_unique<ModelSwitchTest<APIType::Arbrook>>());

		m_testScenes.emplace(APIType::Native, std::vector<std::unique_ptr<rendering_test>>());
		m_testScenes[APIType::Native].emplace_back(std::make_unique<DrawIndexedInstancedTest<APIType::Native>>());
		m_testScenes[APIType::Native].emplace_back(std::make_unique<ModelSwitchTest<APIType::Native>>());

		m_testScenes.emplace(APIType::BGFX, std::vector<std::unique_ptr<rendering_test>>());
		m_testScenes[APIType::BGFX].emplace_back(std::make_unique<DrawIndexedInstancedTest<APIType::BGFX>>());
		m_testScenes[APIType::BGFX].emplace_back(std::make_unique<ModelSwitchTest<APIType::BGFX>>());

		cameraEntity = createEntity("Camera");
		{
			auto cameraPos = math::vec3(0.f, 0.f, 0.f);
			auto& transf = cameraEntity.addComponent<core::transform>();
			transf.position = cameraPos;
			transf.rotation = math::quat(math::lookAt(cameraPos, cameraPos + math::vec3::forward, math::vec3::up));
			auto& cam = cameraEntity.addComponent<gfx::camera>();
			cam.farZ = 100.f;
			cam.nearZ = 1.0f;
			cam.fov = 90.f;
		}
	}

	void TestSystem::update()
	{
		currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
	}

	void TestSystem::initializeTest(core::transform transf, gfx::camera cam)
	{
		FrameClock clock("", currentType, "SetupTime");
		renderer.test->setup(cam, transf);
		clock.testName = renderer.test->name;
	}

	void TestSystem::updateTest(core::transform transf, gfx::camera cam)
	{
		FrameClock clock(renderer.test->name, currentType, "FrameTime");
		renderer.test->update(cam, transf);
		currentIteration++;
	}

	void TestSystem::resetTest()
	{
		renderer.test->destroy();
		currentIteration = 0;
		currentType = static_cast<APIType>(currentType + 1);
		if (currentType == None)
		{
			testRunning = false;
			currentType = Arbrook;
			renderer.test = nullptr;
			gfx::WindowProvider::activeWindow->setWindowTitle("Arbrook");
#if RenderingAPI == RenderingAPI_OGL
			CSVWriter::printResults("resources\\data\\ogldata.csv");
#elif RenderingAPI == RenderingAPI_DX11
			CSVWriter::printResults("resources\\data\\dx11data.csv");
#endif
			return;
		}
		renderer.test = m_testScenes[currentType][currentTest].get();
	}

	void TestSystem::BGFXRender()
	{
		gfx::camera cam;
		core::transform transf;
		if (testRunning)
		{
			if (currentIteration > maxIterations)
			{
				resetTest();
				gfx::WindowProvider::destroyWindow("BGFX");
				gfx::Renderer::setPipeline<gfx::DefaultPipeline>();
				return;
			}

			if (!renderer.test->initialized)
			{
				glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
				gfx::Renderer::RI->setWindow(gfx::WindowProvider::addWindow("BGFX"));
				gfx::WindowProvider::setActive("BGFX");
				initializeTest(transf, cam);
			}

			updateTest(transf, cam);
		}
	}

	void TestSystem::onRender(core::transform transf, gfx::camera cam)
	{
		if (currentType == BGFX)
		{
			gfx::Renderer::setPipeline<testing::BGFXPipeline>();
			return;
		}
		if (testRunning)
		{
			if (currentIteration > maxIterations)
			{
				resetTest();
				return;
			}

			if (!renderer.test->initialized)
			{
				auto windowHandle = gfx::WindowProvider::setActive("Arbrook");
				windowHandle->makeCurrent();
				gfx::Renderer::RI->setWindow(windowHandle);
				windowHandle->makeCurrent();
				initializeTest(transf, cam);
			}

			updateTest(transf, cam);
		}
	}

	void TestSystem::guiRender()
	{
		using namespace ImGui;
		if (!testRunning)
		{
			Begin("Set Test");
			Text("Here is where you can select which rendering test to run");
			static const char* testNames[] = { "DrawTest","ModelSwitch","MaterialSwitch","BufferCreation" };
			if (BeginCombo("Test Dropdown", testNames[currentTest]))
			{
				for (int i = 0; i < sizeof(testNames) / (sizeof(const char*)); i++)
				{
					const bool is_selected = (currentTest == i);
					if (Selectable(testNames[i], is_selected))
					{
						currentTest = i;
					}

					if (is_selected)
					{
						SetItemDefaultFocus();
					}
				}
				EndCombo();
			}

			if (Button("Run Test"))
			{
				runTest();
			}

			End();
		}
	}

	void TestSystem::runTest()
	{
		testRunning = true;
		renderer.test = m_testScenes[currentType][currentTest].get();
	}
}