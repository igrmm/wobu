#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "app.h"

struct app {
    SDL_Renderer *renderer;
    struct nk_context *ctx;
    SDL_Texture *tileset_texture;
    struct nk_image tileset_image;
};

struct app *app_create(SDL_Renderer *renderer, struct nk_context *ctx)
{
    SDL_Texture *tileset_texture = IMG_LoadTexture(renderer, "tileset.png");
    if (!tileset_texture) {
        SDL_Log("SDL error in tileset_texture creation: %s", SDL_GetError());
        return NULL;
    }

    struct app *app = malloc(sizeof *app);
    if (!app) {
        SDL_Log("App error: unable to alloc memory.");
        return NULL;
    }

    app->renderer = renderer;
    app->ctx = ctx;
    app->tileset_texture = tileset_texture;
    app->tileset_image = nk_image_ptr(tileset_texture);

    return app;
}

int app_run(struct nk_context *ctx)
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
        nk_layout_row_dynamic(ctx, 20, 1);
        if (nk_button_label(ctx, "exit failure"))
            return 0;
    }
    nk_end(ctx);

    return 1;
}
