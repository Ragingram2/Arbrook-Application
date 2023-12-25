//#pragma once
//#include "core/core.hpp"
//#include "rendering/rendering.hpp"
//#include "input/input.hpp"
//#include "sandbox/components/test_renderer.hpp"
//#include "sandbox/tests/drawindexedtest.hpp"
//#include "sandbox/tests/modelswitch.hpp"
//#include "sandbox/tests/tools/CSVWriter.hpp"
//
//namespace rythe::testing
//{
//	using namespace rythe::core::events;
//	class TestSystem : public core::System<core::transform, test_renderer>
//	{
//		APIType currentType = APIType::Arbrook;
//		std::unordered_map<unsigned int, std::vector<std::unique_ptr<rendering_test>>> m_testScenes;
//
//		core::ecs::entity cameraEntity;
//		test_renderer renderer;
//		gfx::model_handle currentModel;
//		gfx::material_handle currentMatertial;
//
//		math::vec3 cameraPos = math::vec3(0.f,0.f,0.f);
//		math::vec3 cameraUp = math::vec3::up;
//		
//		float deltaTime = 0.0f;
//		float lastFrame = 0.0f;
//		float currentFrame = 0.0f;
//
//		bool initTest = false;
//		bool testRunning = false;
//
//		int maxIterations = 10000;
//		int currentIteration = 0;
//		int currentTest = 0;
//
//#if RenderingAPI == RenderingAPI_OGL
//		CSVWriter writer = CSVWriter("resources\\logs\\ogldata.csv");
//#elif RenderingAPI == RenderingAPI_DX11
//		CSVWriter writer = CSVWriter("resources\\logs\\dx11data.csv");
//#endif
//		void runTest()
//		{
//			testRunning = true;
//			renderer.test = m_testScenes[Arbrook][currentTest].get();
//		}
//	public:
//		void setup();
//		void update();
//		void testRender(core::transform,gfx::camera);
//		void guiRender();
//
//	};
//}