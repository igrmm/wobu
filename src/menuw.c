#include "SDL.h" // IWYU pragma: keep //clangd

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

            if (nk_menu_item_label(ctx, "new", NK_TEXT_LEFT)) {
                map_reset_tiles(app->map);
                map_destroy_entities(app->map->entities.head);
                app->map->entities.head = app->map->entities.tail = NULL;
                app->selection.count = 0;
            }

            if (nk_menu_item_label(ctx, "save", NK_TEXT_LEFT))
                map_serialize(app->map, "start.wb");

            if (nk_menu_item_label(ctx, "load", NK_TEXT_LEFT))
                map_deserialize(app->map, "start.wb");

            nk_menu_end(ctx);
        }

        if (nk_menu_begin_label(ctx, "view", NK_TEXT_ALIGN_LEFT,
                                nk_vec2(100, 120))) {
            nk_layout_row_dynamic(ctx, h, 1);

            // grid
            char grid_title[] = "*grid";
            if (!app->show_grid)
                SDL_snprintf(grid_title, sizeof grid_title, " grid");

            if (nk_menu_item_label(ctx, grid_title, NK_TEXT_LEFT))
                app->show_grid = !app->show_grid;

            // tools window
            char toolsw_title[] = "*tools";
            if (!app->show_toolsw)
                SDL_snprintf(toolsw_title, sizeof toolsw_title, " tools");

            if (nk_menu_item_label(ctx, toolsw_title, NK_TEXT_LEFT))
                app->show_toolsw = !app->show_toolsw;

            // tileset window
            char tilesetw_title[] = "*tileset";
            if (!app->show_tilesetw)
                SDL_snprintf(tilesetw_title, sizeof tilesetw_title, " tileset");

            if (nk_menu_item_label(ctx, tilesetw_title, NK_TEXT_LEFT))
                app->show_tilesetw = !app->show_tilesetw;

            // properties window
            char propertiesw_title[] = "*properties";
            if (!app->show_propertiesw)
                SDL_snprintf(propertiesw_title, sizeof propertiesw_title,
                             " properties");

            if (nk_menu_item_label(ctx, propertiesw_title, NK_TEXT_LEFT))
                app->show_propertiesw = !app->show_propertiesw;

            nk_menu_end(ctx);
        }

        nk_menubar_end(ctx);
    }
    nk_end(ctx);

    return 1;
}
