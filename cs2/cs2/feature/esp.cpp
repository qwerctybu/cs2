#include "Windows.h"
#include <cstdint>
#include "../cs2_dumper/offsets.hpp"
#include "../cs2_dumper/client_dll.hpp"
#include "../utils/Vector.h"
#include <optional>
#include <cmath>
#include "../imgui_d11/imgui.h"
#include "../gui/gui.h"

static uintptr_t GetBaseEntity(int index, uintptr_t client)
{
	auto entListBase = *reinterpret_cast<std::uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwEntityList);
	if (entListBase == 0)
	{
		return 0;
	}

	auto entitylistbase = *reinterpret_cast<std::uintptr_t*>(entListBase + 0x8 * (index >> 9) +16);
	if(entitylistbase == 0)
	{
		return 0;
	}

	return *reinterpret_cast<std::uintptr_t*>(entitylistbase + (0x70 * (index & 0x1FF)));
}

static uintptr_t GetBaseEntityFromHandle(uint32_t uHandle, uintptr_t client)
{
	auto entListBase = *reinterpret_cast<std::uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwEntityList);
	if (entListBase == 0)
	{
		return 0;
	}

	const int nIndex = uHandle & 0x7FFF;

	auto entitylistbase = *reinterpret_cast<std::uintptr_t*>(entListBase + 8 * (nIndex >> 9) +16);
	if(entitylistbase == 0)
	{
		return 0;
	}

	return *reinterpret_cast<std::uintptr_t*>(entitylistbase + (0x70 * (nIndex & 0x1FF)));
}

bool WorldToScreen(Vector3 pWorldPos, Vector3& pScreenPos,float* pMatrixPtr, const FLOAT pWinWidth, const FLOAT pWinHeight)
{
	float matrix2[4][4];

	memcpy(matrix2, pMatrixPtr, 16 * sizeof(float));

	const float mX{ pWinWidth / 2 };
	const float mY{ pWinHeight / 2 };

	const float w{ matrix2[3][0] * pWorldPos.x + matrix2[3][1] * pWorldPos.y + matrix2[3][2] * pWorldPos.z + matrix2[3][3] };

	if (w < 0.65f) return false;

	const float x
	{
		matrix2[0][0] * pWorldPos.x + matrix2[0][1] * pWorldPos.y + matrix2[0][2] * pWorldPos.z + matrix2[0][3]
	};

	const float y
	{
		matrix2[1][0] * pWorldPos.x + matrix2[1][1] * pWorldPos.y + matrix2[1][2] * pWorldPos.z + matrix2[1][3]
	};

	pScreenPos.x = (mX + mX * x / w);
	pScreenPos.y = (mY - mY * y / w);
	pScreenPos.z = w;

	return true;
}

std::optional<Vector3> GetEyePos(uintptr_t addr) noexcept
{
	auto* Origin = reinterpret_cast<Vector3*>(addr + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_vOldOrigin);
	auto* ViewOffset = reinterpret_cast<Vector3*>(addr + cs2_dumper::schemas::client_dll::C_BaseModelEntity::m_vecViewOffset);

	Vector3 LocalEye = *Origin + *ViewOffset;
	if(!std::isfinite(LocalEye.x) || !std::isfinite(LocalEye.y) || !std::isfinite(LocalEye.z))
	{
		return std::nullopt;
	}

	if(LocalEye.Length() < 0.1f)
	{
		return std::nullopt;
	}

	return LocalEye;
}

void draw_esp()
{
	const auto client = reinterpret_cast<uintptr_t>(GetModuleHandle(L"client.dll"));

	auto local_ctrl = *reinterpret_cast<uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwLocalPlayerController);
	if(!local_ctrl)
	{
		return;
	}

	auto local_hpawn = *reinterpret_cast<uint32_t*>(local_ctrl + cs2_dumper::schemas::client_dll::CBasePlayerController::m_hPawn);
	if (local_hpawn == 0xFFFFFFFF)
	{
		return;
	}

	auto localpawn = GetBaseEntityFromHandle(local_hpawn, client);
	if(!localpawn)
	{
		return;
	}

	auto localteam = *reinterpret_cast<int*>(localpawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);

	auto Matrix = reinterpret_cast<float*>(client + cs2_dumper::offsets::client_dll::dwViewMatrix);

	for(int i = 0; i < 64; i++)
	{
		auto player_co = GetBaseEntity(i, client);

		if (!player_co)
		{
			continue;
		}

		auto player_hpawn = *reinterpret_cast<uint32_t*>(player_co + cs2_dumper::schemas::client_dll::CBasePlayerController::m_hPawn);
		if (player_hpawn == 0xFFFFFFFF)
		{
			continue;
		}

		auto player_pawn = GetBaseEntityFromHandle(player_hpawn, client);
		if (!player_pawn)
		{
			continue;
		}

		auto player_team = *reinterpret_cast<int*>(
			player_pawn +
			cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum
			);

		// enemy_only 开启时
		// 过滤队友
		if (!cs2::visuals::enemy_only)
		{
			if (localteam == player_team)
			{
				continue;
			}
		}

		auto player_health = *reinterpret_cast<int*>(player_pawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth);
		if(player_health <= 0)
		{
			continue;
		}

		auto player_Origin = *reinterpret_cast<Vector3*>(player_pawn + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_vOldOrigin);

		auto player_eyepos_op_vec = GetEyePos(player_pawn);

		if(!player_eyepos_op_vec.has_value())
		{
			continue;
		}

		auto player_eyepos = player_eyepos_op_vec.value();

		static const float w = ImGui::GetIO().DisplaySize.x;
		static const float h = ImGui::GetIO().DisplaySize.y;

		Vector3 head_pos_2d{};
		Vector3 abs_pos_2d{};

		if (!WorldToScreen(player_Origin, abs_pos_2d, Matrix, w, h))
		{
			continue;
		}

		if (!WorldToScreen(player_eyepos, head_pos_2d, Matrix, w, h))
		{
			continue;
		}

		const float height{ ::abs(head_pos_2d.y - abs_pos_2d.y) * 1.25f};
		const float width{ height / 2.0f };
		const float x = abs_pos_2d.x - (width / 2.0f);
		const float y = abs_pos_2d.y - (height / 1.1f);

		if (cs2::visuals::box)
		{
			ImGui::GetBackgroundDrawList()->AddRect(ImVec2(x, y), ImVec2(x + width, y + height), IM_COL32(255, 0, 0, 255), 0.0f, 0, 1.5f);
		}
	}
}