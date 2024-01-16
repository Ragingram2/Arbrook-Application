#pragma once
#include <numeric>
#include <tracy/Tracy.hpp>

#include "core/core.hpp"
#include "graphics/rendering.hpp"
#include "input/input.hpp"

#include "core/components/transform.hpp"
#include "sandbox/components/rigidbody.hpp"
#include "sandbox/components/collider.hpp"

namespace rythe::game
{
	constexpr float EARTH_GRAVITY = 9.80665f;
	constexpr float EPSILON = 1e-8f;

	namespace units
	{
		constexpr inline float knots(float meter_per_second) { return meter_per_second * 1.94384f; }

		constexpr inline float meterPerSecond(float kilometer_per_hour) { return kilometer_per_hour / 3.6f; }

		constexpr inline float kilometerPerHour(float meter_per_second) { return meter_per_second * 3.6f; }

		constexpr inline float kelvin(float celsius) { return celsius - 273.15f; }

		constexpr inline float watts(float horsepower) { return horsepower * 745.7f; }

		constexpr inline float mileToKilometre(float mile) { return mile * 1.609f; }

		constexpr inline float feetToMeter(float feet) { return feet * 0.3048f; }
	}; 

	namespace calc
	{
		constexpr inline float torque(float power, float rpm) { return 30.0f * power / (2.0f * rpm); }

		constexpr float fall_time(float height, float acceleration = EARTH_GRAVITY) { return sqrt((2 * height) / acceleration); }
	};

	namespace inertia
	{
		// mass element used for inertia tensor calculation
		struct Element {
			math::vec3 size;
			math::vec3 position;  // position in design coordinates
			math::vec3 inertia;   // moment of inertia
			math::vec3 offset;    // offset from center of gravity
			float mass;
			float volume() const { return size.x * size.y * size.z; }
		};
		// cuboid moment of inertia
		inline math::vec3 cuboid(float mass, const math::vec3& size)
		{
			float x = size.x, y = size.y, z = size.z;
			return math::vec3(math::squared(y) + math::squared(z), math::squared(x) + math::squared(z), math::squared(x) + math::squared(y)) * (1.0f / 12.0f) * mass;
		}

		// sphere moment of inertia
		inline math::vec3 sphere(float mass, float radius) { return math::vec3((2.0f / 5.0f) * mass * math::squared(radius)); }

		inline Element cube(const math::vec3& position, const math::vec3& size, float mass = 0.0f)
		{
			math::vec3 inertia = cuboid(mass, size);
			return { size, position, inertia, position, mass };
		}

		// inertia tensor from moment of inertia
		//inline math::mat3 tensor(const math::vec3& moment_of_inertia) { return math::diagonal3x3(moment_of_inertia); }

		// distribute mass among elements depending on element volume, to be called before passing elements to tensor()
		inline void set_uniform_density(std::vector<Element>& elements, float total_mass)
		{
			auto f = [](float s, auto& e) { return s + e.volume(); };
			float total_volume = std::accumulate(elements.begin(), elements.end(), 0.0f, f);

			for (auto& element : elements) {
				element.mass = (element.volume() / total_volume) * total_mass;
			}
		}

		// calculate inertia tensor for a collection of connected masses
		inline math::mat3 tensor(std::vector<Element>& elements, bool precomputed_offset = false, math::vec3* cg = nullptr)
		{
			float Ixx = 0, Iyy = 0, Izz = 0;
			float Ixy = 0, Ixz = 0, Iyz = 0;

			float mass = 0;
			math::vec3 moment_of_inertia(0.0f);

			for (const auto& element : elements) {
				mass += element.mass;
				moment_of_inertia += element.mass * element.position;
			}

			const auto center_of_gravity = moment_of_inertia / mass;

			for (auto& element : elements) {
				if (!precomputed_offset) {
					element.offset = element.position - center_of_gravity;
				}
				else {
					element.offset = element.position;
				}

				const auto offset = element.offset;

				Ixx += element.inertia.x + element.mass * (math::squared(offset.y) + math::squared(offset.z));
				Iyy += element.inertia.y + element.mass * (math::squared(offset.z) + math::squared(offset.x));
				Izz += element.inertia.z + element.mass * (math::squared(offset.x) + math::squared(offset.y));
				Ixy += element.mass * (offset.x * offset.y);
				Ixz += element.mass * (offset.x * offset.z);
				Iyz += element.mass * (offset.y * offset.z);
			}

			if (cg != nullptr) {
				*cg = center_of_gravity;
			}

			// clang-format off
			return math::mat3{
				Ixx, -Ixy, -Ixz,
				-Ixy, Iyy, -Iyz,
				-Ixz, -Iyz, Izz
			};
			// clang-format on
		}
	};

	class PhysicsSystem : public core::System<PhysicsSystem, core::transform, rigidbody>
	{
	public:
		void setup();
		void update();
	};
}