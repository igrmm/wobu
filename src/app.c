#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "app.h"
#include "calc.h"
#include "colors.h"
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

    return app;
}

int app_run(struct app *app, struct nk_context *ctx)
{
    tileset_window(app, ctx, WINDOW_FLAGS);

    return 1;
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
