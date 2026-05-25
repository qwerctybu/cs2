#include "../imgui_d11/imgui.h"
#include "gui.h"

static const char* tab_names[] = { "Rage", "Legit", "Visuals", "Misc" };
static const int tab_count = 4;

void draw_Menu()
{
    ImGui::SetNextWindowSize(ImVec2(955, 755), ImGuiCond_Always);
    ImGui::Begin("AimWare", nullptr, ImGuiWindowFlags_NoResize);

    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 4.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 0.0f));

        for (int i = 0; i < tab_count; i++)
        {
            if (i > 0)
                ImGui::SameLine();

            if (cs2::menu::tab == i)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
                ImGui::Button(tab_names[i]);
                ImGui::PopStyleColor();
            }
            else
            {
                if (ImGui::Button(tab_names[i]))
                    cs2::menu::tab = i;
            }
        }

        ImGui::PopStyleVar(2);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        switch (cs2::menu::tab)
        {
        case 0: // Rage
            break;

        case 1: // Legit
            ImGui::Checkbox(
                "recoil",
                &cs2::visuals::recoil
            );

            ImGui::Checkbox(
                "tiggerbot",
                &cs2::visuals::tiggerbot
            );
            break;

        case 2: // Visuals
            ImGui::Checkbox(
                "box esp",
                &cs2::visuals::box
            );
            ImGui::SameLine();
            ImGui::ColorEdit4(
                "##box_color",
                cs2::visuals::box_color,
                ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel
            );

            if (cs2::visuals::box)
            {
                ImGui::Indent();

                ImGui::Checkbox(
                    "Deathmatch",
                    &cs2::visuals::enemy_only
                );

                ImGui::Unindent();
            }

            ImGui::Checkbox(
                "glow",
                &cs2::visuals::glow
            );
            ImGui::SameLine();
            ImGui::ColorEdit4(
                "##glow_color",
                cs2::visuals::glow_color,
                ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel
            );

            ImGui::Checkbox(
                "bone",
                &cs2::visuals::bone
            );
            ImGui::SameLine();
            ImGui::ColorEdit4(
                "##bone_color",
                cs2::visuals::bone_color,
                ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel
            );
            break;

        case 3: // Misc
            break;
        }
    }

    ImGui::End();
}
