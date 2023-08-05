#include "SDL.h"
#include "external/json.h"

#include "app.h"
#include "jsonffile.h"
#include "map.h"
#include "propertiesw.h"

static char *get_type_cmp_value(struct map_entity_item *item)
{
    size_t name_size = 0;
    while (item != NULL) {
        if (item->type == STRING) {
            name_size = SDL_arraysize(item->name);
            if (SDL_strncmp(item->name, "type_cmp_type", name_size) == 0) {
                return item->value.string;
            }
        }
        item = item->next;
    }
    return 0;
}

int propertiesw_init(struct propertiesw *propertiesw)
{
    // LOAD ENTITY TEMPLATES
    SDL_Log("Attempting to deserialize entity templates...");
    struct json_value_s *json =
        json_from_file("../assets/entity_templates.json");
    if (json == NULL) {
        SDL_Log("Error parsing entitiy templates json.");
        return 0;
    }
    propertiesw->entity_templates[0] = map_deserialize_entities(json);
    if (propertiesw->entity_templates[0] == NULL) {
        SDL_Log("Error deserializing entity templates.");
        return 0;
    }

    // NAME OF THE FIRST ENTITY TEMPLATE TYPE CMP
    propertiesw->entity_templates_names[0] =
        get_type_cmp_value(propertiesw->entity_templates[0]->item);

    // LOOP THROUGH ENTITY TEMPLATES
    struct map_entity *entity = propertiesw->entity_templates[0]->next;
    size_t entity_templates_max = SDL_arraysize(propertiesw->entity_templates);
    size_t i = 1;
    for (; i < entity_templates_max; i++) {
        if (entity != NULL) {
            propertiesw->entity_templates[i] = entity;
            propertiesw->entity_templates_names[i] =
                get_type_cmp_value(entity->item);
            entity = entity->next;
        } else {
            break;
        }
    }
    propertiesw->number_of_entity_templates = i;

    return 1;
}

void propertiesw_deinit(struct propertiesw *propertiesw)
{
    if (propertiesw->entity_templates[0] != NULL) {
        map_destroy_entities(propertiesw->entity_templates[0]);
    }
}

int properties_window(struct app *app, struct nk_context *ctx, const int flags)
{
    const char *combo[] = {"no selection"};
    static int selected = 0;

    if (nk_begin(ctx, "properties", nk_rect(20, 365, 200, 200), flags)) {
        nk_layout_row_dynamic(ctx, 25, 1);
        selected = nk_combo(ctx, combo, NK_LEN(combo), selected, 25,
                            nk_vec2i(150, 150));
    } else {
        app->show_propertiesw = 0;
    }
    nk_end(ctx);

    return 1;
}
