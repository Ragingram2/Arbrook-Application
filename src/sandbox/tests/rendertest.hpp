#pragma once
#include <string>

#include <rsl/math>

#include <tracy/Tracy.hpp>

#include "graphics/interface/definitions/definitions.hpp"
#include "graphics/rendering.hpp"

namespace rythe::testing
{
	enum APIType
	{
		Arbrook = 0,
		BGFX = 1,
		Native = 2,
		None = 3,
		Count = 4,
	};


	inline std::string getAPIName(APIType type)
	{
		switch (type)
		{
		case APIType::Arbrook:
			return "Arbrook";
		case APIType::BGFX:
			return  "BGFX";
		case APIType::Native:
			return  "Native";
		default:
			return "None";
		}
		return "";
	}

	struct rendering_test
	{
		bool initialized = false;
		std::string name;
		virtual ~rendering_test() = default;
		virtual void setup(gfx::camera& cam, core::transform& camTransf) = 0;
		virtual void update(gfx::camera& cam, core::transform& camTransf) = 0;
		virtual void destroy() = 0;
	};
}

