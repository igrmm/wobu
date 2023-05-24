#include "SDL.h"
#include "SDL_image.h"

#include "app.h"
#include "calc.h"
#include "colors.h"
#include "map.h"
#include "menuw.h"
#include "modelw.h"
#include "statusw.h"
#include "tilesetw.h"
#include "toolsw.h"

static const int WINDOW_FLAGS = NK_WINDOW_BORDER | NK_WINDOW_SCALABLE |
                                NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE |
                                NK_WINDOW_CLOSABLE;

int app_init(struct app *app, SDL_Renderer *renderer)
{
    SDL_Texture *tileset_texture = IMG_LoadTexture(renderer, "tileset.png");
    if (tileset_texture == NULL) {
        SDL_Log("SDL error in tileset_texture creation: %s", SDL_GetError());
        return 0;
    }

    SDL_Texture *pencil_texture =
        IMG_LoadTexture(renderer, "../assets/pencil.png");
    if (pencil_texture == NULL) {
        SDL_Log("SDL error in tools_texture creation: %s", SDL_GetError());
        return 0;
    }

    app->map = map_create();
    if (app->map == NULL) {
        SDL_Log("Map error: unable to alloc memory.");
        app_deinit(app);
        return 0;
    }
    SDL_Log("Map allocated memory: %i kB", (int)sizeof *app->map / 1024);

    // centralize bg
    SDL_GetRendererOutputSize(renderer, &app->screen_width,
                              &app->screen_height);
    int map_size = app->map->size * app->map->tile_size;
    int bg_scroll_x = (app->screen_width - map_size) / 2;
    int bg_scroll_y = (app->screen_height - map_size) / 2;

    app->pencil_texture = pencil_texture;
    app->tileset_texture = tileset_texture;
    app->tileset_selected = nk_vec2(-1, -1);
    app->bg_scroll = nk_vec2(bg_scroll_x, bg_scroll_y);
    app->modelw.offset.x = app->modelw.offset.y = 0;
    app->modelw.pan_start.x = app->modelw.pan_start.y = 0;
    app->modelw.scale = 1;
    app->fps_counter.frames = 0;
    app->fps_counter.timer = SDL_GetTicks64();
    app->show_grid = 1;
    app->show_toolsw = 1;
    app->show_tilesetw = 1;

    return 1;
}

void app_handle_event(struct app *app, SDL_Event *evt)
{
    model_window_handle_event(evt, app);
}

int app_run(struct app *app, struct nk_context *ctx)
{
    if (app->show_toolsw)
        tools_window(app, ctx, WINDOW_FLAGS);

    if (app->show_tilesetw)
        tileset_window(app, ctx, WINDOW_FLAGS);

    status_window(app, ctx);
    menu_window(app, ctx);

    return 1;
}

void app_render(struct app *app, SDL_Renderer *renderer)
{
    model_window_render(renderer, app);
}

void app_deinit(struct app *app)
{
    if (app->pencil_texture != NULL)
        SDL_DestroyTexture(app->pencil_texture);

    if (app->tileset_texture != NULL)
        SDL_DestroyTexture(app->tileset_texture);

    if (app->map != NULL)
        free(app->map);

    IMG_Quit();
}
