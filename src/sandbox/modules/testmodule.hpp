#pragma once
#include "core/logging/logging.hpp"
#include "core/modules/module.hpp"
#include "sandbox/systems/testing/testsystem.hpp"
#include "sandbox/systems/game/game.hpp"
#include "sandbox/systems/game/examplesystem.hpp"
#include "sandbox/systems/game/cameracontrols.hpp"
#include "sandbox/systems/planegame/physicssystem.hpp"

namespace rythe::core
{
	class TestModule : public Module
	{
	public:
		void setup() override
		{
			log::info("Initializing Game Module");
			//reportSystem<game::Game>();
			//reportSystem<game::ExampleSystem>();
			//reportSystem<game::PhysicsSystem>();
			//reportSystem<game::CameraControls>();
			reportSystem<testing::TestSystem>();
		}
	};
}