#include <SDL2/SDL.h>

#include "app.h"
#include "bgw.h"

void bg_render(SDL_Renderer *renderer, struct app *app)
{
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    // prototype rendering in bg
    SDL_RenderDrawLine(renderer, 0, 0, 500, 500);
}
