#include <SDL2/SDL.h>

#include "app.h"
#include "bgw.h"

void bg_handle_event(SDL_Event *evt, struct app *app)
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

    if (evt->button.button == SDL_BUTTON_LEFT &&
        (evt->type == SDL_MOUSEBUTTONDOWN || evt->type == SDL_MOUSEMOTION)) {
        int tile_x = (evt->button.x - app->bg_scroll.x) / app->map->tile_size;
        int tile_y = (evt->button.y - app->bg_scroll.y) / app->map->tile_size;
        if (tile_x < app->map->size && tile_y < app->map->size) {
            app->map->tiles[tile_x][tile_y].x = app->tileset_selected.x;
            app->map->tiles[tile_x][tile_y].y = app->tileset_selected.y;
        }
    }
}

void bg_render(SDL_Renderer *renderer, struct app *app)
{
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);

    int tile_size = app->map->tile_size;
    int map_size = app->map->size;
    SDL_Rect srcrect = {0, 0, tile_size, tile_size};
    SDL_Rect dstrect = {0, 0, tile_size, tile_size};

    int cols = map_size + 1;
    for (int col = 0; col < cols; col++)
        SDL_RenderDrawLine(renderer, col * tile_size + app->bg_scroll.x,
                           app->bg_scroll.y, col * tile_size + app->bg_scroll.x,
                           map_size * tile_size + app->bg_scroll.y);

    int rows = map_size + 1;
    for (int row = 0; row < rows; row++)
        SDL_RenderDrawLine(renderer, app->bg_scroll.x,
                           row * tile_size + app->bg_scroll.y,
                           map_size * tile_size + app->bg_scroll.x,
                           row * tile_size + app->bg_scroll.y);

    for (int i = 0; i < TILES_MAX; i++) {
        for (int j = 0; j < TILES_MAX; j++) {
            srcrect.x = app->map->tiles[i][j].x;
            srcrect.y = app->map->tiles[i][j].y;
            if (srcrect.x >= 0 && srcrect.y >= 0) {
                dstrect.x = i * tile_size + app->bg_scroll.x;
                dstrect.y = j * tile_size + app->bg_scroll.y;
                SDL_RenderCopy(renderer, app->tileset_texture, &srcrect,
                               &dstrect);
            }
        }
    }
}
