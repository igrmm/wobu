#include <SDL2/SDL.h>

#include "app.h"
#include "bgw.h"

void bg_render(SDL_Renderer *renderer, struct app *app)
{
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);

    int cols = app->world_size / app->tile_size;
    for (int col = 0; col < cols; col++)
        SDL_RenderDrawLine(renderer, col * app->tile_size, 0,
                           col * app->tile_size, app->world_size);

    int rows = app->world_size / app->tile_size;
    for (int row = 0; row < rows; row++)
        SDL_RenderDrawLine(renderer, 0, row * app->tile_size, app->world_size,
                           row * app->tile_size);
}
