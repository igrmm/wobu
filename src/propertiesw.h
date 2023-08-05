#ifndef PROPERTIESW_H
#define PROPERTIESW_H

#include "SDL.h"
#include "nk.h"

#include "map.h"

#define ENTITY_TEMPLATES_MAX 10

struct propertiesw {
    struct map_entity *entity_templates[ENTITY_TEMPLATES_MAX];
    const char *entity_templates_names[ENTITY_TEMPLATES_MAX];
    int number_of_entity_templates;
};

struct app;

int propertiesw_init(struct propertiesw *propertiesw);
void propertiesw_deinit(struct propertiesw *propertiesw);
int properties_window(struct app *app, struct nk_context *ctx, const int flags);

#endif
