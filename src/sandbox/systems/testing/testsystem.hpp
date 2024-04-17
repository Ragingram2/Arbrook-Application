#pragma once
#include <format>

#include "core/core.hpp"
#include "graphics/rendering.hpp"
#include "input/input.hpp"
#include "sandbox/components/test_renderer.hpp"
#include "sandbox/tests/stresstest.hpp"
#include "sandbox/tests/drawindexedtest.hpp"
#include "sandbox/tests/modelswitch.hpp"
#include "sandbox/tests/materialswitch.hpp"
#include "sandbox/tests/buffercreation.hpp"
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

		bool initTest = false;
		bool runningTest = false;
		bool runningAllTests = false;

		int currentIteration = 0;
		int currentTest = 0;

		void runTest();
		void runAllTests();
		void writeTests();
	public:
		void setup();
		void update();
		void onRender(core::transform,gfx::camera);
		void BGFXRender();
		void guiRender();

		void initializeTest(core::transform, gfx::camera);
		void resetTest();
		void updateTest(core::transform, gfx::camera);


		void toggleMouseCapture(key_input<inputmap::method::ESCAPE>& input)
		{
			if (input.isPressed())
			{
				Input::mouseCaptured = !Input::mouseCaptured;
			}
		}

	};
}