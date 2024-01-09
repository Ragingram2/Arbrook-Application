#pragma once
#include <rsl/primitives>
#include <rsl/utilities>
#include <rsl/math>
#include <rsl/logging>

#include "core/core.hpp"
#include "graphics/rendering.hpp"
#include "graphics/cache/cache.hpp"
#include "graphics/pipeline/base/graphicsstage.hpp"
#include "graphics/components/components.hpp"

namespace ast = rythe::core::assets;
namespace rythe::testing
{
	using setupFunc = void();
	using renderFunc = void();

	struct bgfx_render_stage : public gfx::graphics_stage<bgfx_render_stage>
	{
	private:
		static rsl::multicast_delegate<renderFunc> m_onRender;
	public:
		virtual void setup(core::transform camTransf, gfx::camera& cam) override
		{
		}

		virtual void render(core::transform camTransf, gfx::camera& cam) override
		{
			m_onRender();
		}

		virtual rsl::priority_type priority() override { return OPAQUE_PRIORITY; }


		template <class T, void(T::* Func)()>
		static void addRender(T* ptr)
		{
			m_onRender.push_back<T, Func>(*ptr);
		}
	};

	inline rsl::multicast_delegate<renderFunc> bgfx_render_stage::m_onRender;
}