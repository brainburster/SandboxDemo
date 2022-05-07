#include "Sandbox.h"

#include <memory>
#include <iostream>

//我打算先做一个CPU版本的，用于调试
//之后再做一个GPU版本的，具体使用DX还是CUDA之后再说
//CPU更新后用把结果复制到显存中去，
//使用IMGUI来设置UI

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")

using namespace std;

int main() {
	auto wnd = std::make_shared<DX11_Wnd>(GetModuleHandle(0));
	wnd->Size(SandboxSetting::GlobalSetting::width, SandboxSetting::GlobalSetting::height)
		.WndClassName(L"Cls_Sandbox_mainwnd")
		.WndName(L"Sandbox")
		.RemoveWndStyle(WS_MAXIMIZEBOX)
		.Init();

	Sandbox Sandbox = { wnd };
	Sandbox.run();
	return 0;
}
