#ifndef APP_H
#define APP_H

#include <SDL2/SDL.h>

#include "nk.h"

struct app;
struct app *app_create(SDL_Renderer *renderer, struct nk_context *ctx);
int app_run(struct app *app);
void app_destroy(struct app *app);

#endif
