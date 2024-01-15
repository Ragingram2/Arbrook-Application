#pragma once

namespace rythe::game
{
	enum class CameraControlMode
	{
		FreeLook,
		Orbit,
		TwoD
	};

	struct camera_settings
	{
		CameraControlMode mode = CameraControlMode::FreeLook;
		float speed = 25.0f;
		float sensitivity = 0.9f;
	};
}