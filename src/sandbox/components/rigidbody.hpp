#pragma once
#include <rsl/math>
#include "core/components/transform.hpp"

namespace rythe::game
{

	class PhysicsSystem;

	struct rigidbody
	{
		friend class PhysicsSystem;
	private:
		math::vec3 m_force;
		math::vec3 m_torque;
	public:
		float mass = 100.0f;
		math::vec3 velocity = math::vec3(0.0f);
		math::vec3 angular_velocity = math::vec3(0.0f);
		math::vec3 center_of_gravity;
		bool apply_gravity = true;
		bool sleep = false;
		math::mat3 inertia = math::mat3(0.0f);
		math::mat3 inverse_inertia = math::mat3(0.0f);

		inline float getInverseMass() const { return 1.0f / mass; }
		inline float getSpeed() const { return math::length(velocity); }
		inline math::vec3 getTorque() const { return m_torque; }
		inline math::vec3 getForce() const { return m_force; }
		inline math::vec3 getBodyVelocity(core::transform& transf) { return inverseTransformDirection(transf, velocity); }
		inline math::vec3 getPointVelocity(core::transform& transf, const math::vec3& point) { return getBodyVelocity(transf) + math::cross(angular_velocity, point); }

		inline void setInertia(const math::mat3& tensor) { inertia = tensor; inverse_inertia = math::inverse(tensor); }

		inline void addForce(const math::vec3& force) { m_force += force; }
		inline void addForceAtPoint(core::transform& transf, const math::vec3& force, const math::vec3& point) { m_force += core::transformDirection(transf, force); m_torque += math::cross(point, force); }
		inline void addRelativeForce(core::transform& transf, const math::vec3& force) { m_force += core::transformDirection(transf, force); }
		inline void addTorque(const math::vec3& torque) { m_torque += torque; }
		inline void addLinearImpulse(const math::vec3& impulse) { velocity += impulse / mass; }
		inline void addRelativeLinearImpulse(core::transform& transf, const math::vec3& impulse) { velocity += core::transformDirection(transf, impulse) / mass; }
		inline void addAngularImpulse(core::transform transf, const math::vec3& impulse) { angular_velocity += inverseTransformDirection(transf, impulse) * inverse_inertia; }
		inline void addRelativeAngularImpulse(const math::vec3& impulse) { angular_velocity += impulse * inverse_inertia; }
	};
}