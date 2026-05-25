#include <Windows.h>
#include <cmath>
#include "../cs2_dumper/client_dll.hpp"
#include "../cs2_dumper/offsets.hpp"
#include "../utils/Vector.h"
#include "../gui/gui.h"

static void NormalizePitch(float& pitch)
{
	pitch = (pitch < -89.0f) ? -89.0f : pitch;
	pitch = (pitch > 89.0f) ? 89.0f : pitch;
}

static void NormalizeYaw(float& pYaw)
{
	while (pYaw > 180.0f) pYaw -= 360.0f;
	while (pYaw < -180.0f) pYaw += 360.0f;
}

void do_recoil_control()
{
	if (!cs2::visuals::recoil)
		return;

	const static auto client = reinterpret_cast<uintptr_t>(GetModuleHandleA("client.dll"));

	auto localplayer_pawn = *reinterpret_cast<uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);

	if (localplayer_pawn == 0)
		return;

	auto health = *reinterpret_cast<int*>(localplayer_pawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth);

	if (health <= 0)
		return;

	static Vector3 old_aimpunch{};

	auto local_view_angles = *reinterpret_cast<Vector3*>(client + cs2_dumper::offsets::client_dll::dwViewAngles);

	auto aimPunchServices = *reinterpret_cast<uintptr_t*>(
		localplayer_pawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_pAimPunchServices);

	auto aimpunchAngles = *reinterpret_cast<Vector3*>(
		aimPunchServices + cs2_dumper::schemas::client_dll::CCSPlayer_AimPunchServices::m_predictableBaseAngle);

	auto shotfired = *reinterpret_cast<int*>(localplayer_pawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_iShotsFired);

	if (shotfired > 1)
	{
		Vector3 new_view_angles{};

		new_view_angles.x =
			local_view_angles.x
			+ old_aimpunch.x
			- (aimpunchAngles.x * 2.f);

		new_view_angles.y =
			local_view_angles.y
			+ old_aimpunch.y
			- (aimpunchAngles.y * 2.f);

		NormalizePitch(new_view_angles.x);
		NormalizeYaw(new_view_angles.y);

		*reinterpret_cast<Vector3*>(
			client + cs2_dumper::offsets::client_dll::dwViewAngles
			) = new_view_angles;

		old_aimpunch.x = aimpunchAngles.x * 2.f;
		old_aimpunch.y = aimpunchAngles.y * 2.f;
	}
	else
	{
		old_aimpunch = Vector3{};
	}
}
