#pragma once
#include "graphics/rendering.hpp"
#include "graphics/pipeline/base/pipeline.hpp"

#include "sandbox/tests/tools/bgfxutils.hpp"
#include "sandbox/tests/bgfxpipeline/bgfxrenderstage.hpp"

namespace rythe::testing
{
	class BGFXPipeline : public gfx::Pipeline<BGFXPipeline>
	{
		virtual void setup() override
		{
			attachStage<bgfx_render_stage>();
		}
	};
}