#include <stdio.h>

#include "app.h"
#include "statusw.h"

int status_window(struct app *app, struct nk_context *ctx)
{
    int h = 20;
    int w = app->screen_width;
    int x = 0;
    int y = app->screen_height - h;

    if (nk_begin(ctx, "status", nk_rect(x, y, w, h), NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_dynamic(ctx, h, 1);
        nk_label(ctx, "fps", NK_TEXT_ALIGN_LEFT);
    }
    nk_end(ctx);

    return 1;
}
