#include "./tiggerbot.h"
#include <Windows.h>
#include <cstdint>
#include "../cs2_dumper/client_dll.hpp"
#include "esp.h"
#include "../cs2_dumper/offsets.hpp"
#include "../cs2_dumper/buttons.hpp"
#include "../gui/gui.h"

DWORD tiggerbot(void*)
{
	const auto client = reinterpret_cast<uintptr_t>(GetModuleHandle(L"client.dll"));

	while (true)
	{
		auto local_ctrl = *reinterpret_cast<uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwLocalPlayerController);
		if (!local_ctrl)
		{
			continue;
		}

		auto local_hpawn = *reinterpret_cast<uint32_t*>(local_ctrl + cs2_dumper::schemas::client_dll::CBasePlayerController::m_hPawn);
		if (local_hpawn == 0xFFFFFFFF)
		{
			continue;
		}

		auto localpawn = GetBaseEntityFromHandle(local_hpawn, client);
		if (!localpawn)
		{
			continue;
		}

		auto localteam = *reinterpret_cast<int*>(localpawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);

		auto localhealth = *reinterpret_cast<int*>(localpawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth);
		if(localhealth <= 0)
		{
			continue;
		}

		auto crosshair_entity_handle = *reinterpret_cast<uint32_t*>(localpawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_iIDEntIndex);

		if(crosshair_entity_handle == 0 || crosshair_entity_handle == 0xFFFFFFFF)
		{
			continue;
		}

		auto playerpawn = GetBaseEntityFromHandle(crosshair_entity_handle, client);
		if (!playerpawn)
		{
			continue;
		}

		auto player_team = *reinterpret_cast<int*>(playerpawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);
		if (player_team == localteam)
		{
			continue;
		}

		auto player_health = *reinterpret_cast<int*>(playerpawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth);
		if (player_health <= 0)
		{
			continue;
		}

		if (cs2::visuals::tiggerbot)
		{
			auto set_force_attack = reinterpret_cast<int*>(client + cs2_dumper::buttons::attack);

			Sleep(25);

			*set_force_attack = 65537;
			Sleep(10);
			*set_force_attack = 256;
		}

		Sleep(1);
	}
}