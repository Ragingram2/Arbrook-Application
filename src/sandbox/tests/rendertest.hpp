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
#ifdef _DEBUG
		int maxIterations = 1000;
#else
		int maxIterations = 10000;
#endif
		bool initialized = false;
		std::string name;
		virtual ~rendering_test() = default;
		virtual void setup(gfx::camera& cam, core::transform& camTransf) = 0;
		virtual void update(gfx::camera& cam, core::transform& camTransf) = 0;
		virtual void destroy() = 0;
	};

#ifdef RenderingAPI_DX11
	inline void InitializeShadersAndLayout(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ID3D11InputLayout* inputLayout, ast::asset_handle<gfx::shader> shader)
	{
		ID3D11VertexShader* vertexShader;
		ID3D11PixelShader* pixelShader;

		auto vtxBlob = shader->getImpl().VS;
		auto pixBlob = shader->getImpl().PS;
		// Vertex shader
		device->CreateVertexShader(vtxBlob->GetBufferPointer(), vtxBlob->GetBufferSize(), 0, &vertexShader);
		device->CreatePixelShader(pixBlob->GetBufferPointer(), pixBlob->GetBufferSize(), 0, &pixelShader);

		// Set the shaders
		deviceContext->VSSetShader(vertexShader, 0, 0);
		deviceContext->PSSetShader(pixelShader, 0, 0);

		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		device->CreateInputLayout(layout, sizeof(layout) / sizeof(layout[0]), vtxBlob->GetBufferPointer(), vtxBlob->GetBufferSize(), &inputLayout);

		// Set the input layout
		deviceContext->IASetInputLayout(inputLayout);
	}
#endif
}

