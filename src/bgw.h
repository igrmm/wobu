#ifndef BGW_H
#define BGW_H

#include "nk.h"

#include "app.h"

void bg_handle_event(SDL_Event *evt, struct app *app);
void bg_render(SDL_Renderer *renderer, struct app *app);

#endif
