﻿#pragma once

#include "raw_wnd.hpp"

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

using namespace DirectX;
using namespace Microsoft::WRL;

class DX11_Wnd : public Wnd<DX11_Wnd>
{
protected:
	ComPtr<ID3D11Device> device;
	ComPtr<IDXGISwapChain> swap_chain;
	ComPtr<ID3D11DeviceContext> context;
	ComPtr<ID3D11RenderTargetView> rtv;
	ComPtr<ID3D11DepthStencilView> dsv;
	ComPtr<ID3D11Texture2D> depth_tex;
	ComPtr<ID3D11RasterizerState> ras_state;
public:

	const auto& GetDevice() const { return device; }
	const auto& GetSwapChain() const { return swap_chain; }
	const auto& GetImCtx() const { return context; }
	const auto& GetRTV() const { return rtv; }
	const auto& GetDsv() const { return dsv; }
	DX11_Wnd(HINSTANCE hinst) : Wnd{ hinst } {};
	DX11_Wnd& Init()
	{
		Wnd::Init();
		//...
		{
			D3D_FEATURE_LEVEL featureLevel;
			DXGI_SWAP_CHAIN_DESC swap_chain_desc{};

			swap_chain_desc.BufferDesc.Width = m_width;
			swap_chain_desc.BufferDesc.Height = m_height;
			swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
			swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
			swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

			swap_chain_desc.SampleDesc.Count = 4;
			swap_chain_desc.SampleDesc.Quality = 0;

			swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

			swap_chain_desc.BufferCount = 2;
			swap_chain_desc.OutputWindow = m_hwnd;
			swap_chain_desc.Windowed = true;
			swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			swap_chain_desc.Flags = 0;

			D3D_FEATURE_LEVEL featureLevels[] =
			{
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_10_1,
				D3D_FEATURE_LEVEL_10_0,
			};
			UINT numFeatureLevels = ARRAYSIZE(featureLevels);

			HRESULT hr = D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, 0, featureLevels, numFeatureLevels,
				D3D11_SDK_VERSION, &swap_chain_desc, swap_chain.GetAddressOf(), device.GetAddressOf(), &featureLevel, context.GetAddressOf());
			assert(SUCCEEDED(hr));
			assert(featureLevel == D3D_FEATURE_LEVEL_11_0);
		}

		{
			ID3D11Texture2D* pBuffer = nullptr;
			HRESULT hr = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)(&pBuffer));
			assert(SUCCEEDED(hr));
			if (!pBuffer) { return *this; }
			hr = device->CreateRenderTargetView(pBuffer, 0, rtv.GetAddressOf());
			pBuffer->Release();
			assert(SUCCEEDED(hr));
			D3D11_TEXTURE2D_DESC ds_desc{};
			ds_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			ds_desc.Width = m_width;
			ds_desc.Height = m_height;
			ds_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			ds_desc.MipLevels = 1;
			ds_desc.ArraySize = 1;
			ds_desc.CPUAccessFlags = 0;
			ds_desc.SampleDesc.Count = 4;
			ds_desc.SampleDesc.Quality = 0;
			ds_desc.Usage = D3D11_USAGE_DEFAULT;
			hr = device->CreateTexture2D(&ds_desc, 0, depth_tex.GetAddressOf());
			assert(SUCCEEDED(hr));
			D3D11_DEPTH_STENCIL_VIEW_DESC  dsv_desc{};
			dsv_desc.Format = ds_desc.Format;
			dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
			hr = device->CreateDepthStencilView(depth_tex.Get(), &dsv_desc, dsv.GetAddressOf());
			assert(SUCCEEDED(hr));
			context->OMSetRenderTargets(1, rtv.GetAddressOf(), dsv.Get());
		}

		{
			D3D11_RASTERIZER_DESC raster_desc = {};
			raster_desc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
			raster_desc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
			raster_desc.FrontCounterClockwise = TRUE;
			raster_desc.DepthClipEnable = FALSE;
			raster_desc.MultisampleEnable = FALSE;
			raster_desc.DepthBiasClamp = 0;
			raster_desc.SlopeScaledDepthBias = 0;

			HRESULT hr = device->CreateRasterizerState(&raster_desc, ras_state.GetAddressOf());
			assert(SUCCEEDED(hr));
			D3D11_VIEWPORT vp{};
			vp.Width = (FLOAT)m_width;
			vp.Height = (FLOAT)m_height;
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;
			context->RSSetState(ras_state.Get());
			context->RSSetViewports(1, &vp);
		}
		return *this;
	}
};
