#include "../imgui_d11/imgui.h"
#include "gui.h"

void draw_Menu()
{
    ImGui::Begin("AimWare");

    {
        ImGui::Checkbox(
            "box esp",
            &cs2::visuals::box
        );

        // 只有 box 开启时
        // 才显示 Deathmatch

        if (cs2::visuals::box)
        {
            ImGui::Indent();

            ImGui::Checkbox(
                "Deathmatch",
                &cs2::visuals::enemy_only
            );

            ImGui::Unindent();
        }
    }

    ImGui::End();
}