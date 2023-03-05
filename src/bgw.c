#include <SDL2/SDL.h>

#include "app.h"
#include "bgw.h"

void bg_render(SDL_Renderer *renderer, struct app *app)
{
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);

    int cols = app->world_size + 1;
    for (int col = 0; col < cols; col++)
        SDL_RenderDrawLine(renderer, col * app->tile_size + app->bg_scroll.x,
                           app->bg_scroll.y,
                           col * app->tile_size + app->bg_scroll.x,
                           app->world_size * app->tile_size + app->bg_scroll.y);

    int rows = app->world_size + 1;
    for (int row = 0; row < rows; row++)
        SDL_RenderDrawLine(renderer, app->bg_scroll.x,
                           row * app->tile_size + app->bg_scroll.y,
                           app->world_size * app->tile_size + app->bg_scroll.x,
                           row * app->tile_size + app->bg_scroll.y);
}
