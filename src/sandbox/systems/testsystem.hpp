#pragma once
#include "core/core.hpp"
#include "graphics/rendering.hpp"
#include "input/input.hpp"
#include "sandbox/components/test_renderer.hpp"
#include "sandbox/tests/drawindexedtest.hpp"
#include "sandbox/tests/modelswitch.hpp"
#include "sandbox/tests/tools/CSVWriter.hpp"
#include "sandbox/tests/bgfxpipeline/bgfxrenderstage.hpp"

namespace rythe::testing
{
	using namespace rythe::core::events;
	namespace fs = std::filesystem;
	namespace ast = rythe::core::assets;
	class TestSystem : public core::System<TestSystem, core::transform, test_renderer>
	{
		APIType currentType = APIType::Arbrook;
		std::unordered_map<unsigned int, std::vector<std::unique_ptr<rendering_test>>> m_testScenes;

		core::ecs::entity cameraEntity;
		test_renderer renderer;
		ast::asset_handle<gfx::model> currentModel;
		ast::asset_handle<gfx::material> currentMatertial;

		math::vec3 cameraPos = math::vec3(0.f,0.f,0.f);
		math::vec3 cameraUp = math::vec3::up;
		
		float deltaTime = 0.0f;
		float lastFrame = 0.0f;
		float currentFrame = 0.0f;

		bool initTest = false;
		bool testRunning = false;

		int maxIterations = 10000;
		int currentIteration = 0;
		int currentTest = 0;

//#if RenderingAPI == RenderingAPI_OGL
//		CSVWriter writer = CSVWriter("resources\\logs\\ogldata.csv");
//#elif RenderingAPI == RenderingAPI_DX11
//		CSVWriter writer = CSVWriter("resources\\logs\\dx11data.csv");
//#endif
		void runTest();
	public:
		void setup();
		void update();
		void testRender(core::transform,gfx::camera);
		void guiRender();

		void BGFXRender();

		void toggleMouseCapture(key_input<inputmap::method::ESCAPE>& input)
		{
			if (input.isPressed())
			{
				input::InputSystem::mouseCaptured = !input::InputSystem::mouseCaptured;
			}
		}

	};
}