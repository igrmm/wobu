#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "app.h"
#include "bgw.h"
#include "calc.h"
#include "colors.h"
#include "map.h"
#include "menuw.h"
#include "statusw.h"
#include "tilesetw.h"

static const int WINDOW_FLAGS = NK_WINDOW_BORDER | NK_WINDOW_SCALABLE |
                                NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE |
                                NK_WINDOW_CLOSABLE;

struct app *app_create(SDL_Renderer *renderer)
{
    SDL_Texture *tileset_texture = IMG_LoadTexture(renderer, "tileset.png");
    if (tileset_texture == NULL) {
        SDL_Log("SDL error in tileset_texture creation: %s", SDL_GetError());
        return NULL;
    }

    struct app *app = malloc(sizeof *app);
    if (app == NULL) {
        SDL_Log("App error: unable to alloc memory.");
        return NULL;
    }

    app->map = map_create();
    if (app->map == NULL) {
        SDL_Log("Map error: unable to alloc memory.");
        app_destroy(app);
        return NULL;
    }
    SDL_Log("Map allocated memory: %i kB", (int)sizeof *app->map / 1024);

    app->tileset_texture = tileset_texture;
    app->tileset_selected = nk_vec2(-1, -1);
    app->bg_scroll = nk_vec2(0, 0);
    app->fps_counter.frames = 0;
    app->fps_counter.timer = SDL_GetTicks64();
    app->show_tilesetw = 1;

    return app;
}

void app_handle_event(SDL_Event *evt, struct app *app)
{
    bg_handle_event(evt, app);
}

int app_run(struct app *app, struct nk_context *ctx)
{
    if (app->show_tilesetw)
        tileset_window(app, ctx, WINDOW_FLAGS);

    status_window(app, ctx);
    menu_window(app, ctx);

    return 1;
}

void app_render(SDL_Renderer *renderer, struct app *app)
{
    bg_render(renderer, app);
}

void app_destroy(struct app *app)
{
    if (app != NULL) {
        if (app->tileset_texture != NULL)
            SDL_DestroyTexture(app->tileset_texture);

        if (app->map != NULL)
            free(app->map);

        free(app);
    }
    IMG_Quit();
}
