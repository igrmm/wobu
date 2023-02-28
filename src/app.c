#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "app.h"
#include "bgw.h"
#include "calc.h"
#include "colors.h"
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

    app->tileset_texture = tileset_texture;
    app->tileset_selected = nk_vec2(-1, -1);
    app->tile_size = 32;
    app->world_size = 640;
    app->bg_scroll = nk_vec2(0, 0);

    return app;
}

void app_handle_event(SDL_Event *evt, struct app *app)
{
    if (evt->button.button == SDL_BUTTON_MIDDLE) {
        if (evt->type == SDL_MOUSEBUTTONDOWN) {
            app->bg_scroll_bkp = app->bg_scroll;
            app->bg_scroll0.x = evt->button.x;
            app->bg_scroll0.y = evt->button.y;

        } else if (evt->type == SDL_MOUSEMOTION) {
            app->bg_scroll.x =
                app->bg_scroll_bkp.x + evt->button.x - app->bg_scroll0.x;

            app->bg_scroll.y =
                app->bg_scroll_bkp.y + evt->button.y - app->bg_scroll0.y;
        }
    }
}

int app_run(struct app *app, struct nk_context *ctx)
{
    tileset_window(app, ctx, WINDOW_FLAGS);
    status_window(app, ctx);

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

        free(app);
    }
    IMG_Quit();
}
