#ifndef APP_H
#define APP_H

#include "SDL.h"
#include "nk.h"

#include "map.h"
#include "modelw.h"
#include "statusw.h"

struct app {
    int screen_width, screen_height;
    SDL_Texture *tileset_texture;
    struct nk_vec2 tileset_selected;
    struct map *map;
    struct modelw modelw;
    struct fps_counter fps_counter;
    int show_grid, show_toolsw, show_tilesetw;
};

int app_init(struct app *app, SDL_Renderer *renderer);
void app_handle_event(struct app *app, SDL_Event *evt);
int app_run(struct app *app, struct nk_context *ctx);
void app_render(struct app *app, SDL_Renderer *renderer);
void app_deinit(struct app *app);

#endif
