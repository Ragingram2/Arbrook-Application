//#pragma once
//
//#include <tracy/Tracy.hpp>
//
//#include "core/core.hpp"
//#include "rendering/rendering.hpp"
//#include "input/input.hpp"
//
//namespace rythe::game
//{
//	using namespace rythe::core::events;
//	//using moveInput = axis_input < axis_data{inputmap::method::A, inputmap::method::D, 1, -1}, axis_data{inputmap::method::W, inputmap::method::S, 1, -1}, axis_data{ inputmap::method::Q, inputmap::method::E, 1, -1 } >;
//
//	class Game : public core::System<Game,core::transform, gfx::mesh_renderer>
//	{
//	private:
//		core::ecs::entity camera;
//		core::ecs::entity cube;
//		gfx::model_handle modelHandle;
//		gfx::material mat;
//		gfx::material lit;
//		gfx::material color;
//
//		math::vec3 velocity = math::vec3(0.0f);
//
//		math::vec2 mousePos;
//		math::vec2 lastMousePos;
//		math::vec2 mouseDelta;
//		math::vec2 rotationDelta;
//
//		math::vec3 front;
//		math::vec3 right;
//		math::vec3 up;
//
//		float speed = 25.0f;
//
//		float deltaTime = 0.0f;
//		float lastFrame = 0.0f;
//		float currentFrame = 0.0f;
//
//		float sensitivity = 0.9f;
//
//		float yaw = -90.0f;
//		float pitch = 90.0f;
//
//	public:
//		void setup();
//		void update();
//		void guiRender();
//		void setModel(gfx::model_handle handle);
//		void setMaterial(gfx::material_handle handle);
//
//		void reloadShaders(key_input<inputmap::method::NUM1>& input);
//		void move(moveInput& input);
//		void mouselook(mouse_input& input);
//
//		void toggleMouseCapture(key_input<inputmap::method::ESCAPE>& input)
//		{
//			if (input.isPressed())
//			{
//				input::InputSystem::mouseCaptured = !input::InputSystem::mouseCaptured;
//			}
//		}
//	};
//}