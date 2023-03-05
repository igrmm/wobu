#include "tilesetw.h"
#include "app.h"
#include "calc.h"
#include "colors.h"

static void select_tile_on_click(struct app *app, struct nk_context *ctx,
                                 struct nk_rect tileset_rect)
{
    if (nk_input_is_mouse_click_in_rect(&ctx->input, NK_BUTTON_LEFT,
                                        tileset_rect)) {
        int tile_size = app->map->tile_size;

        float tileset_selected_x =
            (int)((ctx->input.mouse.pos.x - tileset_rect.x +
                   ctx->current->scrollbar.x) /
                  tile_size) *
            tile_size;
        float tileset_selected_y =
            (int)((ctx->input.mouse.pos.y - tileset_rect.y +
                   ctx->current->scrollbar.y) /
                  tile_size) *
            tile_size;

        // sets the x,y position of selected tile in tileset
        app->tileset_selected = nk_vec2(tileset_selected_x, tileset_selected_y);
    }
}

static void render(struct app *app, struct nk_context *ctx,
                   struct nk_command_buffer *canvas,
                   struct nk_rect tileset_rect)
{
    struct nk_image tileset_image = nk_image_ptr(app->tileset_texture);
    int tile_size = app->map->tile_size;

    // draw tileset
    nk_image(ctx, tileset_image);

    // draw grid - column lines
    int cols = tileset_rect.w + ctx->current->scrollbar.x;
    for (int col = 0; col <= cols; col += tile_size) {
        float col0_x = tileset_rect.x + col - ctx->current->scrollbar.x;
        float col0_y = tileset_rect.y;
        float col1_x = tileset_rect.x + col - ctx->current->scrollbar.x;
        float col1_y = tileset_rect.y + tileset_rect.h;

        nk_stroke_line(canvas, col0_x, col0_y, col1_x, col1_y, 1.0f, GREY);
    }

    // draw grid - row lines
    int rows = tileset_rect.h + ctx->current->scrollbar.y;
    for (int row = 0; row <= rows; row += tile_size) {
        float row0_x = tileset_rect.x;
        float row0_y = tileset_rect.y + row - ctx->current->scrollbar.y;
        float row1_x = tileset_rect.x + tileset_rect.w;
        float row1_y = tileset_rect.y + row - ctx->current->scrollbar.y;

        nk_stroke_line(canvas, row0_x, row0_y, row1_x, row1_y, 1.0f, GREY);
    }

    // draws semi transparent green rect on top of selected tile
    if (app->tileset_selected.x >= 0 && app->tileset_selected.y >= 0) {
        float green_rect_x = tileset_rect.x + app->tileset_selected.x -
                             ctx->current->scrollbar.x;
        float green_rect_y = tileset_rect.y + app->tileset_selected.y -
                             ctx->current->scrollbar.y;

        nk_fill_rect(canvas,
                     nk_rect(green_rect_x, green_rect_y, tile_size, tile_size),
                     0, nk_rgba(GREEN.r, GREEN.g, GREEN.b, 150));
    }
}

int tileset_window(struct app *app, struct nk_context *ctx, const int flags)
{
    int tileset_w, tileset_h;
    SDL_QueryTexture(app->tileset_texture, NULL, NULL, &tileset_w, &tileset_h);

    // backup padding
    struct nk_vec2 padding_bkp = ctx->style.window.padding;
    ctx->style.window.padding = nk_vec2(0, 0);

    if (nk_begin(ctx, "tileset", nk_rect(20, 20, 200, 200), flags)) {
        nk_layout_row_static(ctx, tileset_h, tileset_w, 1);

        struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);

        float w = min(tileset_w, canvas->clip.w);
        float h = min(tileset_h, canvas->clip.h);
        struct nk_rect tileset_rect =
            nk_rect(canvas->clip.x, canvas->clip.y, w, h);

        select_tile_on_click(app, ctx, tileset_rect);
        render(app, ctx, canvas, tileset_rect);
    }
    nk_end(ctx);

    // restore padding
    ctx->style.window.padding = padding_bkp;

    return 1;
}
