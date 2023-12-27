#pragma once
#include <string>

#include <rsl/math>

#include "graphics/interface/definitions/definitions.hpp"
#include "graphics/rendering.hpp"

namespace rythe::testing
{
	enum APIType
	{
		Arbrook = 0,
		Native = 1,
		BGFX = 2,
		None = 3
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
		case APIType::None:
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

//namespace std {
//	template <> struct hash<rythe::testing::APIType> {
//		size_t operator() (const rythe::testing::APIType& t) const { return size_t(t); }
//	};
//}

