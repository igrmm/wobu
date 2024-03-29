#ifndef APP_H
#define APP_H

#include "SDL.h" // IWYU pragma: keep //clangd
#include "nk.h"

#include "map.h"
#include "modelw.h"
#include "statusw.h"

#define ENTITY_TYPE_TAG "type_cmp_type"

struct app {
    int screen_width, screen_height;
    SDL_Texture *tileset_texture;
    struct nk_vec2 tileset_selected;
    struct map *map;
    struct map_entity_group selection;
    struct map_entity_group template;
    struct modelw modelw;
    struct fps_counter fps_counter;
    int show_grid, show_toolsw, show_tilesetw, show_propertiesw;
};

char *app_get_entity_type(struct map_entity_item *item);
int app_init(struct app *app, SDL_Renderer *renderer);
void app_handle_event(struct app *app, SDL_Event *evt);
int app_run(struct app *app, struct nk_context *ctx);
void app_render(struct app *app, SDL_Renderer *renderer);
void app_deinit(struct app *app);

#endif
