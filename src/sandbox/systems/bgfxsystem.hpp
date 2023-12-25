#pragma once
#include <string>
#include <format>
#include <tracy/Tracy.hpp>

#include "core/core.hpp"
#include "rendering/rendering.hpp"
#include "input/input.hpp"

#include "sandbox/tests/tools/bgfxutils.hpp"

namespace rythe::game
{
	using namespace testing;
	class BgfxSystem : public core::System<BgfxSystem,gfx::camera,core::transform>
	{
	private:
		GLFWwindow* m_glfwWindow;
		gfx::window_handle m_winHandle;
		core::ecs::entity camEntity;
		gfx::camera cam;
		core::transform camTransf;
#if RenderingAPI == RenderingAPI_OGL
		bgfx::RendererType::Enum type = bgfx::RendererType::OpenGL;
#elif RenderingAPI == RenderingAPI_DX11
		bgfx::RendererType::Enum type = bgfx::RendererType::Direct3D11;
#endif
		gfx::camera_data data;
		gfx::mesh_handle meshHandle;

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
	public:
		void setup()
		{
			m_winHandle = gfx::WindowProvider::addWindow();
			m_winHandle->initialize(math::ivec2(Screen_Width,Screen_Height),"Arbrook");


#if RenderingAPI == RenderingAPI_OGL
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#elif RenderingAPI == RenderingAPI_DX11

#endif

			meshHandle = gfx::MeshCache::loadMesh("teapot","resources/meshes/glb/teapot.glb");

			bgfx::Init init;
			init.type = type;

			init.platformData.ndt = nullptr;
			init.platformData.type = bgfx::NativeWindowHandleType::Default;
			init.platformData.nwh = glfwGetWin32Window(m_winHandle->getGlfwWindow());
#if RenderingAPI == RenderingAPI_OGL
			init.platformData.context = wglGetCurrentContext();
#elif RenderingAPI == RenderingAPI_DX11
			init.platformData.context = gfx::Renderer::RI->getWindowHandle()->dev;
#endif
			init.resolution.width = Screen_Width;
			init.resolution.height = Screen_Height;

#ifdef _DEBUG
			init.callback = &callback;
#endif

			if (!bgfx::init(init))
			{
				log::error("BGFX did not initialize properly");
			}

			bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x6495EDff, 1.0f, 0);
			bgfx::setViewMode(0, bgfx::ViewMode::Sequential);
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

		}

		void update()
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
	};
}