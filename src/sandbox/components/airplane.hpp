#pragma once
#include <rsl/math>
#include <rsl/primitives>
#include "sandbox/systems/planegame/physicssystem.hpp"

namespace rythe::game
{
	template <typename T>
	constexpr inline float inverse_lerp(T a, T b, T v)
	{
		v = math::clamp(v, a, b);
		return (v - a) / (b - a);
	}

	namespace isa
	{
		// get temperture in kelvin
		inline float getAirTemperature(float altitude)
		{
			assert(0.0f <= altitude && altitude <= 11000.0f);
			return 288.15f - 0.0065f * altitude;
		}

		// only accurate for altitudes < 11km
		float getAirDensity(float altitude)
		{
			assert(0.0f <= altitude && altitude <= 11000.0f);
			float temperature = getAirTemperature(altitude);
			float pressure = 101325.0f * std::pow(1 - 0.0065f * (altitude / 288.15f), 5.25f);
			return 0.00348f * (pressure / temperature);
		}

		const float sea_level_air_density = getAirDensity(0.0f);
	};

	namespace wgs84
	{
		constexpr float EARTH_RADIUS = 6378.0f;

		math::vec2 coordinate_diff_to_meters(const math::vec2& diff, float latitude)
		{
			float km_per_latitude = (math::pi() / 180.0f) * EARTH_RADIUS;
			float km_per_longitude = (math::pi() / 180.0f) * EARTH_RADIUS * cos(latitude * math::pi() / 180.0f);
			return math::vec2(km_per_latitude, km_per_longitude) / 1000.0f;
		}

		// origin is lat/lon, offset in meters
		math::vec2 lat_lon_from_offset(const math::vec2& origin, const math::vec2& offset)
		{
			float latitude = origin.x, longitude = origin.y;
			float new_latitude = latitude + (offset.y / EARTH_RADIUS) * (180.0f / math::pi());
			float new_longitude = longitude + (offset.x / EARTH_RADIUS) * (180.0f / math::pi()) / cos(latitude * math::pi()/ 180.0f);
			return math::vec2(new_latitude, new_longitude);
		}
	}  // namespace wgs84

	struct Engine
	{
		float throttle = 0.25f;
		math::vec3 relative_position = math::vec3(0);
	};

	struct SimpleEngine : public Engine
	{
		const float thrust;
	};

	struct PropellerEngine : public Engine
	{
		float horspower = 160.0f;
		float rpm = 2400.0f;
		float prop_diameter = 1.9f;
	};

	using AeroData = math::vec3;
	struct Airfoil
	{
		float min_alpha, max_alpha;
		float cl_max;
		int max_index;
		std::vector<AeroData> data;

		Airfoil(const std::vector<AeroData>& curve) : data(curve)
		{
			min_alpha = curve.front().x, max_alpha = curve.back().x;
			max_index = static_cast<int>(data.size() - 1);

			cl_max = 0.0f;

			for (auto& val : curve) {
				if (val.y > cl_max) cl_max = val.y;
			}
		}

		// lift_coeff, drag_coeff
		std::tuple<float, float> sample(float alpha) const
		{
			float t = inverse_lerp(min_alpha, max_alpha, alpha)* max_index;
			float integer = std::floor(t);
			float fractional = t - integer;
			int index = static_cast<int>(integer);
			auto value = (index < max_index) ? math::lerp(data[index], data[index + 1], fractional) : data[max_index];
			return { value.y, value.z };
		}
	};

	struct wing_component
	{
		const float area;
		const float wingspan;
		const float chord;
		const float aspect_ratio;
		const Airfoil* airfoil;
		const math::vec3 normal;
		const math::vec3 center_of_pressure;
		const float flap_ratio;
		const float efficiency_factor = 1.0f;

		float control_input = 0.0f;

		wing_component(const Airfoil* airfoil, const math::vec3& relative_position, float area, float span, const math::vec3& normal, float flap_ratio = 0.25f) :
			airfoil(airfoil), center_of_pressure(relative_position), area(area), chord(area / span), wingspan(span), normal(normal), aspect_ratio(math::squared(span) / area), flap_ratio(flap_ratio) {}

		wing_component(const math::vec3& position, float span, float chord, const Airfoil* airfoil, const math::vec3& normal, float flap_ratio = 0.25f) :
			airfoil(airfoil), center_of_pressure(position), area(span* chord), chord(chord), wingspan(span), normal(normal), aspect_ratio(math::squared(span) / area), flap_ratio(flap_ratio) { }

		struct aileron
		{
			wing_component* owner;
			float aileronArea;
			float aileronSpan;
			math::vec3 aileronOffset = math::vec3(0.0f, 0.0f, 0.0f);

			aileron(wing_component* _owner) : owner(_owner) {}
		};
		std::vector<aileron> ailerons;

		void setControlInput(float input) { control_input = math::clamp(input, -1.0f, 1.0f); }
	};

	struct airplane
	{
	public:
		float throttle = 0.25f;
		std::vector<wing_component> wings;
		std::vector<Engine*> engines;

		bool is_landed = false;
	};
}