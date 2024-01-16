#pragma once
#include "core/core.hpp"

namespace rythe::game
{
	using namespace rythe::core::events;
	class ExampleSystem : public core::System<ExampleSystem,core::examplecomp>
	{
	public:
		void update()
		{
			for (auto ent : m_filter)
			{
				auto& comp = ent.getComponent<core::examplecomp>();
				comp.pos += comp.speed;

				auto& transf = ent.getComponent<core::transform>();
				transf.position = math::vec3(math::sin(math::radians(comp.pos)) * comp.range * comp.direction, transf.position.y, transf.position.z);
				transf.rotation = math::toQuat(math::rotate(transf.to_parent(), math::radians(comp.angularSpeed) * comp.direction, transf.up()));
			}
		}
	};
}
