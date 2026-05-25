#pragma once
#include "../utils/Vector.h"
#include <vector>
#include "../imgui_d11/imgui.h"

uintptr_t GetBaseEntity(int index, uintptr_t client);
uintptr_t GetBaseEntityFromHandle(uint32_t uHandle, uintptr_t client);

void draw_esp();

namespace Bone_Base
{
	enum BoneIndex : int32_t
	{
		Head = 7, //头
		Head_Alt = 8, //头部备用索引
		Neck_0 = 6, //颈部
		Spine_1 = 4,//背部
		Spine_2 = 3, //背部
		Pelvis = 2, //骨盆
		Arm_Upper_L = 9, //左臂
		Arm_Lower_L = 10,//左臂
		Hand_L = 11,//左手
		Arm_Upper_R = 13,//右臂
		Arm_Lower_R = 14,//右臂
		Hand_R = 15,//右手
		Leg_Upper_L = 17,//左腿
		Leg_Lower_L = 18,//左腿
		Foot_L = 19,//左脚
		Leg_Upper_R = 20,//右腿
		Leg_Lower_R = 21,//右腿
		Foot_R = 22,//右脚
	};

	Vector3 BonePos(uintptr_t addr, int32_t index);

	void Bone_Start(uintptr_t pawn, ImColor BoneColor, float* Matrix);

	void DrawLine(std::vector<Vector3> list, ImColor Color, float* Matrix);
}

inline std::vector<Vector3>BoneDrawList{};