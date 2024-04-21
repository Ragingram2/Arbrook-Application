#pragma once
#include "core/core.hpp"

namespace rythe::game
{
	using namespace rythe::core::events;
	class ExampleSystem : public core::System<ExampleSystem, core::examplecomp>
	{
	public:
		void setup()
		{
			for (auto ent : m_filter)
			{
				auto& comp = ent.getComponent<core::examplecomp>();
				auto& transf = ent.getComponent<core::transform>();
				comp.initPosition = transf.position;

			}
		}

		void update()
		{
			for (auto ent : m_filter)
			{
				auto& comp = ent.getComponent<core::examplecomp>();
				if (!ent->enabled || !comp.enabled) continue;

				comp.pos += comp.speed * core::Time::deltaTime;

				auto& transf = ent.getComponent<core::transform>();
				transf.position = comp.initPosition + (comp.direction * math::sin(math::radians(comp.pos)) * comp.range);

				transf.rotation = math::toQuat(math::rotate(transf.to_parent(), math::radians(comp.angularSpeed * core::Time::deltaTime), comp.axis));
			}
		}
	};
}
