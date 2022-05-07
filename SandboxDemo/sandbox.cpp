#include "Sandbox.h"

#include <d3dcompiler.h>

#include <iostream>

#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

Sandbox::Sandbox(decltype(dx11_wnd) wnd) : dx11_wnd{wnd}, impl{} {
  size_t height = wnd->getHeight() / SandboxSetting::GlobalSetting::grid_size;
  size_t width = wnd->getWidth() / SandboxSetting::GlobalSetting::grid_size;
}

Sandbox::~Sandbox() {
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();
}

void Sandbox::run() {
  init();
  set_input_callback();
  DX11_Wnd& wnd = *dx11_wnd;
  while (!wnd.app_should_close()) {
    wnd.PeekMsg();
    update();
    before_print_to_screen();
    print_to_screen();
    after_print_to_screen();
  }
}

void Sandbox::wait_for_gpu() {
  decltype(auto) device = dx11_wnd->GetDevice();
  decltype(auto) ctx = dx11_wnd->GetImCtx();
  ComPtr<ID3D11Query> event_query{};
  D3D11_QUERY_DESC queryDesc{};
  queryDesc.Query = D3D11_QUERY_EVENT;
  queryDesc.MiscFlags = 0;
  device->CreateQuery(&queryDesc, event_query.GetAddressOf());
  ctx->End(event_query.Get());
  while (ctx->GetData(event_query.Get(), NULL, 0, 0) == S_FALSE) {
  }
}

void Sandbox::init() {
  init_shaders();
  init_resources();
  impl.init();
  init_imgui();
}

void Sandbox::init_imgui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
                                           // io.ConfigFlags |=
                                           // ImGuiConfigFlags_NavEnableGamepad;
                                           // // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;    // Enable Docking
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  // Enable Multi-Viewport
                                                       // / Platform Windows
  ImGui::StyleColorsDark();
  ImGuiStyle& style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }
  bool result = ImGui_ImplWin32_Init(dx11_wnd->Hwnd());
  assert(result && "failed to Init ImGui_Implwin32");
  result = ImGui_ImplDX11_Init(dx11_wnd->GetDevice().Get(),
                               dx11_wnd->GetImCtx().Get());
  assert(result && "failed to Init ImGui_ImplDX11");
  io.Fonts->AddFontFromFileTTF(
      "resource/fonts/SourceHanSansCN-Medium.otf", 18.f, nullptr,
      io.Fonts
          ->GetGlyphRangesChineseSimplifiedCommon());  // GetGlyphRangesChineseFull
}

void Sandbox::update() {
  handle_input();
  update_cbuf();
  impl.update();
}

void Sandbox::update_cbuf() {
  decltype(auto) ctx = dx11_wnd->GetImCtx();
  D3D11_MAPPED_SUBRESOURCE mapped_subresource = {};
  HRESULT hr = ctx->Map(cbuf_perframe.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0,
                        &mapped_subresource);
  assert(SUCCEEDED(hr));
  PerFrame* p_data = (PerFrame*)mapped_subresource.pData;

  const HWND hwnd = dx11_wnd->Hwnd();
  POINT point{};
  GetCursorPos(&point);
  ::ScreenToClient(hwnd, &point);
  RECT rect{};
  GetClientRect(hwnd, &rect);
  PerFrame pf = {
      .iResolution = {rect.right, rect.bottom},
      .iMouse = {point.x, point.y},
      .iGridSize = SandboxSetting::GlobalSetting::grid_size,
      .BrushSize = SandboxSetting::GlobalSetting::brush_size(),
      .iShowVelocity = SandboxSetting::GlobalSetting::show_velocity()};
  memcpy_s(p_data, 32, &pf, sizeof(pf));
  ctx->Unmap(cbuf_perframe.Get(), 0);
}

void Sandbox::handle_input() { draw_line(); }

void Sandbox::draw_line() {
  auto& io = ImGui::GetIO();
  // bresenham's line算法,
  // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
  static const auto plot_line = [&](int x0, int y0, int x1, int y1) {
    const int dx = abs(x1 - x0);
    const int dy = -abs(y1 - y0);
    const int sx = x0 < x1 ? 1 : -1;
    const int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy; /* error value e_xy */
    while (true) {
      impl.set(x0, y0);
      if (x0 == x1 && y0 == y1) break;
      const int e2 = 2 * err;
      /* e_xy+e_x > 0 */
      if (e2 >= dy) {
        err += dy;
        x0 += sx;
      }
      /* e_xy+e_y < 0 */
      if (e2 <= dx) {
        err += dx;
        y0 += sy;
      }
    }
  };
  POINT pos = {(long)io.MousePos.x, (long)io.MousePos.y};
  ScreenToClient(dx11_wnd->Hwnd(), &pos);
  static POINT last = {-1, -1};
  if (!io.WantCaptureMouse && (io.MouseDown[0] || io.MouseDown[1])) {
    SandboxSetting::GlobalSetting::use_eraser() = io.MouseDown[1];
    int x = pos.x / grid_size;
    int y = pos.y / grid_size;
    if (last.x < 0) {
      impl.set(x, y);
    } else {
      plot_line(last.x, last.y, x, y);
    }
    last = {x, y};
  } else {
    last = {-1, -1};
  }
}

void Sandbox::before_print_to_screen() {
  const auto& ctx = dx11_wnd->GetImCtx();

  //更新tex_display
  {
    HRESULT hr = NULL;
    D3D11_MAPPED_SUBRESOURCE mapped_data{};
    // wait_for_gpu();
    hr = ctx->Map(tex_display.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0,
                  &mapped_data);
    assert((SUCCEEDED(hr)) && "failed to map tex_display");
    BYTE* p_data = reinterpret_cast<BYTE*>(mapped_data.pData);
    // D3D11_TEXTURE2D_DESC desc{};
    // tex_display->GetDesc(&desc);
    // memcpy_s(p_data, 4ULL * desc.Width * desc.Height, display_buffer.data(),
    // display_buffer.size() * sizeof(decltype(display_buffer.data()[0])));
    const size_t width = SandboxGrid::w;
    const size_t height = SandboxGrid::h;
    auto& grid = impl.get_grid();
    for (size_t i = 0; i < height; ++i) {
      if (i * width + width > grid.visualization.buffer.size()) {
        break;
      }
      memcpy_s(p_data, 4ULL * width, &grid.visualization.buffer[i * width],
               4ULL * width);
      p_data += mapped_data.RowPitch;
    }

    ctx->Unmap(tex_display.Get(), 0);
  }

  //更新tex_info
  {
    // HRESULT hr = NULL;
    // D3D11_MAPPED_SUBRESOURCE mapped_data{};
    // wait_for_gpu();
    // hr = ctx->Map(tex_info.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0,
    // &mapped_data); assert((SUCCEEDED(hr)) && "failed to map tex_info"); BYTE*
    // p_data = reinterpret_cast<BYTE*>(mapped_data.pData); const int width =
    // SandboxGrid::w; const int height = SandboxGrid::h; auto& grid =
    // impl.get_grid(); for (int j = 0; j < height; ++j)
    //{
    //	for (int i = 0; i < width; ++i)
    //	{
    //		Vec2 u = grid.fluids[0].first.Get(i, j).u;
    //		u += grid.fluids[1].first.Get(i, j).u;
    //		u += grid.fluids[2].first.Get(i, j).u;
    //		float show = grid.solid.first.Get(i, j) & 0b00111111 ? 0.f
    //: 1.f; 		Color4f data = { u.x,u.y,show,0.f };
    //: memcpy_s(p_data + 16 * i,
    // 16ULL, &data, sizeof(data));
    //	}
    //	p_data += mapped_data.RowPitch;
    //  }

    // ctx->Unmap(tex_info.Get(), 0);
  }

  // imgui
  static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  {
    static bool show_demo_window = true;
    static bool show_another_window = false;
    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // 1. Show the big demo window (Most of the sample code is in
    // ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear
    // ImGui!).
    if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair
    // to created a named window.
    {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin("Hello, world!");  // Create a window called "Hello, world!"
                                      // and append into it.

      ImGui::Text("This is some useful text.");  // Display some text (you can
                                                 // use a format strings too)
      ImGui::Checkbox(
          "Demo Window",
          &show_demo_window);  // Edit bools storing our window open/close state
      ImGui::Checkbox("Another Window", &show_another_window);

      ImGui::SliderFloat(
          "float", &f, 0.0f,
          1.0f);  // Edit 1 float using a slider from 0.0f to 1.0f
      ImGui::ColorEdit3(
          "clear color",
          (float*)&clear_color);  // Edit 3 floats representing a color

      if (ImGui::Button(
              "Button"))  // Buttons return true when clicked (most widgets
                          // return true when edited/activated)
        counter++;
      ImGui::SameLine();
      ImGui::Text("counter = %d", counter);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window) {
      ImGui::Begin(
          "Another Window",
          &show_another_window);  // Pass a pointer to our bool variable (the
                                  // window will have a closing button that will
                                  // clear the bool when clicked)
      ImGui::Text("Hello from another window!");
      if (ImGui::Button("Close Me")) show_another_window = false;
      ImGui::End();
    }

    ImGui::Begin(U8("工具"));
    if (ImGui::TreeNode(U8("笔刷"))) {
      static const char* items[] = {
          U8("墙"),   U8("橡皮"), U8("水"), U8("油"),   U8("空气"), U8("沙子"),
          U8("石头"), U8("食盐"), U8("糖"), U8("加热"), U8("降温")};
      ImGui::Combo(U8("选择笔刷"),
                   &SandboxSetting::GlobalSetting::current_brush(), items,
                   IM_ARRAYSIZE(items) /*, 5*/);
      ImGui::SliderFloat(U8("笔刷大小"),
                         &SandboxSetting::GlobalSetting::brush_size(), 1.0f,
                         100.0f);
      ImGui::TreePop();
    }
    if (ImGui::TreeNode(U8("全局变量"))) {
      ImGui::SliderFloat2(
          U8("重力"),
          &reinterpret_cast<float&>(SandboxSetting::GlobalSetting::g()), -0.1f,
          0.1f, "%.4f" /*, .5f*/);
      ImGui::SliderFloat(U8("背景温度"), &SandboxSetting::GlobalSetting::T(),
                         0.f, 1000.f, "%.2fK");
      ImGui::TreePop();
    }
    ImGui::Text(U8("%.3f ms/frame (%.1f FPS)"),
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

    // Rendering
    ImGui::Render();
  }

  // clear
  {
    // float back_color[4] = {
    // clear_color.x,clear_color.y,clear_color.z,clear_color.w };// {
    // 0.f,0.f,0.f,1.f };
    const Color4f back_color = SandboxSetting::ComponentColor::bg;
    ctx->OMSetRenderTargets(1, dx11_wnd->GetRTV().GetAddressOf(),
                            dx11_wnd->GetDsv().Get());
    ctx->ClearRenderTargetView(dx11_wnd->GetRTV().Get(),
                               (const float*)&back_color);
    ctx->ClearDepthStencilView(dx11_wnd->GetDsv().Get(),
                               D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
  }
}

void Sandbox::print_to_screen() {
  const auto& ctx = dx11_wnd->GetImCtx();

  ctx->Draw(4, 0);
}

void Sandbox::after_print_to_screen() {
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

  // Update and Render additional Platform Windows
  if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
  }
  dx11_wnd->GetSwapChain()->Present(1, 0);
}

#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
void Sandbox::set_input_callback() {
  dx11_wnd->AddWndProc(WM_MOUSEWHEEL, [&](auto wparam, auto lparam) {
    int yPos = GET_Y_LPARAM(lparam);
    int zDelta = GET_WHEEL_DELTA_WPARAM(wparam);
    if (fabs(lparam) > 1) {
      SandboxSetting::GlobalSetting::brush_size() += 1.f * yPos / zDelta;
      SandboxSetting::GlobalSetting::brush_size() =
          fminf(fmaxf(SandboxSetting::GlobalSetting::brush_size(), 1.f), 100.f);
    }

    return true;
  });
  dx11_wnd->AddWndProc(WM_KEYDOWN, [&](auto wparam, auto lparam) {
    switch (wparam) {
      case 'V':
        [[fallthrough]];
      case 'S': {
        SandboxSetting::GlobalSetting::show_velocity() =
            SandboxSetting::GlobalSetting::show_velocity() ? 0 : 1;
      } break;
      default:
        break;
    }
    return true;
  });
}

void Sandbox::init_resources() {
  const auto& ctx = dx11_wnd->GetImCtx();
  decltype(auto) device = dx11_wnd->GetDevice();
  HRESULT hr = NULL;

  //创建tex_display 以及其 srv 大小为 窗口的size/grid_size
  {
    D3D11_TEXTURE2D_DESC tex_desc = {};
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    // tex_display 的 desc
    const UINT width = SandboxGrid::w;
    const UINT height = SandboxGrid::h;
    tex_desc.Width = width;
    tex_desc.Height = height;
    tex_desc.ArraySize = 1;
    tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    tex_desc.Usage = D3D11_USAGE_DYNAMIC;
    tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    tex_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    tex_desc.MipLevels = 1;
    tex_desc.SampleDesc.Count = 1;
    // tex_display 的 srv 的 desc
    srv_desc.Format = tex_desc.Format;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = 1;
    // 创建tex_display 和 srv_tex_display
    hr = device->CreateTexture2D(&tex_desc, 0, tex_display.GetAddressOf());
    assert(SUCCEEDED(hr) && "failed to create tex_display");
    hr = device->CreateShaderResourceView(tex_display.Get(), &srv_desc,
                                          srv_tex_display.GetAddressOf());
    assert(SUCCEEDED(hr) && "failed to create srv_tex_display");
    // 创建tex_info 和 srv_tex_info
    tex_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srv_desc.Format = tex_desc.Format;
    hr = device->CreateTexture2D(&tex_desc, 0, tex_info.GetAddressOf());
    assert(SUCCEEDED(hr) && "failed to create tex_info");
    hr = device->CreateShaderResourceView(tex_info.Get(), &srv_desc,
                                          srv_tex_info.GetAddressOf());
    assert(SUCCEEDED(hr) && "failed to create srv_tex_info");
  }

  //创建顶点缓冲区
  {
    D3D11_BUFFER_DESC vbd{};
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(vertices);
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA sd{};
    sd.pSysMem = vertices;
    hr = device->CreateBuffer(&vbd, &sd, vertex_buffer.GetAddressOf());
    assert(SUCCEEDED(hr) && "failed to create vertex buffer");
  }

  //设置cbuffer
  {
    //创建cbuf_SimSetting
    D3D11_BUFFER_DESC buf_desc = {};
    buf_desc.Usage = D3D11_USAGE_DYNAMIC;
    buf_desc.ByteWidth = 32;
    buf_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    buf_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    D3D11_SUBRESOURCE_DATA InitData = {};
    const int w = dx11_wnd->getWidth();
    const int h = dx11_wnd->getHeight();

    PerFrame data = {.iResolution = {w, h},
                     .iMouse = {w / 2, h / 2},
                     .iGridSize = SandboxSetting::GlobalSetting::grid_size,
                     .BrushSize = SandboxSetting::GlobalSetting::brush_size()};
    InitData.pSysMem = &data;
    hr = device->CreateBuffer(&buf_desc, &InitData,
                              cbuf_perframe.GetAddressOf());
    assert(SUCCEEDED(hr) && "failed to create cbuffer");
  }

  //创建blendstate
  {
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable =
        false;  // alpha-to-coverage, 对alpha多重采样，计算像素的alpha
                // sample覆盖率
    blendDesc.IndependentBlendEnable = false;  //多渲染目标分别处理
    blendDesc.RenderTarget[0].BlendEnable = true;
    // dst.rgb = dst.rgb * (1.0 - src.a) + src.rgb * (src.a)
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    // dst.a = 0.0 + 1.0
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    // 写入全部4个通道
    blendDesc.RenderTarget[0].RenderTargetWriteMask =
        D3D11_COLOR_WRITE_ENABLE_ALL;
    device->CreateBlendState(&blendDesc, bs.GetAddressOf());
  }

  //设置管线状态
  {
    UINT stride = sizeof(VsIn);
    UINT offset = 0;
    float bf[4] = {0.f, 0.f, 0.f, 0.f};
    ctx->OMSetRenderTargets(1, dx11_wnd->GetRTV().GetAddressOf(),
                            dx11_wnd->GetDsv().Get());
    ctx->OMSetBlendState(bs.Get(), bf, 0xFFFFFFFF);
    ctx->OMSetDepthStencilState(0, 0);
    ctx->VSSetShader(vs.Get(), 0, 0);
    ctx->PSSetShader(ps.Get(), 0, 0);
    ctx->IASetInputLayout(input_layout.Get());
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    ctx->IASetVertexBuffers(0, 1, vertex_buffer.GetAddressOf(), &stride,
                            &offset);
    ctx->PSSetShaderResources(0, 1, srv_tex_display.GetAddressOf());
    ctx->PSSetShaderResources(1, 1, srv_tex_info.GetAddressOf());
    ctx->PSSetConstantBuffers(0, 1, cbuf_perframe.GetAddressOf());
  }
}

void Sandbox::init_shaders() {
  constexpr char vs_ps_src[] = R"(
		Texture2D<float4> srv_display : register(t0);
		Texture2D<float4> srv_info : register(t1);

		SamplerState sampler0
		{
			Filter = MIN_MAG_MIP_LINEAR;
			AddressU = Wrap;
			AddressV = Wrap;
		};

		struct VsIn
		{
			float2 pos : POSITION;
			float2 uv : TEXCOORD;
		};

		struct VsOut
		{
			float4 posH : SV_POSITION;
			float2 uv : TEXCOORD;
		};

		cbuffer PerFrame : register(b0)
		{
			int2 iResolution;
			int2 iMouse;
			int iGridSize;
			float BrushSize;
			int iShowVelocity;
		}

		VsOut vs_main(VsIn vs_in)
		{
			VsOut vs_out;
			vs_out.posH = float4(vs_in.pos, 0.f, 1.f);
			vs_out.uv = vs_in.uv;
			return vs_out;
		}

		float sdf_ring( in float2 p, in float r1, in float r2 )
		{
		  return abs(length(p)-r1) - r2;
		}

		// draw arrows
		// 2D vector field visualization by Morgan McGuire, from @morgan3d, http://casual-effects.com
		static const float PI = 3.1415927;
		static const int   ARROW_V_STYLE = 1;
		static const int   ARROW_LINE_STYLE = 2;
		static const int   ARROW_STYLE = ARROW_LINE_STYLE;
		static const float ARROW_TILE_SIZE = 12.;
		static const float ARROW_HEAD_ANGLE = 90.0 * PI / 180.0;
		static const float ARROW_HEAD_LENGTH = ARROW_TILE_SIZE / 6.;
		static const float ARROW_SHAFT_THICKNESS = 2.;
		float2 arrowTileCenterCoord(float2 pos) {
			return (floor(pos / ARROW_TILE_SIZE) + 0.5) * ARROW_TILE_SIZE;
		}
		float arrow(float2 p, float2 v) {
			p -= arrowTileCenterCoord(p);
			float mag_v = length(v), mag_p = length(p);
			if (mag_v > 1.) {
				float2 dir_p = p / mag_p, dir_v = v / mag_v;
				mag_v = clamp(mag_v, 1., ARROW_TILE_SIZE / 3.);
				v = dir_v * mag_v;
				float dist;
				if (ARROW_STYLE == ARROW_LINE_STYLE) {
					dist = max(ARROW_SHAFT_THICKNESS / 4.0 -
							max(abs(dot(p, float2(dir_v.y, -dir_v.x))),
								abs(dot(p, dir_v)) - mag_v + ARROW_HEAD_LENGTH / 2.0),
								min(0.0, dot(v - p, dir_v) - cos(ARROW_HEAD_ANGLE / 2.0) * length(v - p)) * 2.0 +
								min(0.0, dot(p, dir_v) + ARROW_HEAD_LENGTH - mag_v));
				} else {
					dist = min(0.0, mag_v - mag_p) * 2.0 +
						   min(0.0, dot(normalize(v - p), dir_v) - cos(ARROW_HEAD_ANGLE / 2.0)) * 2.0 * length(v - p) +
						   min(0.0, dot(p, dir_v) + 1.0) +
						   min(0.0, cos(ARROW_HEAD_ANGLE / 2.0) - dot(normalize(v * 0.33 - p), dir_v)) * mag_v * 0.8;
				}

				return clamp(0.6 + dist, 0.0, 1.0);
			} else {
				return max(0.0, 1.2 - mag_p);
			}
		}

		float4 ps_main(VsOut vs_out) : SV_TARGET
		{
			float4 color = srv_display.Sample(sampler0, vs_out.uv);
			float2 p = vs_out.posH.xy-(float2)iMouse;
			if(sdf_ring(p,BrushSize*0.5f*iGridSize,0.5f)<0.f){
				color = float4(0.9f,0.9f,0.9f,0.5f);
			}
			//float4 info = srv_info.Sample(sampler0, vs_out.uv);
			float2 coord = vs_out.uv * iResolution;
			float4 info_tile = srv_info.Sample(sampler0,arrowTileCenterCoord(coord)/iResolution);
			color+= iShowVelocity * arrow(coord,info_tile.xy*100.f)*float4(1.f,1.f,0.f,0.f)*info_tile.z;
			return color;
		}
	)";

  auto& device = dx11_wnd->GetDevice();
  ComPtr<ID3DBlob> blob{};
  ComPtr<ID3DBlob> error{};
  HRESULT hr = NULL;
  //编译并创建VS shader
  hr = D3DCompile(vs_ps_src, strlen(vs_ps_src), "vs", nullptr, nullptr,
                  "vs_main", "vs_5_0", 0, 0, blob.ReleaseAndGetAddressOf(),
                  error.ReleaseAndGetAddressOf());
  if (FAILED(hr)) {
    std::cout << (char*)error->GetBufferPointer() << "\n";
    return;
  }
  hr = device->CreateVertexShader(blob->GetBufferPointer(),
                                  blob->GetBufferSize(), nullptr,
                                  vs.GetAddressOf());
  if (FAILED(hr)) {
    std::cout << "failed to create VertexShader"
              << "\n";
    return;
  }

  //创建input layout
  hr = device->CreateInputLayout(
      VsIn::input_layout, VsIn::num_elements, blob->GetBufferPointer(),
      blob->GetBufferSize(), input_layout.GetAddressOf());
  if (FAILED(hr)) {
    std::cout << "failed to create input layout"
              << "\n";
    return;
  }

  //编译并创建PS shader
  hr = D3DCompile(vs_ps_src, strlen(vs_ps_src), "ps", nullptr, nullptr,
                  "ps_main", "ps_5_0", 0, 0, blob.ReleaseAndGetAddressOf(),
                  error.ReleaseAndGetAddressOf());
  if (FAILED(hr)) {
    std::cout << (char*)error->GetBufferPointer() << "\n";
    return;
  }
  hr =
      device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(),
                                nullptr, ps.GetAddressOf());
  if (FAILED(hr)) {
    std::cout << "failed to create PixelShader"
              << "\n";
    return;
  }
}
