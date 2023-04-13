#ifndef MODELW_H
#define MODELW_H

#include "SDL.h"
#include "nk.h"

struct app;

void model_window_handle_event(SDL_Event *evt, struct app *app);
void model_window_render(SDL_Renderer *renderer, struct app *app);

#endif
