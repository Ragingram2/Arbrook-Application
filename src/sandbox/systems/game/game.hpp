#pragma once
#include <chrono>
#include <functional>
#include <thread>
#include <future>

#include <filesystem>
#include <rfl.hpp>
#include <rfl/json.hpp>

#include "core/utils/profiler.hpp"

#include "core/core.hpp"
#include "graphics/rendering.hpp"
#include "input/input.hpp"

#include "sandbox/components/camerasettings.hpp"

namespace rythe::game
{
	using namespace std::chrono_literals;
	using namespace rythe::core::events;
	namespace fs = std::filesystem;
	namespace ast = rythe::core::assets;

	class Game : public core::System<Game, core::transform>
	{
	private:
		ast::asset_handle<gfx::model> modelHandle;
		ast::asset_handle<gfx::material> mat;
		ast::asset_handle<gfx::material> lit;
		ast::asset_handle<gfx::material> colorMat;

		std::future<void> future;

	public:
		void setup();
		void update();

		void reloadShaders(key_input<inputmap::method::F1>& input)
		{
			if (input.wasPressed())
			{
				ast::AssetCache<gfx::shader_source>::loadAssets("resources/shaders/", gfx::default_shader_params, true);
				gfx::ShaderCache::compileShaders();
			}
		}

		void toggleMouseCapture(key_input<inputmap::method::MOUSE_RIGHT>& input)
		{
			if (input.isPressed())
			{
				Input::mouseCaptured = true;
			}

			if (input.wasPressed())
			{
				Input::mouseCaptured = false;
			}
		}

		template<class _Rep, class _Period>
		std::future<void> TimerAsync(std::chrono::duration<_Rep, _Period> duration, std::function<void()> callback)
		{
			return std::async(std::launch::async, [duration, callback]()
				{
					std::this_thread::sleep_for(duration);
					callback();
				});
		}
	};
}