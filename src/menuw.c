#include "menuw.h"
#include "app.h"

int menu_window(struct app *app, struct nk_context *ctx)
{
    int h = 25;
    int w = app->screen_width;
    int x = 0;
    int y = 0;

    if (nk_begin(ctx, "menu", nk_rect(x, y, w, h), NK_WINDOW_NO_SCROLLBAR)) {
        nk_menubar_begin(ctx);

        nk_layout_row_dynamic(ctx, h, 1);

        if (nk_menu_begin_label(ctx, "view", NK_TEXT_ALIGN_LEFT,
                                nk_vec2(100, 100))) {
            nk_layout_row_dynamic(ctx, h, 1);
            nk_menu_item_label(ctx, "tileset", NK_TEXT_LEFT);
            nk_menu_end(ctx);
        }

        nk_menubar_end(ctx);
    }
    nk_end(ctx);

    return 1;
}
