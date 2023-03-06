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
}

void bg_render(SDL_Renderer *renderer, struct app *app)
{
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);

    int tile_size = app->map->tile_size;
    int map_size = app->map->size;

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
}
