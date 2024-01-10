#pragma once
#include "sandbox/tests/rendertest.hpp"
#include "sandbox/tests/tools/bgfxutils.hpp"
#include "sandbox/tests/tools/CSVWriter.hpp"

namespace rythe::testing
{
	template<enum APIType type>
	struct BufferCreationTest : public rendering_test { };

	template<>
	struct BufferCreationTest<APIType::Arbrook> : public rendering_test
	{
		gfx::camera_data data;
		ast::asset_handle<gfx::material> mat;
		gfx::buffer_handle vBuffer;
		gfx::buffer_handle idxBuffer;
		gfx::buffer_handle cBuffer;
		ast::asset_handle<gfx::mesh> meshHandle;
		gfx::inputlayout layout;


		virtual void setup(gfx::camera& cam, core::transform& camTransf) override
		{
			name = "BufferCreation";
			log::info("Initializing {}_Test{}", getAPIName(APIType::Arbrook), name);
			glfwSetWindowTitle(gfx::Renderer::RI->getGlfwWindow(), std::format("{}_Test{}", getAPIName(APIType::Arbrook), name).c_str());
			initialized = true;
		}

		virtual void update(gfx::camera& cam, core::transform& camTransf) override
		{

		}

		virtual void destroy() override
		{
			initialized = false;
		}
	};

	template<>
	struct BufferCreationTest<APIType::BGFX> : public rendering_test
	{
		gfx::camera_data data;
		ast::asset_handle<gfx::material> mat;
		ast::asset_handle<gfx::mesh> meshHandle;


		virtual void setup(gfx::camera& cam, core::transform& camTransf) override
		{
			name = "BufferCreation";
			log::info("Initializing {}_Test{}", getAPIName(APIType::BGFX), name);
			glfwSetWindowTitle(gfx::Renderer::RI->getGlfwWindow(), std::format("{}_Test{}", getAPIName(APIType::BGFX), name).c_str());
			initialized = true;
		}

		virtual void update(gfx::camera& cam, core::transform& camTransf) override
		{

		}

		virtual void destroy() override
		{
			initialized = false;
		}
	};

#if RenderingAPI == RenderingAPI_OGL
	template<>
	struct BufferCreationTest<APIType::Native> : public rendering_test
	{
		gfx::camera_data data;
		ast::asset_handle<gfx::material> mat;
		ast::asset_handle<gfx::mesh> meshHandle;


		virtual void setup(gfx::camera& cam, core::transform& camTransf) override
		{
			name = "BufferCreation";
			log::info("Initializing {}_Test{}", getAPIName(APIType::Native), name);
			glfwSetWindowTitle(gfx::Renderer::RI->getGlfwWindow(), std::format("{}_Test{}", getAPIName(APIType::Native), name).c_str());
			initialized = true;
		}

		virtual void update(gfx::camera& cam, core::transform& camTransf) override
		{

		}

		virtual void destroy() override
		{
			initialized = false;
		}
	};

#elif RenderingAPI == RenderingAPI_DX11
	template<>
	struct BufferCreationTest<APIType::Native> : public rendering_test
	{
		gfx::camera_data data;
		ast::asset_handle<gfx::material> mat;
		ast::asset_handle<gfx::mesh> meshHandle;


		virtual void setup(gfx::camera& cam, core::transform& camTransf) override
		{
			name = "BufferCreation";
			log::info("Initializing {}_Test{}", getAPIName(APIType::Native), name);
			glfwSetWindowTitle(gfx::Renderer::RI->getGlfwWindow(), std::format("{}_Test{}", getAPIName(APIType::Native), name).c_str());
			initialized = true;
		}

		virtual void update(gfx::camera& cam, core::transform& camTransf) override
		{

		}

		virtual void destroy() override
		{
			initialized = false;
		}
	};
#endif
}