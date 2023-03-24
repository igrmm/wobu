#ifndef MODELW_H
#define MODELW_H

#include "nk.h"

#include "app.h"

void model_window_handle_event(SDL_Event *evt, struct app *app);
void model_window_render(SDL_Renderer *renderer, struct app *app);

#endif
