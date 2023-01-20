#include "app.h"

int app(struct nk_context *ctx)
{
    static int window_flags = 0;
    window_flags |= NK_WINDOW_BORDER;
    window_flags |= NK_WINDOW_SCALABLE;
    window_flags |= NK_WINDOW_MOVABLE;
    window_flags |= NK_WINDOW_MINIMIZABLE;
    window_flags |= NK_WINDOW_BACKGROUND;
    if (nk_begin(ctx, "wobu", nk_rect(20, 20, 200, 300), window_flags)) {
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "this is a window", NK_TEXT_LEFT);
    }
    nk_end(ctx);

    return 0;
}
