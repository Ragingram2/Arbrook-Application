#pragma once
#include <rsl/math>

namespace rythe::game
{
	enum ColliderType
	{
		CUBE,
		SPHERE
	};

	template<ColliderType Type>
	struct collider {};

	template<>
	struct collider<CUBE>
	{
		math::vec3 size;
		math::vec3 center;
		float volume() const { return size.x * size.y * size.z; }
	};

	using box_collider = collider<CUBE>;
}