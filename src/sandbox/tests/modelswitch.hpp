#pragma once
#include "sandbox/tests/rendertest.hpp"
#include "sandbox/tests/tools/bgfxutils.hpp"

namespace rythe::testing
{
	template<enum APIType type>
	struct ModelSwitchTest : public rendering_test { };

	template<>
	struct ModelSwitchTest<APIType::Arbrook> : public rendering_test
	{
		gfx::camera_data data;
		gfx::material_handle mat;
		gfx::buffer_handle vBuffer;
		gfx::buffer_handle idxBuffer;
		gfx::buffer_handle cBuffer;
		gfx::mesh_handle meshHandle;
		gfx::inputlayout layout;
		float i = 0;
		int modelIdx = 0;
		std::vector<std::string> modelNames;

		void setup(gfx::camera& cam, core::transform& camTransf)
		{
			name = "ModelSwitch";
			log::debug("Initializing {}_Test{}", getAPIName(APIType::Arbrook), name);
			glfwSetWindowTitle(gfx::Renderer::RI->getGlfwWindow(), std::format("{}_Test{}", getAPIName(APIType::Arbrook), name).c_str());

			modelNames = gfx::ModelCache::getModelNames();

			meshHandle = gfx::ModelCache::getModel(modelNames[modelIdx])->meshHandle;
			mat = gfx::MaterialCache::loadMaterial("test", "cube");
			vBuffer = gfx::BufferCache::createBuffer<math::vec4>("Vertex Buffer", gfx::TargetType::VERTEX_BUFFER, gfx::UsageType::STATICDRAW, meshHandle->vertices);
			idxBuffer = gfx::BufferCache::createBuffer<unsigned int>("Index Buffer", gfx::TargetType::INDEX_BUFFER, gfx::UsageType::STATICDRAW, meshHandle->indices);
			cBuffer = gfx::BufferCache::createConstantBuffer<gfx::camera_data>("ConstantBuffer", 0, gfx::UsageType::STATICDRAW);
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

		void update(gfx::camera& cam, core::transform& camTransf)
		{
			data.view = cam.calculate_view(&camTransf);

			modelIdx++;

			if (modelIdx >= modelNames.size())
			{
				modelIdx = 0;
			}

			meshHandle = gfx::ModelCache::getModel(modelNames[modelIdx])->meshHandle;
			vBuffer->bufferData(meshHandle->vertices.data(), meshHandle->vertices.size());
			idxBuffer->bufferData(meshHandle->indices.data(), meshHandle->indices.size());

			layout.bind();
			i += .1f;
			math::vec3 pos = math::vec3{ 0, 0, 10.f };
			auto model = math::translate(math::mat4(1.0f), pos);
			model = math::rotate(model, math::radians(i), math::vec3(0.0f, 1.0f, 0.0f));
			data.model = model;

			mat->shader->setData("ConstantBuffer", &data);
			vBuffer->bind();
			idxBuffer->bind();
			gfx::Renderer::RI->drawIndexedInstanced(gfx::PrimitiveType::TRIANGLESLIST, meshHandle->indices.size(), 1, 0, 0, 0);
		}

		void destroy()
		{
			gfx::BufferCache::deleteBuffer("Vertex Buffer");
			gfx::BufferCache::deleteBuffer("Index Buffer");
			gfx::BufferCache::deleteBuffer("ConstantBuffer");
			gfx::MaterialCache::deleteMaterial("test");
			layout.release();
			initialized = false;
		}
	};

	template<>
	struct ModelSwitchTest<APIType::BGFX> : public rendering_test
	{
		gfx::camera_data data;
		gfx::material_handle mat;
		gfx::mesh_handle meshHandle;

#if RenderingAPI == RenderingAPI_OGL
		bgfx::RendererType::Enum type = bgfx::RendererType::OpenGL;
#elif RenderingAPI == RenderingAPI_DX11
		bgfx::RendererType::Enum type = bgfx::RendererType::Direct3D11;
#endif

		bgfx::PlatformData platformData;
		bgfx::VertexBufferHandle vertexBuffer;
		bgfx::IndexBufferHandle indexBuffer;
		bgfx::ProgramHandle shader;
		bgfx::VertexLayout inputLayout;

		BgfxCallback callback;
		uint64_t state = 0
			| BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			| BGFX_STATE_WRITE_Z
			| BGFX_STATE_FRONT_CCW
			| 0;

		float i = 0;
		int modelIdx = 0;
		std::vector<std::string> modelNames;

		void setup(gfx::camera& cam, core::transform& camTransf)
		{
			gfx::Renderer::RI->BGFXMode(true);
			name = "ModelSwitch";
			log::debug("Initializing {}_Test{}", getAPIName(APIType::BGFX), name);
			glfwSetWindowTitle(gfx::Renderer::RI->getGlfwWindow(), std::format("{}_Test{}", getAPIName(APIType::BGFX), name).c_str());
			modelNames = gfx::ModelCache::getModelNames();
			meshHandle = gfx::ModelCache::getModel(modelNames[modelIdx])->meshHandle;

			bgfx::Init init;
			init.type = type;

			init.platformData.ndt = nullptr;
			init.platformData.type = bgfx::NativeWindowHandleType::Default;
			init.platformData.nwh = glfwGetWin32Window(gfx::Renderer::RI->getGlfwWindow());
#if RenderingAPI == RenderingAPI_OGL
			init.platformData.context = wglGetCurrentContext();
#elif RenderingAPI == RenderingAPI_DX11
			init.platformData.context = gfx::Renderer::RI->getWindowHandle()->dev;
#endif
			init.resolution.width = gfx::Renderer::RI->getWindowHandle()->getResolution().x;
			init.resolution.height = gfx::Renderer::RI->getWindowHandle()->getResolution().y;

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

		void update(gfx::camera& cam, core::transform& camTransf)
		{
			data.view = cam.calculate_view(&camTransf);
			bgfx::setViewTransform(0, data.view.data, data.projection.data);

			i += .1f;
			bgfx::touch(0);

			math::vec3 pos = math::vec3{ 0, 0, 10.f };
			auto model = math::translate(math::mat4(1.0f), pos);
			model = math::rotate(model, math::radians(i), math::vec3(0.0f, 1.0f, 0.0f));
			bgfx::setTransform(model.data);

			bgfx::setVertexBuffer(0, vertexBuffer);
			bgfx::setIndexBuffer(indexBuffer);
			bgfx::setState(state);
			bgfx::submit(0, shader);
			bgfx::frame();
		}

		void destroy()
		{
			bgfx::destroy(indexBuffer);
			bgfx::destroy(vertexBuffer);
			bgfx::destroy(shader);
			bgfx::shutdown();
			gfx::Renderer::RI->BGFXMode(false);
			initialized = false;
		}
	};

#if RenderingAPI == RenderingAPI_OGL
	template<>
	struct ModelSwitchTest<APIType::Native> : public rendering_test
	{
		gfx::camera_data data;
		gfx::mesh_handle meshHandle;
		gfx::material_handle mat;

		unsigned int vboId;
		unsigned int vaoId;
		unsigned int eboId;
		unsigned int matrixBufferId;
		unsigned int constantBufferId;
		unsigned int shaderId;

		float i = 0;
		int modelIdx = 0;
		std::vector<std::string> modelNames;

		void setup(gfx::camera& cam, core::transform& camTransf)
		{
			name = "ModelSwitch";
			log::debug("Initializing {}OGL_Test{}", getAPIName(APIType::Native), name);
			glfwSetWindowTitle(gfx::Renderer::RI->getGlfwWindow(), std::format("{}OGL_Test{}", getAPIName(APIType::Native), name).c_str());

			modelNames = gfx::ModelCache::getModelNames();
			meshHandle = gfx::ModelCache::getModel(modelNames[modelIdx])->meshHandle;
			mat = gfx::MaterialCache::loadMaterial("test", "cube");

			shaderId = mat->shader->getId();

			glGenBuffers(1, &constantBufferId);
			glBindBuffer(GL_UNIFORM_BUFFER, constantBufferId);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(data), &data, GL_STATIC_DRAW);
			glBindBufferRange(GL_UNIFORM_BUFFER, 0, constantBufferId, 0, sizeof(data));
			glUseProgram(shaderId);
			glUniformBlockBinding(shaderId, glGetUniformBlockIndex(shaderId, "ConstantBuffer"), 0);

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

		void update(gfx::camera& cam, core::transform& camTransf)
		{
			modelIdx++;

			if (modelIdx >= modelNames.size())
			{
				modelIdx = 0;
			}

			meshHandle = gfx::ModelCache::getModel(modelNames[modelIdx])->meshHandle;

			glBindBuffer(GL_ARRAY_BUFFER, vboId);
			glBufferData(GL_ARRAY_BUFFER, meshHandle->vertices.size() * sizeof(math::vec4), meshHandle->vertices.data(), static_cast<GLenum>(gfx::UsageType::STATICDRAW));

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboId);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshHandle->indices.size() * sizeof(unsigned int), meshHandle->indices.data(), static_cast<GLenum>(gfx::UsageType::STATICDRAW));


			data.view = cam.calculate_view(&camTransf);
			i += .1f;

			math::vec3 pos = math::vec3{ 0, 0, 10.0f };
			auto model = math::translate(math::mat4(1.0f), pos);
			data.model = math::rotate(model, math::radians(i), math::vec3(0.0f, 1.0f, 0.0f));
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(gfx::camera_data), &data);
			glDrawElements(GL_TRIANGLES, meshHandle->indices.size(), GL_UNSIGNED_INT, reinterpret_cast <void*>(0));
		}

		void destroy()
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
	struct ModelSwitchTest<APIType::Native> : public rendering_test
	{
		gfx::camera_data data;
		gfx::material_handle mat;
		gfx::mesh_handle meshHandle;

		float i = 0;
		int modelIdx = 0;
		std::vector<std::string> modelNames;

		void setup(gfx::camera& cam, core::transform& camTransf)
		{
			name = "ModelSwitch";
			log::debug("Initializing {}DX11_Test{}", getAPIName(APIType::Native), name);
			glfwSetWindowTitle(gfx::Renderer::RI->getGlfwWindow(), std::format("{}DX11_Test{}", getAPIName(APIType::Native), name).c_str());

			initialized = true;
		}

		void update(gfx::camera& cam, core::transform& camTransf)
		{

		}

		void destroy()
		{

			initialized = false;
		}
		};
#endif
	}