#pragma once

namespace cs2
{
    namespace visuals
    {
        inline bool box = true;

		inline bool bone = false;

		inline float bone_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

        inline bool enemy_only = false;

        inline float box_color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };

        inline bool glow = false;

        inline float glow_color[4] = { 0.0f, 1.0f, 0.0f, 1.0f };

        inline bool recoil = false;

        inline bool tiggerbot = false;
    }

    namespace menu
    {
        inline bool opened = false;
        inline int tab = 0;
    }
}

void draw_Menu();