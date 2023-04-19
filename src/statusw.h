#ifndef STATUSW_H
#define STATUSW_H

#include "SDL.h"
#include "nk.h"

struct fps_counter {
    int frames;
    Uint32 timer;
    char fps[9];
};

struct app;

int status_window(struct app *app, struct nk_context *ctx);

#endif
