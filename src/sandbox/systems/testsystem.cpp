//#include "sandbox/systems/testsystem.hpp"
//
//namespace rythe::testing
//{
//	void TestSystem::setup()
//	{
//		log::debug("Initializing TestSystem");
//
//		gfx::render_stage::addRender<TestSystem, &TestSystem::testRender>(this);
//		gfx::gui_stage::addGuiRender<TestSystem, &TestSystem::guiRender>(this);
//
//		gfx::ModelCache::loadModels("resources/meshes/");
//		//gfx::ModelCache::createModel("teapot", "resources/meshes/teapot.obj");
//		gfx::TextureCache::loadTextures("resources/textures/");
//		gfx::ShaderCache::loadShaders("resources/shaders/");
//
//		m_testScenes.emplace(APIType::Arbrook, std::vector<std::unique_ptr<rendering_test>>());
//		m_testScenes[APIType::Arbrook].emplace_back(std::make_unique<DrawIndexedInstancedTest<APIType::Arbrook>>());
//		m_testScenes[APIType::Arbrook].emplace_back(std::make_unique<ModelSwitchTest<APIType::Arbrook>>());
//
//		m_testScenes.emplace(APIType::BGFX, std::vector<std::unique_ptr<rendering_test>>());
//		m_testScenes[APIType::BGFX].emplace_back(std::make_unique<DrawIndexedInstancedTest<APIType::BGFX>>());
//		m_testScenes[APIType::BGFX].emplace_back(std::make_unique<ModelSwitchTest<APIType::BGFX>>());
//
//		m_testScenes.emplace(APIType::Native, std::vector<std::unique_ptr<rendering_test>>());
//		m_testScenes[APIType::Native].emplace_back(std::make_unique<DrawIndexedInstancedTest<APIType::Native>>());
//		m_testScenes[APIType::Native].emplace_back(std::make_unique<ModelSwitchTest<APIType::Native>>());
//
//		cameraEntity = createEntity("Camera");
//		{
//			auto& transf = cameraEntity.addComponent<core::transform>();
//			transf.position = cameraPos;
//			transf.rotation = math::quat(math::lookAt(cameraPos, cameraPos + math::vec3::forward, cameraUp));
//			auto& cam = cameraEntity.addComponent<gfx::camera>();
//			cam.farZ = 100.f;
//			cam.nearZ = 1.0f;
//			cam.fov = 90.f;
//		}
//	}
//
//	void TestSystem::update()
//	{
//		currentFrame = glfwGetTime();
//		deltaTime = currentFrame - lastFrame;
//		lastFrame = currentFrame;
//	}
//
//	void TestSystem::testRender(core::transform transf, gfx::camera cam)
//	{
//		if (testRunning)
//		{
//			if (currentIteration > maxIterations)
//			{
//				renderer.test->destroy();
//				currentIteration = 0;
//				currentType = static_cast<APIType>(currentType + 1);
//				if (currentType == None)
//				{
//					testRunning = false;
//					currentType = Arbrook;
//					renderer.test = nullptr;
//					writer.printResults();
//					return;
//				}
//				renderer.test = m_testScenes[currentType][currentTest].get();
//				return;
//			}
//
//			if (!renderer.test->initialized)
//			{
//				auto start = std::chrono::high_resolution_clock::now();
//				renderer.test->setup(cam, transf);
//				auto end = std::chrono::high_resolution_clock::now();
//				writer.writeTime(renderer.test->name, currentType, "SetupTime", std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
//			}
//			auto start = std::chrono::high_resolution_clock::now();
//			renderer.test->update(cam, transf);
//			auto end = std::chrono::high_resolution_clock::now();
//			writer.writeTime(renderer.test->name, currentType,"FrameTime", std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
//
//			currentIteration++;
//		}
//	}
//
//	void TestSystem::guiRender()
//	{
//		using namespace ImGui;
//		if (!testRunning)
//		{
//			Begin("Set Test");
//			//ShowDemoWindow();
//
//			//Text("Here is where you can select which API to use for rendering");
//			//static const char* typeNames[] = { "Arbrook","BGFX","Native" };
//			//if (BeginCombo("API Dropdown", typeNames[currentType]))
//			//{
//			//	for (int i = 0; i < 3; i++)
//			//	{
//			//		const bool is_selected = (currentType == i);
//			//		if (Button(typeNames[i]))
//			//		{
//			//			currentType = static_cast<APIType>(i);
//			//			testRenderer.test->destroy();
//			//			testRenderer.test = m_testScenes[currentType][0].get();
//			//		}
//
//			//		if (is_selected)
//			//		{
//			//			SetItemDefaultFocus();
//			//		}
//			//	}
//			//	EndCombo();
//			//}
//
//			Text("Here is where you can select which rendering test to run");
//			static const char* testNames[] = { "DrawTest","ModelSwitch","MaterialSwitch","BufferCreation"};
//			if (BeginCombo("Test Dropdown", testNames[currentTest]))
//			{
//				for (int i = 0; i < sizeof(testNames)/(sizeof(const char*)); i++)
//				{
//					const bool is_selected = (currentTest == i);
//					if (Selectable(testNames[i], is_selected))
//					{
//						currentTest = i;
//					}
//
//					if (is_selected)
//					{
//						SetItemDefaultFocus();
//					}
//				}
//				EndCombo();
//			}
//
//			if (Button("Run Test"))
//			{
//				runTest();
//			}
//
//			ImGui::End();
//		}
//	}
//}