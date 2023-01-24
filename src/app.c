#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "app.h"

static const int WINDOW_FLAGS = NK_WINDOW_BORDER | NK_WINDOW_SCALABLE |
                                NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE |
                                NK_WINDOW_CLOSABLE;

struct app {
    SDL_Renderer *renderer;
    struct nk_context *ctx;
    SDL_Texture *tileset_texture;
    struct nk_image tileset_image;
};

struct app *app_create(SDL_Renderer *renderer, struct nk_context *ctx)
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

    app->renderer = renderer;
    app->ctx = ctx;
    app->tileset_texture = tileset_texture;
    app->tileset_image = nk_image_ptr(tileset_texture);

    return app;
}

int app_run(struct app *app)
{
    struct nk_context *ctx = app->ctx;
    if (nk_begin(ctx, "wobu", nk_rect(20, 20, 200, 300), WINDOW_FLAGS)) {
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "this is a window", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 20, 1);
        if (nk_button_label(ctx, "exit failure"))
            return 0;
    }
    nk_end(ctx);

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
