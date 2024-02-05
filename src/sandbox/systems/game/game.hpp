#pragma once
#include <filesystem>

#include "core/utils/profiler.hpp"

#include "core/core.hpp"
#include "graphics/rendering.hpp"
#include "input/input.hpp"

#include "sandbox/components/camerasettings.hpp"

namespace rythe::game
{
	using namespace rythe::core::events;
	namespace fs = std::filesystem;
	namespace ast = rythe::core::assets;

	class Game : public core::System<Game, core::transform, gfx::mesh_renderer>
	{
	private:
		core::ecs::entity cube;
		ast::asset_handle<gfx::model> modelHandle;
		ast::asset_handle<gfx::material> mat;
		ast::asset_handle<gfx::material> lit;
		ast::asset_handle<gfx::material> colorMat;

	public:
		void setup();
		void update();
		void guiRender();
		void setModel(ast::asset_handle<gfx::model> handle);
		void setMaterial(ast::asset_handle<gfx::material> handle);

		void reloadShaders(key_input<inputmap::method::NUM1>& input);

		void toggleMouseCapture(key_input<inputmap::method::ESCAPE>& input)
		{
			if (input.isPressed())
			{
				input::InputSystem::mouseCaptured = !input::InputSystem::mouseCaptured;
			}
		}
	};
}