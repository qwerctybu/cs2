#pragma once

namespace cs2
{
    namespace visuals
    {
        inline bool box = true;

        // true = 只画敌人
        // false = 所有人都画
        inline bool enemy_only = false;
    }

    namespace menu
    {
        inline bool opened = false;
    }
}

void draw_Menu();