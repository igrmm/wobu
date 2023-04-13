#ifndef APP_H
#define APP_H

#include "SDL.h"
#include "nk.h"

#include "map.h"

struct fps_counter {
    int frames;
    Uint32 timer;
    char fps[9];
};

struct app {
    SDL_Texture *pencil_texture;
    int screen_width, screen_height;
    SDL_Texture *tileset_texture;
    struct nk_vec2 tileset_selected;
    struct map *map;
    struct nk_vec2 bg_scroll_bkp, bg_scroll0, bg_scroll;
    struct fps_counter fps_counter;
    int show_grid, show_toolsw, show_tilesetw;
};

struct app *app_create(SDL_Renderer *renderer);
void app_handle_event(SDL_Event *evt, struct app *app);
int app_run(struct app *app, struct nk_context *ctx);
void app_render(SDL_Renderer *renderer, struct app *app);
void app_destroy(struct app *app);

#endif
