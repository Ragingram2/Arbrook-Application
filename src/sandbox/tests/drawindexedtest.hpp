#pragma once
#include "sandbox/tests/rendertest.hpp"
#include "sandbox/tests/tools/bgfxutils.hpp"
#include "sandbox/tests/tools/CSVWriter.hpp"

namespace ast = rythe::core::assets;
namespace rythe::testing
{
	template<enum APIType type>
	struct DrawIndexedInstancedTest : public rendering_test { };

	template<>
	struct DrawIndexedInstancedTest<APIType::Arbrook> : public rendering_test
	{
		gfx::camera_data data;
		ast::asset_handle<gfx::material> mat;
		gfx::buffer_handle vBuffer;
		gfx::buffer_handle idxBuffer;
		gfx::buffer_handle cBuffer;
		ast::asset_handle<gfx::mesh> meshHandle;
		gfx::inputlayout layout;
		float i = 0;

		virtual void setup(gfx::camera& cam, core::transform& camTransf) override
		{
			name = "DrawIndexedInstanced";
			log::info("Initializing {}_Test{}", getAPIName(APIType::Arbrook), name);
			glfwSetWindowTitle(gfx::Renderer::RI->getGlfwWindow(), std::format("{}_Test{}", getAPIName(APIType::Arbrook), name).c_str());

			meshHandle = ast::AssetCache<gfx::mesh>::getAsset("teapot");
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
			math::vec3 pos = math::vec3{ 0, 0, 10.f };
			auto model = math::translate(math::mat4(1.0f), pos);
			model = math::rotate(model, math::radians(i), math::vec3(0.0f, 1.0f, 0.0f));
			data.model = model;

			ZoneScopedN("[Arbrook] Index Draw Test Update");
			{
				layout.bind();
				mat->shader->setData("CameraBuffer", &data);
				vBuffer->bind();
				idxBuffer->bind();
				{
					FrameClock clock(name, APIType::Arbrook, "DrawCallTime");
					gfx::Renderer::RI->drawIndexedInstanced(gfx::PrimitiveType::TRIANGLESLIST, meshHandle->indices.size(), 1, 0, 0, 0);
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
	struct DrawIndexedInstancedTest<APIType::BGFX> : public rendering_test
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

		virtual void setup(gfx::camera& cam, core::transform& camTransf) override
		{
			name = "DrawIndexedInstanced";
			log::info("Initializing {}_Test{}", getAPIName(APIType::BGFX), name);

			gfx::WindowProvider::activeWindow->initialize(math::ivec2(Screen_Width, Screen_Height), std::format("{}_Test{}", getAPIName(APIType::BGFX), name));
			gfx::WindowProvider::activeWindow->makeCurrent();

			meshHandle = ast::AssetCache<gfx::mesh>::getAsset("teapot");

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

			data.projection = cam.calculate_projection();

			inputLayout.begin().add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float).end();

			vertexBuffer = bgfx::createVertexBuffer(bgfx::makeRef(meshHandle->vertices.data(), meshHandle->vertices.size() * sizeof(math::vec4)), inputLayout);

			indexBuffer = bgfx::createIndexBuffer(bgfx::makeRef(meshHandle->indices.data(), meshHandle->indices.size() * sizeof(unsigned int)), BGFX_BUFFER_INDEX32);

			shader = loadProgram("testVS", "testFS");

			if (shader.idx == bgfx::kInvalidHandle)
				log::error("Shader failed to compile");

			data.view = cam.calculate_view(&camTransf);
			bgfx::setViewTransform(0, data.view.data, data.projection.data);

			initialized = true;
		}

		virtual void update(gfx::camera& cam, core::transform& camTransf) override
		{

			data.view = cam.calculate_view(&camTransf);
			i += .1f;
			math::vec3 pos = math::vec3{ 0, 0, 10.f };
			auto model = math::translate(math::mat4(1.0f), pos);
			model = math::rotate(model, math::radians(i), math::vec3(0.0f, 1.0f, 0.0f));

			ZoneScopedN("[BGFX] Index Draw Test Update");
			{
				bgfx::setViewTransform(0, data.view.data, data.projection.data);

				bgfx::touch(0);

				bgfx::setTransform(model.data);

				bgfx::setVertexBuffer(0, vertexBuffer);
				bgfx::setIndexBuffer(indexBuffer);
				bgfx::setState(state);
				{
					FrameClock clock(name, APIType::BGFX, "DrawCallTime");
					bgfx::submit(0, shader);
					bgfx::frame();
				}
			}
		}

		virtual void destroy() override
		{
			bgfx::destroy(indexBuffer);
			bgfx::destroy(vertexBuffer);
			bgfx::destroy(shader);
			bgfx::shutdown();

			gfx::WindowProvider::destroyWindow("BGFX");
			initialized = false;
		}
	};

#if RenderingAPI == RenderingAPI_OGL
	template<>
	struct DrawIndexedInstancedTest<APIType::Native> : public rendering_test
	{
		gfx::camera_data data;
		ast::asset_handle<gfx::mesh> meshHandle;
		ast::asset_handle<gfx::material> mat;

		unsigned int vboId;
		unsigned int vaoId;
		unsigned int eboId;
		unsigned int matrixBufferId;
		unsigned int constantBufferId;
		unsigned int shaderId;

		float i = 0;

		virtual void setup(gfx::camera& cam, core::transform& camTransf) override
		{
			name = "DrawIndexedInstanced";
			log::info("Initializing {}OGL_Test{}", getAPIName(APIType::Native), name);
			glfwSetWindowTitle(gfx::Renderer::RI->getGlfwWindow(), std::format("{}OGL_Test{}", getAPIName(APIType::Native), name).c_str());

			meshHandle = ast::AssetCache<gfx::mesh>::getAsset("teapot");
			mat = gfx::MaterialCache::loadMaterial("test", "color");

			shaderId = mat->shader->getId();

			glGenBuffers(1, &constantBufferId);
			glBindBuffer(GL_UNIFORM_BUFFER, constantBufferId);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(data), &data, GL_STATIC_DRAW);
			glBindBufferRange(GL_UNIFORM_BUFFER, 0, constantBufferId, 0, sizeof(data));
			glUseProgram(shaderId);
			glUniformBlockBinding(shaderId, glGetUniformBlockIndex(shaderId, "CameraBuffer"), 0);

			glGenBuffers(1, &vboId);
			glBindBuffer(GL_ARRAY_BUFFER, vboId);
			glBufferData(GL_ARRAY_BUFFER, meshHandle->vertices.size() * sizeof(math::vec4), meshHandle->vertices.data(), static_cast<GLenum>(gfx::UsageType::STATICDRAW));
			glGenBuffers(1, &eboId);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboId);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshHandle->indices.size() * sizeof(unsigned int), meshHandle->indices.data(), static_cast<GLenum>(gfx::UsageType::STATICDRAW));
			glUseProgram(shaderId);
			glGenVertexArrays(1, &vaoId);
			glBindVertexArray(vaoId);
			glBindBuffer(GL_ARRAY_BUFFER, vboId);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 4, static_cast<GLenum>(gfx::DataType::FLOAT), false, sizeof(math::vec4), reinterpret_cast<void*>(0));
			glVertexAttribDivisor(0, 0);

			glGenBuffers(1, &matrixBufferId);
			glBindBuffer(GL_ARRAY_BUFFER, matrixBufferId);
			glBufferData(GL_ARRAY_BUFFER, sizeof(math::mat4), nullptr, static_cast<GLenum>(gfx::UsageType::STATICDRAW));
			glEnableVertexAttribArray(2);
			glEnableVertexAttribArray(3);
			glEnableVertexAttribArray(4);
			glEnableVertexAttribArray(5);

			glVertexAttribPointer(2, 4, static_cast<GLenum>(gfx::DataType::FLOAT), false, sizeof(math::mat4), reinterpret_cast<void*>(0 * sizeof(math::vec4)));
			glVertexAttribPointer(3, 4, static_cast<GLenum>(gfx::DataType::FLOAT), false, sizeof(math::mat4), reinterpret_cast<void*>(1 * sizeof(math::vec4)));
			glVertexAttribPointer(4, 4, static_cast<GLenum>(gfx::DataType::FLOAT), false, sizeof(math::mat4), reinterpret_cast<void*>(2 * sizeof(math::vec4)));
			glVertexAttribPointer(5, 4, static_cast<GLenum>(gfx::DataType::FLOAT), false, sizeof(math::mat4), reinterpret_cast<void*>(3 * sizeof(math::vec4)));

			glVertexAttribDivisor(2, 1);
			glVertexAttribDivisor(3, 1);
			glVertexAttribDivisor(4, 1);
			glVertexAttribDivisor(5, 1);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboId);

			data.projection = cam.projection;
			data.view = cam.calculate_view(&camTransf);

			initialized = true;
		}

		virtual void update(gfx::camera& cam, core::transform& camTransf) override
		{
			data.view = cam.calculate_view(&camTransf);
			i += .1f;
			math::vec3 pos = math::vec3{ 0, 0, 10.0f };
			auto model = math::translate(math::mat4(1.0f), pos);
			model = math::rotate(model, math::radians(i), math::vec3(0.0f, 1.0f, 0.0f));
			data.model = model;

			{
				glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(gfx::camera_data), &data);
				FrameClock clock(name, APIType::Native, "DrawCallTime");
				glDrawElements(GL_TRIANGLES, meshHandle->indices.size(), GL_UNSIGNED_INT, reinterpret_cast <void*>(0));
			}

		}

		virtual void destroy() override
		{
			gfx::MaterialCache::deleteMaterial("test");
			glDeleteBuffers(1, &vboId);
			glDeleteBuffers(1, &eboId);
			glDeleteBuffers(1, &matrixBufferId);
			glDeleteBuffers(1, &constantBufferId);
			glDeleteVertexArrays(1, &vaoId);
			initialized = false;
		}
	};
#elif RenderingAPI == RenderingAPI_DX11
	template<>
	struct DrawIndexedInstancedTest<APIType::Native> : public rendering_test
	{
		gfx::camera_data data;
		ast::asset_handle<gfx::mesh> meshHandle;
		ast::asset_handle<gfx::material> mat;

		float i = 0;

		virtual void setup(gfx::camera& cam, core::transform& camTransf) override
		{
			name = "DrawIndexedInstanced";
			log::info("Initializing {}DX11_Test{}", getAPIName(APIType::Native), name);
			glfwSetWindowTitle(gfx::Renderer::RI->getGlfwWindow(), std::format("{}DX11_Test{}", getAPIName(APIType::Native), name).c_str());

			meshHandle = ast::AssetCache<gfx::mesh>::getAsset("teapot");
			mat = gfx::MaterialCache::loadMaterial("test", "color");

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