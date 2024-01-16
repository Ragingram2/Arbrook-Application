#include "sandbox/systems/planegame/airplanesystem.hpp"

namespace rythe::game
{
	void AirplaneSystem::setup()
	{

	}

	void AirplaneSystem::update()
	{

	}

	void AirplaneSystem::applyWingForce(rigidbody& rb, core::transform& transf, wing_component& wing)
	{
		math::vec3 local_velocity = rb.getPointVelocity(transf, wing.center_of_pressure);
		float speed = math::length(local_velocity);

		if (speed <= EPSILON) return;

		math::vec3 drag_direction = math::normalize(-local_velocity);
		math::vec3 lift_direction = math::normalize(math::cross(math::cross(drag_direction, wing.normal), drag_direction));

		float angle_of_attack = math::rad2deg(std::asin(math::dot(drag_direction, wing.normal)));

		auto [lift_coeff, drag_coeff] = wing.airfoil->sample(angle_of_attack);

		if (wing.flap_ratio > 0.0f)
		{
			float delta_lift_coeff = math::sqrt(wing.flap_ratio) * wing.airfoil->cl_max * wing.control_input;
			lift_coeff += delta_lift_coeff;
		}

		float induced_drag_coeff = math::squared(wing.flap_ratio) / (math::pi() * wing.aspect_ratio * wing.efficiency_factor);
		drag_coeff += induced_drag_coeff;

		float air_density = isa::getAirDensity(transf.position.y);

		float dynamic_pressure = 0.5f * math::squared(speed) * air_density * wing.area;

		math::vec3 lift = lift_direction * lift_coeff * dynamic_pressure;
		math::vec3 drag = drag_direction * drag_coeff * dynamic_pressure;

		rb.addForceAtPoint(transf, lift + drag, wing.center_of_pressure);
	}
}