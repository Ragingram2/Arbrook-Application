#pragma once
#include "core/logging/logging.hpp"
#include "core/modules/module.hpp"
#include "sandbox/systems/testsystem.hpp"
#include "sandbox/systems/game.hpp"
#include "sandbox/systems/examplesystem.hpp"

namespace rythe::core
{
	class TestModule : public Module
	{
	public:
		void setup() override
		{
			log::info("Initializing Game Module");
			//reportSystem<game::Game>();
			reportSystem<game::ExampleSystem>();
			reportSystem<testing::TestSystem>();
		}
	};
}