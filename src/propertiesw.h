#ifndef PROPERTIESW_H
#define PROPERTIESW_H

#include "SDL.h" // IWYU pragma: keep //clangd
#include "nk.h"

#include "map.h"

struct app;

int properties_window(struct app *app, struct nk_context *ctx, const int flags);

#endif
