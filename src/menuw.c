#include <stdio.h>

#include "app.h"
#include "map.h"
#include "menuw.h"

int menu_window(struct app *app, struct nk_context *ctx)
{
    int h = 25;
    int w = app->screen_width;
    int x = 0;
    int y = 0;

    if (nk_begin(ctx, "menu", nk_rect(x, y, w, h), NK_WINDOW_NO_SCROLLBAR)) {
        nk_menubar_begin(ctx);
        nk_layout_row_static(ctx, h, 45, 2);

        if (nk_menu_begin_label(ctx, "file", NK_TEXT_ALIGN_LEFT,
                                nk_vec2(100, 100))) {
            nk_layout_row_dynamic(ctx, h, 1);

            if (nk_menu_item_label(ctx, "new", NK_TEXT_LEFT))
                map_reset_tiles(app->map);

            if (nk_menu_item_label(ctx, "save", NK_TEXT_LEFT))
                map_serialize(app->map, "start.wb");

            if (nk_menu_item_label(ctx, "load", NK_TEXT_LEFT))
                map_deserialize(app->map, "start.wb");

            nk_menu_end(ctx);
        }

        if (nk_menu_begin_label(ctx, "view", NK_TEXT_ALIGN_LEFT,
                                nk_vec2(100, 100))) {
            nk_layout_row_dynamic(ctx, h, 1);

            // tools window
            char toolsw_title[] = "*tools";
            if (!app->show_toolsw)
                snprintf(toolsw_title, sizeof toolsw_title, " tools");

            if (nk_menu_item_label(ctx, toolsw_title, NK_TEXT_LEFT))
                app->show_toolsw = !app->show_toolsw;

            // tileset window
            char tilesetw_title[] = "*tileset";
            if (!app->show_tilesetw)
                snprintf(tilesetw_title, sizeof tilesetw_title, " tileset");

            if (nk_menu_item_label(ctx, tilesetw_title, NK_TEXT_LEFT))
                app->show_tilesetw = !app->show_tilesetw;

            nk_menu_end(ctx);
        }

        nk_menubar_end(ctx);
    }
    nk_end(ctx);

    return 1;
}
