#pragma once
#include "sandbox/tests/rendertest.hpp"
#include "sandbox/tests/tools/bgfxutils.hpp"
#include "sandbox/tests/tools/CSVWriter.hpp"

namespace rythe::testing
{
	inline void iterateMaterialIndex(int& index, int size)
	{
		index++;
		if (index >= size)
			index = 0;
	}
	template<enum APIType type>
	struct MaterialSwitchTest : public rendering_test { };

	template<>
	struct MaterialSwitchTest<APIType::Arbrook> : public rendering_test
	{
		gfx::camera_data data;
		gfx::buffer_handle vBuffer;
		gfx::buffer_handle idxBuffer;
		gfx::buffer_handle cBuffer;
		gfx::inputlayout layout;
		ast::asset_handle<gfx::mesh> meshHandle;
		ast::asset_handle<gfx::material> currentMat;
		std::vector<ast::asset_handle<gfx::material>> materials;

		float i = 0;
		int matIdx = 0;

		virtual void setup(gfx::camera& cam, core::transform& camTransf) override
		{
			name = "MaterialSwitch";
			log::info("Initializing {}_Test{}", getAPIName(APIType::Arbrook), name);
			glfwSetWindowTitle(gfx::Renderer::RI->getGlfwWindow(), std::format("{}_Test{}", getAPIName(APIType::Arbrook), name).c_str());

			meshHandle = ast::AssetCache<gfx::mesh>::getAsset("beast");
			materials.push_back(gfx::MaterialCache::getMaterial("red"));
			materials.push_back(gfx::MaterialCache::getMaterial("green"));
			materials.push_back(gfx::MaterialCache::getMaterial("blue"));
			materials.push_back(gfx::MaterialCache::getMaterial("white"));

			vBuffer = gfx::BufferCache::createBuffer<math::vec4>("Vertex Buffer", gfx::TargetType::VERTEX_BUFFER, gfx::UsageType::STATICDRAW, meshHandle->vertices);
			idxBuffer = gfx::BufferCache::createBuffer<unsigned int>("Index Buffer", gfx::TargetType::INDEX_BUFFER, gfx::UsageType::STATICDRAW, meshHandle->indices);
			cBuffer = gfx::BufferCache::createConstantBuffer<gfx::camera_data>("CameraBuffer", 0, gfx::UsageType::STATICDRAW);

			for (auto mat : materials)
				mat->shader->addBuffer(cBuffer);

			idxBuffer->bind();
			layout.initialize(1, materials[0]->shader);
			layout.setAttributePtr(vBuffer, "POSITION", 0, gfx::FormatType::RGBA32F, 0, sizeof(math::vec4), 0);
			layout.submitAttributes();

			data.projection = cam.projection;
			data.view = cam.calculate_view(&camTransf);

			initialized = true;
		}

		virtual void update(gfx::camera& cam, core::transform& camTransf) override
		{
			iterateMaterialIndex(matIdx, materials.size());
			rotateModel(i, data);

			{
				FrameClock clock(name, APIType::Native, "Material Switch Time");
				currentMat = materials[matIdx];
				currentMat->bind();
				currentMat->shader->setData("CameraBuffer", &data);
			}

			layout.bind();
			vBuffer->bind();
			idxBuffer->bind();
			gfx::Renderer::RI->drawIndexedInstanced(gfx::PrimitiveType::TRIANGLESLIST, meshHandle->indices.size(), 1, 0, 0, 0);
		}

		virtual void destroy() override
		{
			gfx::BufferCache::deleteBuffer("Vertex Buffer");
			gfx::BufferCache::deleteBuffer("Index Buffer");
			gfx::BufferCache::deleteBuffer("CameraBuffer");
			layout.release();
			initialized = false;
		}
	};

	template<>
	struct MaterialSwitchTest<APIType::BGFX> : public rendering_test
	{
		gfx::camera_data data;
		ast::asset_handle<gfx::material> mat;
		ast::asset_handle<gfx::mesh> meshHandle;

#if RenderingAPI == RenderingAPI_OGL
		bgfx::RendererType::Enum type = bgfx::RendererType::OpenGL;
#elif RenderingAPI == RenderingAPI_DX11
		bgfx::RendererType::Enum type = bgfx::RendererType::Direct3D11;
#endif

		bgfx::PlatformData platformData;
		bgfx::DynamicVertexBufferHandle vertexBuffer;
		bgfx::DynamicIndexBufferHandle indexBuffer;
		bgfx::ProgramHandle currentShader;
		std::vector<bgfx::ProgramHandle> shaders;
		bgfx::VertexLayout inputLayout;

		BgfxCallback callback;
		uint64_t state = 0
			| BGFX_STATE_WRITE_MASK
			| BGFX_STATE_DEPTH_TEST_LESS
			| BGFX_STATE_FRONT_CCW
			| BGFX_STATE_CULL_CW
			| 0;

		int matIdx = 0;
		float i = 0;

		virtual void setup(gfx::camera& cam, core::transform& camTransf) override
		{
			name = "MaterialSwitch";
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
			init.resolution.reset = entry::s_reset;

#ifdef _DEBUG
			init.callback = &callback;
#endif

			if (!bgfx::init(init))
			{
				log::error("BGFX did not initialize properly");
			}

			bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x6495EDff, 1.0f, 0);
			bgfx::setViewMode(0, bgfx::ViewMode::Default);


			data.projection = cam.calculate_projection();

			inputLayout.begin().add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float).end();

			vertexBuffer = bgfx::createDynamicVertexBuffer(bgfx::makeRef(meshHandle->vertices.data(), meshHandle->vertices.size() * sizeof(math::vec4)), inputLayout, BGFX_BUFFER_ALLOW_RESIZE);

			indexBuffer = bgfx::createDynamicIndexBuffer(bgfx::makeRef(meshHandle->indices.data(), meshHandle->indices.size() * sizeof(unsigned int)), BGFX_BUFFER_INDEX32 | BGFX_BUFFER_ALLOW_RESIZE);

			shaders.push_back(loadProgram("defaultVS", "redFS"));
			shaders.push_back(loadProgram("defaultVS", "greenFS"));
			shaders.push_back(loadProgram("defaultVS", "blueFS"));
			shaders.push_back(loadProgram("defaultVS", "whiteFS"));

			data.view = cam.calculate_view(&camTransf);
			bgfx::setViewTransform(0, data.view.data, data.projection.data);
			bgfx::setViewRect(0, 0, 0, Screen_Width, Screen_Height);
			bgfx::frame();

			initialized = true;
		}

		virtual void update(gfx::camera& cam, core::transform& camTransf) override
		{
			data.view = cam.calculate_view(&camTransf);
			bgfx::setViewTransform(0, data.view.data, data.projection.data);
			bgfx::touch(0);

			iterateMaterialIndex(matIdx, shaders.size());

			currentShader = shaders[matIdx];
	
			rotateModel(i, data);
			bgfx::setTransform(data.model.data);
			bgfx::setVertexBuffer(0, vertexBuffer);
			bgfx::setIndexBuffer(indexBuffer);
			bgfx::setState(state);
			{
				FrameClock clock(name, APIType::Native, "Material Switch Time");
				bgfx::submit(0, currentShader);
			}
			bgfx::frame();
		}

		virtual void destroy() override
		{
			bgfx::destroy(indexBuffer);
			bgfx::destroy(vertexBuffer);
			for (auto shad : shaders)
			{
				bgfx::destroy(shad);
			}
			bgfx::shutdown();
			initialized = false;
		}
	};

#if RenderingAPI == RenderingAPI_OGL
	template<>
	struct MaterialSwitchTest<APIType::Native> : public rendering_test
	{
		gfx::camera_data data;
		ast::asset_handle<gfx::mesh> meshHandle;
		ast::asset_handle<gfx::material> currentMat;
		std::vector<ast::asset_handle<gfx::material>> materials;

		unsigned int vboId;
		unsigned int vaoId;
		unsigned int eboId;
		unsigned int constantBufferId;
		unsigned int currentShaderId;

		float i = 0;
		int matIdx = 0;

		virtual void setup(gfx::camera& cam, core::transform& camTransf) override
		{
			name = "MaterialSwitch";
			log::info("Initializing {}_Test{}", getAPIName(APIType::Native), name);
			glfwSetWindowTitle(gfx::Renderer::RI->getGlfwWindow(), std::format("{}_Test{}", getAPIName(APIType::Native), name).c_str());

			meshHandle = ast::AssetCache<gfx::mesh>::getAsset("beast");
			materials.push_back(gfx::MaterialCache::getMaterial("red"));
			materials.push_back(gfx::MaterialCache::getMaterial("green"));
			materials.push_back(gfx::MaterialCache::getMaterial("blue"));
			materials.push_back(gfx::MaterialCache::getMaterial("white"));

			currentShaderId = materials[matIdx]->shader->getId();

			glGenBuffers(1, &constantBufferId);
			glBindBuffer(GL_UNIFORM_BUFFER, constantBufferId);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(data), &data, GL_STATIC_DRAW);
			glBindBufferRange(GL_UNIFORM_BUFFER, 0, constantBufferId, 0, sizeof(data));

			for (auto& mat : materials)
			{
				glUseProgram(mat->shader->getId());
				glUniformBlockBinding(mat->shader->getId(), glGetUniformBlockIndex(mat->shader->getId(), "CameraBuffer"), 0);
			}

			glGenBuffers(1, &vboId);
			glBindBuffer(GL_ARRAY_BUFFER, vboId);
			glBufferData(GL_ARRAY_BUFFER, meshHandle->vertices.size() * sizeof(math::vec4), meshHandle->vertices.data(), static_cast<GLenum>(gfx::UsageType::STATICDRAW));
			glGenBuffers(1, &eboId);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboId);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshHandle->indices.size() * sizeof(unsigned int), meshHandle->indices.data(), static_cast<GLenum>(gfx::UsageType::STATICDRAW));

			glUseProgram(currentShaderId);
			glGenVertexArrays(1, &vaoId);
			glBindVertexArray(vaoId);
			glBindBuffer(GL_ARRAY_BUFFER, vboId);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 4, static_cast<GLenum>(gfx::DataType::FLOAT), false, sizeof(math::vec4), reinterpret_cast<void*>(0));
			glVertexAttribDivisor(0, 0);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboId);

			data.projection = cam.projection;
			data.view = cam.calculate_view(&camTransf);

			initialized = true;
		}

		virtual void update(gfx::camera& cam, core::transform& camTransf) override
		{
			data.view = cam.calculate_view(&camTransf);

			iterateMaterialIndex(matIdx, materials.size());
			rotateModel(i, data);

			{
				FrameClock clock(name, APIType::Native, "Material Switch Time");
				glUseProgram(materials[matIdx]->shader->getId());
				glBindBuffer(GL_UNIFORM_BUFFER, constantBufferId);
				glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(gfx::camera_data), &data);
			}

			glDrawElements(GL_TRIANGLES, meshHandle->indices.size(), GL_UNSIGNED_INT, reinterpret_cast <void*>(0));
		}

		virtual void destroy() override
		{
			glDeleteBuffers(1, &vboId);
			glDeleteBuffers(1, &eboId);
			glDeleteBuffers(1, &constantBufferId);
			glDeleteVertexArrays(1, &vaoId);
			initialized = false;
		}
	};

#elif RenderingAPI == RenderingAPI_DX11
	template<>
	struct MaterialSwitchTest<APIType::Native> : public rendering_test
	{
		gfx::camera_data data;

		ast::asset_handle<gfx::mesh> meshHandle;
		ast::asset_handle<gfx::material> currentMat;
		std::vector<ast::asset_handle<gfx::material>> materials;

		ID3D11Buffer* vertexBuffer;
		ID3D11Buffer* indexBuffer;
		ID3D11Buffer* constantBuffer;
		ID3D11InputLayout* inputLayout;
		ID3D11DeviceContext* deviceContext;
		ID3D11Device* device;

		ID3D11VertexShader* vertexShader;
		ID3D11PixelShader* pixelShader;

		float i = 0;
		int matIdx = 0;
		virtual void setup(gfx::camera& cam, core::transform& camTransf) override
		{
			name = "MaterialSwitch";
			log::info("Initializing {}_Test{}", getAPIName(APIType::Native), name);
			glfwSetWindowTitle(gfx::Renderer::RI->getGlfwWindow(), std::format("{}_Test{}", getAPIName(APIType::Native), name).c_str());
			device = gfx::WindowProvider::activeWindow->dev;
			deviceContext = gfx::WindowProvider::activeWindow->devcon;

			meshHandle = ast::AssetCache<gfx::mesh>::getAsset("beast");
			materials.push_back(gfx::MaterialCache::getMaterial("red"));
			materials.push_back(gfx::MaterialCache::getMaterial("green"));
			materials.push_back(gfx::MaterialCache::getMaterial("blue"));
			materials.push_back(gfx::MaterialCache::getMaterial("white"));

			data.projection = cam.calculate_projection();
			data.view = cam.calculate_view(&camTransf);

			//Create Camera Buffer
			D3D11_BUFFER_DESC bd = {};
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(gfx::camera_data);
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bd.CPUAccessFlags = 0;
			D3D11_SUBRESOURCE_DATA initData = {};
			initData.pSysMem = &data;
			CHECKERROR(device->CreateBuffer(&bd, &initData, &constantBuffer), "Failed to create Constant Buffer", gfx::Renderer::RI->checkError());

			deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);

			// Create the vertex buffer
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = meshHandle->vertices.size() * sizeof(math::vec4);
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = 0;
			initData.pSysMem = meshHandle->vertices.data();
			CHECKERROR(device->CreateBuffer(&bd, &initData, &vertexBuffer), "Failed to create Vertex Buffer", gfx::Renderer::RI->checkError());

			// Set the vertex buffer
			UINT stride = sizeof(math::vec4);
			UINT offset = 0;
			deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);


			// Create the index buffer
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = meshHandle->indices.size() * sizeof(unsigned int);
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bd.CPUAccessFlags = 0;
			initData.pSysMem = meshHandle->indices.data();
			CHECKERROR(device->CreateBuffer(&bd, &initData, &indexBuffer), "Failed to create Index Buffer", gfx::Renderer::RI->checkError());

			// Set the index buffer
			deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

			// Create and set the shaders and Set the input layout
			InitializeShadersAndLayout(device, deviceContext, inputLayout, materials[0]->shader);

			// Set primitive topology
			deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


			initialized = true;
		}

		virtual void update(gfx::camera& cam, core::transform& camTransf) override
		{
			iterateMaterialIndex(matIdx, materials.size());
			rotateModel(i, data);
			currentMat = materials[matIdx];

			{
				FrameClock clock(name, APIType::Native, "Material Switch Time");
				auto vtxBlob = currentMat->shader->getImpl().VS;
				auto pixBlob = currentMat->shader->getImpl().PS;
				// Vertex shader
				device->CreateVertexShader(vtxBlob->GetBufferPointer(), vtxBlob->GetBufferSize(), 0, &vertexShader);
				device->CreatePixelShader(pixBlob->GetBufferPointer(), pixBlob->GetBufferSize(), 0, &pixelShader);
				// Set the shaders
				deviceContext->VSSetShader(vertexShader, 0, 0);
				deviceContext->PSSetShader(pixelShader, 0, 0);

				deviceContext->UpdateSubresource(constantBuffer, 0, nullptr, &data, 0, 0);
				deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
			}


			deviceContext->DrawIndexed(meshHandle->indices.size(), 0, 0);

			vertexShader->Release();
			pixelShader->Release();
		}

		virtual void destroy() override
		{
			if (vertexBuffer) vertexBuffer->Release();
			if (indexBuffer) indexBuffer->Release();
			if (constantBuffer) constantBuffer->Release();
			if (inputLayout) inputLayout->Release();
			initialized = false;
		}
	};
#endif
}
