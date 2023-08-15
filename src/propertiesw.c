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
    propertiesw->entity_templates[0] = map_deserialize_entities(json, NULL);
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

static int show_entity(struct nk_context *ctx, struct propertiesw *propertiesw,
                       struct map_entity *entity)
{
    // get current type of entity
    if (propertiesw->selected_entity_template < 0) {
        const char *entity_type = get_type_cmp_value(entity->item);
        for (int i = 0; i < propertiesw->number_of_entity_templates; i++) {
            if (SDL_strncmp(entity_type, propertiesw->entity_templates_names[i],
                            ENTITY_STR_BUFSIZ) == 0) {
                propertiesw->selected_entity_template = i;
                break;
            }
        }
    }

    nk_layout_row_dynamic(ctx, 25, 2);
    struct map_entity_item *item = entity->item;
    while (item != NULL) {
        nk_label(ctx, item->name, NK_TEXT_LEFT);

        // show combobox with entity types
        if (SDL_strncmp(item->name, "type_cmp_type", ENTITY_STR_BUFSIZ) == 0) {
            int selected = nk_combo(ctx, propertiesw->entity_templates_names,
                                    propertiesw->number_of_entity_templates,
                                    propertiesw->selected_entity_template, 25,
                                    nk_vec2i(150, 150));

            if (propertiesw->selected_entity_template != selected) {
                SDL_Rect rect = {0};
                map_entity_get_rect(entity, &rect);
                map_destroy_entity_items(entity);
                entity->item = map_create_entity_items(
                    propertiesw->entity_templates[selected]);
                map_entity_set_rect(entity, &rect);
            }
            propertiesw->selected_entity_template = selected;

            // show other items
        } else {
            switch (item->type) {

            case NUMBER: {
                char buffer[ENTITY_STR_BUFSIZ];
                SDL_snprintf(buffer, SDL_arraysize(buffer), "%i",
                             item->value.number);
                nk_edit_string_zero_terminated(ctx, NK_EDIT_SIMPLE, buffer,
                                               ENTITY_STR_BUFSIZ,
                                               nk_filter_decimal);
                item->value.number = SDL_strtol(buffer, NULL, 10);
                break;
            }

            case STRING: {
                nk_edit_string_zero_terminated(
                    ctx, NK_EDIT_SIMPLE, item->value.string, ENTITY_STR_BUFSIZ,
                    nk_filter_default);
                break;
            }

            default:
                SDL_Log("Map entity item type not supported.");
                return 0;
            }
        }

        item = item->next;
    }
    return 1;
}

int properties_window(struct app *app, struct nk_context *ctx, const int flags)
{
    if (nk_begin(ctx, "properties", nk_rect(20, 365, 200, 200), flags)) {
        if (app->selected_entities != NULL) {
            show_entity(ctx, &app->propertiesw, app->selected_entities);
        } else {
            nk_layout_row_dynamic(ctx, 25, 1);
            nk_label(ctx, "no selection", NK_TEXT_LEFT);
        }
    } else {
        app->show_propertiesw = 0;
    }
    nk_end(ctx);

    return 1;
}
