#pragma once
#include "sandbox/tests/rendertest.hpp"
#include "sandbox/tests/tools/bgfxutils.hpp"
#include "sandbox/tests/tools/CSVWriter.hpp"

namespace rythe::testing
{
	template<enum APIType type>
	struct StressTest : public rendering_test { };

	template<>
	struct StressTest<APIType::Arbrook> : public rendering_test
	{
		gfx::camera_data data;
		ast::asset_handle<gfx::material> mat;
		gfx::buffer_handle vBuffer;
		gfx::buffer_handle idxBuffer;
		gfx::buffer_handle cBuffer;
		ast::asset_handle<gfx::mesh> meshHandle;
		gfx::inputlayout layout;
		float i = 0;

		int renderCount = 10;
		float vertStep = 100.0f;
		float horStep = 100.0f;

		virtual void setup(gfx::camera& cam, core::transform& camTransf) override
		{
			name = "Stress";
			log::info("Initializing {}_Test{}", getAPIName(APIType::Arbrook), name);
			glfwSetWindowTitle(gfx::Renderer::RI->getGlfwWindow(), std::format("{}_Test{}", getAPIName(APIType::Arbrook), name).c_str());

			meshHandle = ast::AssetCache<gfx::mesh>::getAsset("beast");
			mat = gfx::MaterialCache::loadMaterial("test", "color");
			vBuffer = gfx::BufferCache::createBuffer<math::vec4>("Vertex Buffer", gfx::TargetType::VERTEX_BUFFER, gfx::UsageType::STATICDRAW, meshHandle->vertices);
			idxBuffer = gfx::BufferCache::createBuffer<unsigned int>("Index Buffer", gfx::TargetType::INDEX_BUFFER, gfx::UsageType::STATICDRAW, meshHandle->indices);
			cBuffer = gfx::BufferCache::createConstantBuffer<gfx::camera_data>("CameraBuffer", 0, gfx::UsageType::STATICDRAW);
			mat->shader->addBuffer(cBuffer);
			mat->bind();

			idxBuffer->bind();
			layout.initialize(1, mat->shader);
			layout.setAttributePtr(vBuffer, "POSITION", 0, gfx::FormatType::RGBA32F, 0, sizeof(math::vec4), 0);
			layout.submitAttributes();

			data.projection = cam.projection;
			data.view = cam.calculate_view(&camTransf);
			initialized = true;
		}

		virtual void update(gfx::camera& cam, core::transform& camTransf) override
		{
			data.view = cam.calculate_view(&camTransf);
			i += .1f;
			layout.bind();

			vBuffer->bind();
			idxBuffer->bind();

			float scaledHorStep = horStep / Screen_Width;
			float scaledVertStep = vertStep / Screen_Height;
			int width = 2;
			int height = 2;
			for (int x = 0; x < width; x++)
			{
				for (int y = 0; y < height; y++)
				{
					math::vec3 pos = math::vec3{ (x - (width / 2.0f)) * scaledHorStep, (y - (height / 2.0f)) * scaledVertStep, 10.f };
					auto model = math::translate(math::mat4(1.0f), pos);
					model = math::rotate(model, math::radians(i), math::vec3(0.0f, 1.0f, 0.0f));
					data.model = model;
					mat->shader->setData("CameraBuffer", &data);
					{
						FrameClock clock(name, APIType::Arbrook, "DrawCallTime");
						gfx::Renderer::RI->drawIndexed(gfx::PrimitiveType::TRIANGLESLIST, meshHandle->indices.size(), 0, 0);
					}
				}
			}
		}

		virtual void destroy() override
		{
			gfx::BufferCache::deleteBuffer("Vertex Buffer");
			gfx::BufferCache::deleteBuffer("Index Buffer");
			gfx::BufferCache::deleteBuffer("CameraBuffer");
			gfx::MaterialCache::deleteMaterial("test");
			layout.release();
			initialized = false;
		}
	};

	template<>
	struct StressTest<APIType::BGFX> : public rendering_test
	{
#if RenderingAPI == RenderingAPI_OGL
		bgfx::RendererType::Enum type = bgfx::RendererType::OpenGL;
#elif RenderingAPI == RenderingAPI_DX11
		bgfx::RendererType::Enum type = bgfx::RendererType::Direct3D11;
#endif

		gfx::camera_data data;
		ast::asset_handle<gfx::mesh> meshHandle;

		bgfx::PlatformData platformData;
		bgfx::VertexBufferHandle vertexBuffer;
		bgfx::IndexBufferHandle indexBuffer;
		bgfx::ProgramHandle shader;
		bgfx::VertexLayout inputLayout;

		BgfxCallback callback;
		uint64_t state = 0
			| BGFX_STATE_WRITE_MASK
			| BGFX_STATE_DEPTH_TEST_LESS
			| BGFX_STATE_FRONT_CCW
			| BGFX_STATE_CULL_CW
			| 0;

		float i = 0;

		int renderCount = 100;
		int vertStep = 10.0f;
		int horStep = 10.0f;

		virtual void setup(gfx::camera& cam, core::transform& camTransf) override
		{
			name = "Stress";
			log::info("Initializing {}_Test{}", getAPIName(APIType::BGFX), name);
			glfwSetWindowTitle(gfx::Renderer::RI->getGlfwWindow(), std::format("{}_Test{}", getAPIName(APIType::BGFX), name).c_str());

			gfx::WindowProvider::activeWindow->initialize(math::ivec2(Screen_Width, Screen_Height), std::format("{}_Test{}", getAPIName(APIType::BGFX), name));
			gfx::WindowProvider::activeWindow->makeCurrent();

			meshHandle = ast::AssetCache<gfx::mesh>::getAsset("beast");

			bgfx::Init init;
			init.type = type;

			init.platformData.nwh = glfwGetWin32Window(gfx::WindowProvider::activeWindow->getGlfwWindow());
			init.platformData.ndt = nullptr;
#ifdef RenderingAPI_DX11
			init.platformData.context = gfx::WindowProvider::activeWindow->dev;
#endif
			init.platformData.type = bgfx::NativeWindowHandleType::Default;
			init.resolution.width = gfx::WindowProvider::activeWindow->getResolution().x;
			init.resolution.height = gfx::WindowProvider::activeWindow->getResolution().y;

#ifdef _DEBUG
			init.callback = &callback;
#endif

			if (!bgfx::init(init))
			{
				log::error("BGFX did not initialize properly");
			}

			bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x6495EDff, 1.0f, 0);
			bgfx::setViewMode(0, bgfx::ViewMode::Default);
			bgfx::setViewRect(0, 0, 0, Screen_Width, Screen_Height);

			initialized = true;
		}

		virtual void update(gfx::camera& cam, core::transform& camTransf) override
		{

		}

		virtual void destroy() override
		{
			//bgfx::destroy(indexBuffer);
			//bgfx::destroy(vertexBuffer);
			//bgfx::destroy(shader);
			bgfx::shutdown();
			gfx::WindowProvider::destroyWindow("BGFX");
			initialized = false;
		}
	};

#if RenderingAPI == RenderingAPI_OGL
	template<>
	struct StressTest<APIType::Native> : public rendering_test
	{
		gfx::camera_data data;
		ast::asset_handle<gfx::material> mat;
		ast::asset_handle<gfx::mesh> meshHandle;


		float i = 0;

		int renderCount = 100;
		int vertStep = 10.0f;
		int horStep = 10.0f;

		virtual void setup(gfx::camera& cam, core::transform& camTransf) override
		{
			name = "Stress";
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
	struct StressTest<APIType::Native> : public rendering_test
	{
		gfx::camera_data data;
		ast::asset_handle<gfx::material> mat;
		ast::asset_handle<gfx::mesh> meshHandle;

		float i = 0;

		int renderCount = 100;
		int vertStep = 10.0f;
		int horStep = 10.0f;

		virtual void setup(gfx::camera& cam, core::transform& camTransf) override
		{
			name = "Stress";
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