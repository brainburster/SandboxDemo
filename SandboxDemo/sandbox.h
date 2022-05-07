#pragma once

#include "dx11_wnd.hpp"
#include "Sandbox_impl.h"

#include "imgui.h"
#include <memory>
#include <vector>

class Sandbox final
{
private:
	std::shared_ptr<DX11_Wnd> dx11_wnd;
	SandboxImpl impl;

	struct VsIn
	{
		XMFLOAT2 pos;
		XMFLOAT2 uv;
		static constexpr UINT num_elements = 2;
		static constexpr D3D11_INPUT_ELEMENT_DESC  input_layout[num_elements] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,           D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	};
	static constexpr VsIn vertices[4] = {
		{ XMFLOAT2{1.f, -1.f}, XMFLOAT2{1.f,1.f} },
		{ XMFLOAT2{1.f, 1.f }, XMFLOAT2{1.f,0.f} },
		{ XMFLOAT2{-1.f,-1.f}, XMFLOAT2{0.f,1.f} },
		{ XMFLOAT2{-1.f, 1.f}, XMFLOAT2{0.f,0.f} },
	};
	enum
	{
		grid_size = SandboxSetting::GlobalSetting::grid_size,
		grid_height = SandboxSetting::GlobalSetting::grid_height,
		grid_width = SandboxSetting::GlobalSetting::grid_width,
	};

	//顶点缓冲、输入布局，以及顶点、像素着色器
	ComPtr<ID3D11Buffer> vertex_buffer;
	ComPtr<ID3D11InputLayout> input_layout;
	ComPtr<ID3D11VertexShader> vs;
	ComPtr<ID3D11PixelShader> ps;
	ComPtr<ID3D11BlendState> bs;

	//cbuffer
	struct PerFrame
	{
		int iResolution[2];
		int iMouse[2];
		int iGridSize;
		float BrushSize;
		int iShowVelocity;
	};
	ComPtr<ID3D11Buffer> cbuf_perframe;

	//显示用纹理，每帧从CPU复制到GPU上
	ComPtr<ID3D11Texture2D> tex_display;
	ComPtr<ID3D11ShaderResourceView> srv_tex_display;
	ComPtr<ID3D11Texture2D> tex_info;
	ComPtr<ID3D11ShaderResourceView> srv_tex_info;
public:
	//
	Sandbox(decltype(dx11_wnd) wnd);
	~Sandbox();
	void run();
private:
	void wait_for_gpu();
	void init();
	void init_imgui();
	void update();
	void update_cbuf();
	void handle_input();
	void draw_line();
	void before_print_to_screen();
	void print_to_screen();
	void after_print_to_screen();
	void set_input_callback();
	void init_resources();
	void init_shaders();
};
