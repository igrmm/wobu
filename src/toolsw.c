#include "toolsw.h"
#include "app.h"
#include "colors.h"
#include "modelw.h"

int tools_window(struct app *app, struct nk_context *ctx, const int flags)
{
    if (nk_begin(ctx, "tools", nk_rect(20, 260, 200, 85), flags)) {
        int btn_size = 35;
        struct nk_color color_bkp = ctx->style.button.border_color;

        nk_layout_row_static(ctx, btn_size, btn_size, 3);

        // PENCIL TOOL
        if (app->modelw.current_tool->type == PENCIL)
            ctx->style.button.border_color = RED;
        if (nk_button_image(ctx,
                            nk_image_ptr(app->modelw.tools[PENCIL].texture)))
            app->modelw.current_tool = &app->modelw.tools[PENCIL];
        ctx->style.button.border_color = color_bkp;

        // ERASER TOOL
        if (app->modelw.current_tool->type == ERASER)
            ctx->style.button.border_color = RED;
        if (nk_button_image(ctx,
                            nk_image_ptr(app->modelw.tools[ERASER].texture)))
            app->modelw.current_tool = &app->modelw.tools[ERASER];
        ctx->style.button.border_color = color_bkp;

        // ENTITY TOOL
        if (app->modelw.current_tool->type == ENTITY)
            ctx->style.button.border_color = RED;
        if (nk_button_image(ctx,
                            nk_image_ptr(app->modelw.tools[ENTITY].texture)))
            app->modelw.current_tool = &app->modelw.tools[ENTITY];
        ctx->style.button.border_color = color_bkp;

    } else {
        app->show_toolsw = 0;
    }
    nk_end(ctx);

    return 0;
}
