#include <Windows.h>
#include <cstdint>
#include <cstring>
#include <thread>
#include "../cs2_dumper/offsets.hpp"
#include "../cs2_dumper/client_dll.hpp"
#include "../minhook_debug_x64/include/MinHook.h"
#include "../gui/gui.h"
#include "glow.h"
#include <Psapi.h>

using ShouldGlowFn = bool(__fastcall*)(void*);
using ApplyGlowFn = void(__fastcall*)(void*, CGlowObject*);

static ShouldGlowFn oShouldGlow = nullptr;
static ApplyGlowFn oApplyGlow = nullptr;

static uintptr_t pattern_scan(const uint8_t* pattern, const char* mask, uintptr_t moduleBase, size_t moduleSize)
{
	size_t maskLen = strlen(mask);

	for (uintptr_t i = moduleBase; i < moduleBase + moduleSize - maskLen; i++)
	{
		bool found = true;
		for (size_t j = 0; j < maskLen; j++)
		{
			if (mask[j] == 'x' && *(uint8_t*)(i + j) != pattern[j])
			{
				found = false;
				break;
			}
		}
		if (found)
			return i;
	}

	return 0;
}

static uintptr_t resolve_call(uintptr_t addr)
{
	auto rel = *reinterpret_cast<int32_t*>(addr + 1);
	return addr + 5 + rel;
}

static bool __fastcall hkShouldGlow(void* glowProperty)
{
	if (!cs2::visuals::glow)
		return oShouldGlow(glowProperty);

	uintptr_t p = (uintptr_t)glowProperty;

	*reinterpret_cast<int*>(p + cs2_dumper::schemas::client_dll::CGlowProperty::m_iGlowType) = 0;
	*reinterpret_cast<int*>(p + cs2_dumper::schemas::client_dll::CGlowProperty::m_iGlowTeam) = -1;
	*reinterpret_cast<int*>(p + cs2_dumper::schemas::client_dll::CGlowProperty::m_nGlowRange) = 10000;
	*reinterpret_cast<int*>(p + cs2_dumper::schemas::client_dll::CGlowProperty::m_nGlowRangeMin) = 0;
	*reinterpret_cast<float*>(p + cs2_dumper::schemas::client_dll::CGlowProperty::m_flGlowTime) = 86400.0f;
	*reinterpret_cast<float*>(p + cs2_dumper::schemas::client_dll::CGlowProperty::m_flGlowStartTime) = 0.0f;
	*reinterpret_cast<bool*>(p + cs2_dumper::schemas::client_dll::CGlowProperty::m_bGlowing) = true;

	return true;
}

static void __fastcall hkApplyGlow(void* glowProperty, CGlowObject* glowObject)
{
	oApplyGlow(glowProperty, glowObject);

	if (!cs2::visuals::glow)
		return;

	glowObject->red = cs2::visuals::glow_color[0];
	glowObject->green = cs2::visuals::glow_color[1];
	glowObject->blue = cs2::visuals::glow_color[2];
	glowObject->alpha = cs2::visuals::glow_color[3];
}

static void glow_thread()
{
	while (true)
	{
		Sleep(1);

		if (!cs2::visuals::glow)
			continue;

		const auto client = reinterpret_cast<uintptr_t>(GetModuleHandle(L"client.dll"));
		if (!client)
			continue;

		auto entListBase = *reinterpret_cast<uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwEntityList);
		if (!entListBase)
			continue;

		for (int i = 0; i < 64; i++)
		{
			auto player_co = *reinterpret_cast<uintptr_t*>(entListBase + 8 * (i >> 9) + 16);
			if (!player_co)
				continue;

			player_co = *reinterpret_cast<uintptr_t*>(player_co + 0x70 * (i & 0x1FF));
			if (!player_co)
				continue;

			auto player_hpawn = *reinterpret_cast<uint32_t*>(player_co + cs2_dumper::schemas::client_dll::CBasePlayerController::m_hPawn);
			if (player_hpawn == 0xFFFFFFFF)
				continue;

			const int nIndex = player_hpawn & 0x7FFF;
			auto node = *reinterpret_cast<uintptr_t*>(entListBase + 8 * (nIndex >> 9) + 16);
			if (!node)
				continue;

			auto player_pawn = *reinterpret_cast<uintptr_t*>(node + 0x70 * (nIndex & 0x1FF));
			if (!player_pawn)
				continue;

			auto player_team = *reinterpret_cast<int*>(player_pawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);
			if (player_team != 2 && player_team != 3)
				continue;

			auto player_health = *reinterpret_cast<int*>(player_pawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth);
			if (player_health <= 0)
				continue;

			auto pGlow = player_pawn + cs2_dumper::schemas::client_dll::C_BaseModelEntity::m_Glow;

			*reinterpret_cast<int*>(pGlow + cs2_dumper::schemas::client_dll::CGlowProperty::m_iGlowType) = 0;
			*reinterpret_cast<int*>(pGlow + cs2_dumper::schemas::client_dll::CGlowProperty::m_iGlowTeam) = -1;
			*reinterpret_cast<int*>(pGlow + cs2_dumper::schemas::client_dll::CGlowProperty::m_nGlowRange) = 10000;
			*reinterpret_cast<int*>(pGlow + cs2_dumper::schemas::client_dll::CGlowProperty::m_nGlowRangeMin) = 0;
			*reinterpret_cast<float*>(pGlow + cs2_dumper::schemas::client_dll::CGlowProperty::m_flGlowTime) = 86400.0f;
			*reinterpret_cast<float*>(pGlow + cs2_dumper::schemas::client_dll::CGlowProperty::m_flGlowStartTime) = 0.0f;
			*reinterpret_cast<bool*>(pGlow + cs2_dumper::schemas::client_dll::CGlowProperty::m_bGlowing) = true;
		}
	}
}

void init_glow_hooks()
{
	HMODULE hClient = nullptr;
	while (!hClient)
	{
		hClient = GetModuleHandleA("client.dll");
		Sleep(100);
	}

	auto client = reinterpret_cast<uintptr_t>(hClient);

	MODULEINFO modInfo;
	GetModuleInformation(GetCurrentProcess(), (HMODULE)client, &modInfo, sizeof(modInfo));
	auto moduleSize = modInfo.SizeOfImage;

	uint8_t sigShouldGlow[] = { 0xE8, 0x00, 0x00, 0x00, 0x00, 0x33, 0xDB, 0x84, 0xC0, 0x0F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x4F };
	uintptr_t addrShouldGlow = pattern_scan(sigShouldGlow, "x????xxxxxx????xxx", client, moduleSize);

	if (addrShouldGlow)
	{
		addrShouldGlow = resolve_call(addrShouldGlow);
		MH_CreateHook((void*)addrShouldGlow, hkShouldGlow, (void**)&oShouldGlow);
		MH_EnableHook((void*)addrShouldGlow);
	}

	uint8_t sigApplyGlow[] = { 0xE8, 0x00, 0x00, 0x00, 0x00, 0xF3, 0x0F, 0x10, 0xBE, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xCF };
	uintptr_t addrApplyGlow = pattern_scan(sigApplyGlow, "x????xxxx????xxx", client, moduleSize);

	if (addrApplyGlow)
	{
		addrApplyGlow = resolve_call(addrApplyGlow);
		MH_CreateHook((void*)addrApplyGlow, hkApplyGlow, (void**)&oApplyGlow);
		MH_EnableHook((void*)addrApplyGlow);
	}

	std::thread(glow_thread).detach();
}
