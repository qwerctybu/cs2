#include <Windows.h>
#include <d3d11.h>
#include "../minhook_debug_x64/include/MinHook.h"
#include "../imgui_d11/imgui.h"
#include "../imgui_d11/imgui_impl_win32.h"
#include "../imgui_d11/imgui_impl_dx11.h"
#include <thread>
#include "../gui/gui.h"
#include "../feature/esp.h"
#include "../feature/recoil.h"
#include "../feature/glow.h"
#include "../feature/tiggerbot.h"
#include "../cs2_dumper/offsets.hpp"
static ID3D11Device* g_pd3dDevice = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11DeviceContext* g_pd3dContext = nullptr;
static ID3D11RenderTargetView* view = nullptr;
static HWND g_hwnd = nullptr;
void* origin_present = nullptr;

using Present = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT);



WNDPROC origin_wndProc;

extern LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND,
	UINT,
	WPARAM,
	LPARAM
);

LRESULT __stdcall WndProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	if (cs2::menu::opened)
	{
		ImGui_ImplWin32_WndProcHandler(
			hwnd,
			uMsg,
			wParam,
			lParam
		);

		switch (uMsg)
		{
		case WM_MOUSEMOVE:
		case WM_MOUSEWHEEL:

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:

		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:

		case WM_KEYDOWN:
		case WM_KEYUP:

			return true;
		}
	}

	return CallWindowProc(
		origin_wndProc,
		hwnd,
		uMsg,
		wParam,
		lParam
	);
}

bool inited = false;

long __stdcall my_present(
	IDXGISwapChain* _this,
	UINT a,
	UINT b)
{
	if (!inited)
	{
		_this->GetDevice(
			__uuidof(ID3D11Device),
			(void**)&g_pd3dDevice
		);

		g_pd3dDevice->GetImmediateContext(
			&g_pd3dContext
		);

		DXGI_SWAP_CHAIN_DESC sd;
		_this->GetDesc(&sd);

		g_hwnd = sd.OutputWindow;

		ID3D11Texture2D* buf{};

		_this->GetBuffer(
			0,
			__uuidof(ID3D11Texture2D),
			(void**)&buf
		);

		g_pd3dDevice->CreateRenderTargetView(
			buf,
			nullptr,
			&view
		);

		buf->Release();

		origin_wndProc =
			(WNDPROC)SetWindowLongPtr(
				g_hwnd,
				GWLP_WNDPROC,
				(LONG_PTR)WndProc
			);

		ImGui::CreateContext();

		ImGui_ImplWin32_Init(g_hwnd);

		ImGui_ImplDX11_Init(
			g_pd3dDevice,
			g_pd3dContext
		);

		inited = true;
	}

	// INS 开关菜单
	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		cs2::menu::opened =
			!cs2::menu::opened;

		if (cs2::menu::opened)
		{
			ShowCursor(TRUE);

			ClipCursor(nullptr);

			ReleaseCapture();

			ImGui::GetIO().MouseDrawCursor = false;
		}
		else
		{
			ShowCursor(FALSE);

			ImGui::GetIO().MouseDrawCursor = false;
		}
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();

	// 绘制菜单
	if (cs2::menu::opened)
	{
		draw_Menu();
	}

	// RCS
	do_recoil_control();

	// ESP
	draw_esp();

	ImGui::EndFrame();

	ImGui::Render();

	g_pd3dContext->OMSetRenderTargets(
		1,
		&view,
		nullptr
	);

	ImGui_ImplDX11_RenderDrawData(
		ImGui::GetDrawData()
	);

	return ((Present)origin_present)(
		_this,
		a,
		b
		);
}


DWORD create(void*) {
	const unsigned level_count = 2;
	D3D_FEATURE_LEVEL levels[level_count] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
	DXGI_SWAP_CHAIN_DESC sd{};
	sd.BufferCount = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = GetForegroundWindow();
	sd.SampleDesc.Count = 1;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	auto hr = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		levels,
		level_count,
		D3D11_SDK_VERSION,
		&sd,
		&g_pSwapChain,
		&g_pd3dDevice,
		nullptr,
		nullptr);

	if (g_pSwapChain) {
		auto vtable_ptr = (void***)(g_pSwapChain);
		auto vtable = *vtable_ptr;
		auto present = vtable[8];
		MH_Initialize();
		MH_CreateHook(present, my_present, &origin_present);
		MH_EnableHook(present);
		g_pd3dDevice->Release();
		g_pSwapChain->Release();

		init_glow_hooks();
	}

	return 0;

}

BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == 1)
	{
		CreateThread(NULL, 0, create, NULL, 0, NULL);

		CreateThread(NULL, 0, tiggerbot, NULL, 0, NULL);
	}
	return TRUE;
}