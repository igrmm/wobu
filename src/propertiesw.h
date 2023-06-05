#ifndef PROPERTIESW_H
#define PROPERTIESW_H

#include "SDL.h"
#include "nk.h"

struct app;

int properties_window(struct app *app, struct nk_context *ctx, const int flags);

#endif
