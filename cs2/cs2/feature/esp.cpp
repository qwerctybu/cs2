#include "Windows.h"
#include <cstdint>
#include "../cs2_dumper/offsets.hpp"
#include "../cs2_dumper/client_dll.hpp"
#include "../utils/Vector.h"
#include <optional>
#include <cmath>
#include "../imgui_d11/imgui.h"
#include "../gui/gui.h"
#include "esp.h"

uintptr_t GetBaseEntity(int index, uintptr_t client)
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

uintptr_t GetBaseEntityFromHandle(uint32_t uHandle, uintptr_t client)
{
	const int nIndex = uHandle & 0x7FFF;
	return GetBaseEntity(nIndex, client);
}

bool WorldToScreen(const Vector3& pWorldPos, Vector3& pScreenPos, float* pMatrixPtr, const FLOAT pWinWidth, const FLOAT pWinHeight)
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

Vector3 Bone_Base::BonePos(uintptr_t addr, int32_t index)
{
	int32_t d = 32 * index;
	uintptr_t address{};
	address = *reinterpret_cast<uintptr_t*>(addr + cs2_dumper::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);
	if(!address)
	{
		return Vector3{};
	}
	auto BoneArray = cs2_dumper::schemas::client_dll::CSkeletonInstance::m_modelState + 0x80;
	address = *reinterpret_cast<uintptr_t*>(address + BoneArray);
	if(!address)
	{
		return Vector3{};
	}
	return *reinterpret_cast<Vector3*>(address + d);
}

void DrawLine(std::vector<Vector3> list, ImColor Color, float* Matrix)
{
	Vector3 drawpos;
	std::vector<Vector3>DrawList{};
	for (int i = 0; i < list.size(); ++i)
	{
		if (!WorldToScreen(list[i], drawpos, Matrix, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y))
		{
			continue;
		}

		DrawList.push_back(drawpos);
	}
	
	for(int i = 1; i< DrawList.size(); ++i)
	{
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(DrawList[i].x, DrawList[i].y), ImVec2(DrawList[i - 1].x, DrawList[i - 1].y), Color);
	}
}

void Bone_Start(uintptr_t pawn, ImColor BoneColor, float* Matrix)
{
	BoneDrawList.clear();
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::Head));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::Neck_0));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::Spine_2));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::Pelvis));
	DrawLine(BoneDrawList, BoneColor, Matrix);

	BoneDrawList.clear();
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::Neck_0));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::Arm_Upper_L));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::Arm_Lower_L));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::Hand_L));
	DrawLine(BoneDrawList, BoneColor, Matrix);

	BoneDrawList.clear();
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::Neck_0));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::Arm_Upper_R));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::Arm_Lower_R));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::Hand_R));
	DrawLine(BoneDrawList, BoneColor, Matrix);

	BoneDrawList.clear();
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::Pelvis));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::Leg_Upper_L));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::Leg_Lower_L));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::Foot_L));
	DrawLine(BoneDrawList, BoneColor, Matrix);

	BoneDrawList.clear();
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::Pelvis));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::Leg_Upper_R));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::Leg_Lower_R));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::Foot_R));
	DrawLine(BoneDrawList, BoneColor, Matrix);
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

		const float w = ImGui::GetIO().DisplaySize.x;
		const float h = ImGui::GetIO().DisplaySize.y;

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
			ImU32 color = IM_COL32(
				(int)(cs2::visuals::box_color[0] * 255),
				(int)(cs2::visuals::box_color[1] * 255),
				(int)(cs2::visuals::box_color[2] * 255),
				(int)(cs2::visuals::box_color[3] * 255)
			);
			ImGui::GetBackgroundDrawList()->AddRect(ImVec2(x, y), ImVec2(x + width, y + height), color, 0.0f, 0, 1.5f);
		}

		if(cs2::visuals::bone)
		{
			ImColor boneClr(
				cs2::visuals::bone_color[0],
				cs2::visuals::bone_color[1],
				cs2::visuals::bone_color[2],
				cs2::visuals::bone_color[3]
			);
			Bone_Start(player_pawn, boneClr, Matrix);
		}
	}
}