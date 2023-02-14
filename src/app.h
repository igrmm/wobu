#ifndef APP_H
#define APP_H

#include <SDL2/SDL.h>

#include "nk.h"

struct app {
    SDL_Texture *tileset_texture;
    struct nk_vec2 tileset_selected;
    int tile_size;
};

struct app *app_create(SDL_Renderer *renderer);
int app_run(struct app *app, struct nk_context *ctx);
void app_destroy(struct app *app);

#endif
