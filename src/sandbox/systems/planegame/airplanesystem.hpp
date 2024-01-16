#pragma once
#include <tracy/Tracy.hpp>

#include "core/core.hpp"
#include "graphics/rendering.hpp"
#include "input/input.hpp"

#include "core/components/transform.hpp"
#include "sandbox/components/rigidbody.hpp"
#include "sandbox/components/collider.hpp"
#include "sandbox/components/airplane.hpp"

namespace rythe::game
{

	class AirplaneSystem : public core::System<AirplaneSystem, core::transform, airplane, wing_component, rigidbody>
	{
	private:
		float throttle;
		float aileron;
		float rudder;
		float elevator;
		float trim;
	public:
		void setup();
		void update();

		void adoptWings(core::ecs::entity& ent, airplane& vehicle)
		{

		}

		void applyWingForce(rigidbody& rb, core::transform& transf, wing_component& wing);

		math::vec3 calcWingNormal(const math::vec3& normal, float incidence)
		{
			auto axis = math::normalize(math::cross(math::vec3::forward, normal));
			auto rotation = math::rotate(math::mat4(1.0f), math::radians(incidence), axis);
			return math::vec3(rotation * math::vec4(normal, 1.0f));
		}

		float getG(rigidbody& rb, core::transform& transf) const
		{
			auto velocity = rb.getBodyVelocity(transf);

			// avoid division by zero
			float turn_radius = (std::abs(rb.angular_velocity.z) < EPSILON) ? std::numeric_limits<float>::max()
				: velocity.x / rb.angular_velocity.z;

			// centrifugal force = mass * velocity^2 / radius
			// centrifugal acceleration = force / mass
			// simplified, this results in:
			float centrifugal_acceleration = math::squared(velocity.x) / turn_radius;

			float g_force = centrifugal_acceleration / EARTH_GRAVITY;
			g_force += (transf.up().y * EARTH_GRAVITY) / EARTH_GRAVITY;  // add earth gravity
			return g_force;
		}

		// mach number
		float getMach(rigidbody& rb, core::transform& transf) const
		{
			float temperature = isa::getAirTemperature(transf.position.y);
			float speed_of_sound = std::sqrt(1.402f * 286.f * temperature);
			return rb.getSpeed() / speed_of_sound;
		}

		// angle of attack
		float getAOA(rigidbody& rb, core::transform& transf) const
		{
			auto velocity = rb.getBodyVelocity(transf);
			return math::rad2deg(std::asin(math::dot(math::normalize(-velocity), math::vec3::up)));
		}

		// indicated air speed
		float getIAS(rigidbody& rb, core::transform& transf) const
		{
			// See: https://aerotoolbox.com/airspeed-conversions/
			float air_density = isa::getAirDensity(transf.position.y);
			float dynamic_pressure = 0.5f * math::squared(rb.getSpeed()) * air_density;  // bernoulli's equation
			return std::sqrt(2 * dynamic_pressure / isa::sea_level_air_density);
		}
	};
}