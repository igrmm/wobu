#ifndef MODELW_H
#define MODELW_H

#include "SDL.h"
#include "nk.h"

enum tool_type { PENCIL, NUMBER_OF_TOOLS };

struct tool {
    enum tool_type type;
    SDL_FRect rect;
    SDL_Color rect_color;
    SDL_Texture *texture;
};

struct modelw {
    SDL_FPoint offset;
    SDL_FPoint pan_start;
    float scale;
};

struct app;

void model_window_handle_event(SDL_Event *evt, struct app *app);
void model_window_render(SDL_Renderer *renderer, struct app *app);

#endif
