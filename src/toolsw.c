#include "toolsw.h"
#include "app.h"
#include "modelw.h"

int tools_window(struct app *app, struct nk_context *ctx, const int flags)
{
    int btn_size = 35;

    if (nk_begin(ctx, "tools", nk_rect(20, 260, 200, 85), flags)) {
        nk_layout_row_static(ctx, btn_size, btn_size, 1);
        nk_button_image(ctx, nk_image_ptr(app->tools[PENCIL].texture));
    } else {
        app->show_toolsw = 0;
    }
    nk_end(ctx);

    return 0;
}
