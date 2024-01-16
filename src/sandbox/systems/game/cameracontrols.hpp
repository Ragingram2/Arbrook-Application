#pragma once
#include "core/core.hpp"
#include "graphics/rendering.hpp"
#include "input/input.hpp"
#include "sandbox/components/camerasettings.hpp"

namespace rythe::game
{
	using namespace rythe::core::events;
	namespace fs = std::filesystem;
	namespace ast = rythe::core::assets;

	class CameraControls : public core::System<CameraControls, core::transform, gfx::camera, camera_settings>
	{
	public:
		core::ecs::entity camera;

	private:
		math::vec3 velocity = math::vec3(0.0f);

		math::vec2 mousePos;
		math::vec2 lastMousePos;
		math::vec2 mouseDelta;
		math::vec2 rotationDelta;

		math::vec3 front;
		math::vec3 right;
		math::vec3 up;

		float yaw = -90.0f;
		float pitch = 90.0f;

	public:
		void setup();
		void update();

		void mouselook(mouse_input& input);
		void move(moveInput& input);

		void orbit(mouse_input& input);
		void freeLook(mouse_input& input);

		core::ecs::entity& getCameraEntity()
		{
			return m_filter[0];
		}

		gfx::camera& getCamera()
		{
			return m_filter[0].getComponent<gfx::camera>();
		}
	};
}