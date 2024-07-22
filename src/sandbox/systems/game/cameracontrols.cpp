#include "sandbox/systems/game/cameracontrols.hpp"

namespace rythe::game
{
	void CameraControls::setup()
	{
		bindEvent<moveInput, &CameraControls::move>();
		bindEvent<mouse_input, &CameraControls::mouselook>();

		camera = getCameraEntity();
	}

	void CameraControls::update()
	{
		ZoneScopedN("Camera Controls");
		auto& camTransf = camera.getComponent<core::transform>();

		camTransf.position += velocity;
		velocity = math::vec3::zero;

		camTransf.rotation = math::conjugate(math::toQuat(math::lookAt(math::vec3::zero, front, up)));
	}

	void CameraControls::move(moveInput& input)
	{
		auto& camSettings = camera.getComponent<camera_settings>();
		auto& transf = camera.getComponent<core::transform>();

		//auto leftRight = input.m_values["Left/Right"];
		//auto forwardBackward = input.m_values["Forward/Backward"];
		//auto upDown = input.m_values["Up/Down"];

		auto leftRight = input.getValue(0);
		auto forwardBackward = input.getValue(1);
		auto upDown = input.getValue(2);
		velocity += transf.right() * leftRight;
		velocity += transf.forward() * forwardBackward;
		velocity += transf.up() * upDown;
		if (velocity.length() > 0.00001f)
			velocity = math::normalize(velocity) * camSettings.speed * core::Time::deltaTime;
	}

	void CameraControls::mouselook(mouse_input& input)
	{
		if (!Input::mouseCaptured) return;

		auto& camSettings = camera.getComponent<camera_settings>();
		switch (camSettings.mode)
		{
		case CameraControlMode::FreeLook:
			freeLook(input);
			break;
		case CameraControlMode::Orbit:
			orbit(input);
			break;
		default:
			break;
		}

	}

	void CameraControls::orbit(mouse_input& input)
	{

	}

	void CameraControls::freeLook(mouse_input& input)
	{
		auto& camSettings = camera.getComponent<camera_settings>();

		static bool firstMouse = true;

		if (firstMouse)
		{
			lastMousePos = input.lastPosition;
			firstMouse = false;
		}

		mousePos = input.position;
		mouseDelta = input.positionDelta;
		lastMousePos = input.lastPosition;

		rotationDelta = -math::vec2(mouseDelta.x, mouseDelta.y) * camSettings.sensitivity;

		pitch = math::clamp(pitch + rotationDelta.y, -89.99f, 89.99);
		yaw += rotationDelta.x;

		front.x = cos(math::radians(yaw)) * cos(math::radians(pitch));
		front.y = sin(math::radians(pitch));
		front.z = sin(math::radians(yaw)) * cos(math::radians(pitch));
		front = math::normalize(front);
		right = math::normalize(math::cross(front, math::vec3::up));
		up = math::normalize(math::cross(right, front));
	}
}