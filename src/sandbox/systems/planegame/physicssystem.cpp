#include "sandbox/systems/planegame/physicssystem.hpp"

namespace rythe::game
{
	void PhysicsSystem::setup()
	{

	}

	void PhysicsSystem::update()
	{
		for (auto& ent : m_filter)
		{
			auto& rb = ent.getComponent<rigidbody>();
			auto& transf = ent.getComponent<core::transform>();

			if (rb.sleep) continue;

			math::vec3 acceleration = rb.m_force / rb.mass;

			if (rb.apply_gravity) acceleration.y -= EARTH_GRAVITY;

			rb.velocity += acceleration * core::Time::deltaTime;
			transf.position += rb.velocity * core::Time::deltaTime;

			rb.angular_velocity += rb.inverse_inertia * (rb.m_torque - math::cross(rb.angular_velocity, rb.inertia * rb.angular_velocity)) * core::Time::deltaTime;
			transf.rotation += (transf.rotation * math::quat(0.0, rb.angular_velocity)) * (0.5f * core::Time::deltaTime);
			transf.rotation = math::normalize(transf.rotation);

			rb.m_force = math::vec3::zero;
			rb.m_torque = math::vec3::zero;
		}
	}
}