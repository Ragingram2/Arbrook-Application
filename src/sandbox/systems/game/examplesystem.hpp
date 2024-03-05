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
				comp.pos += comp.speed * core::Time::deltaTime;

				auto& transf = ent.getComponent<core::transform>();
				transf.position = comp.direction * math::sin(math::radians(comp.pos)) * comp.range;
				transf.rotation = math::toQuat(math::rotate(transf.to_parent(), math::radians(comp.angularSpeed * core::Time::deltaTime), comp.axis));
			}
		}
	};
}
