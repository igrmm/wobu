#include "SDL.h"

#include "app.h"
#include "map.h"
#include "propertiesw.h"

static int current_selected_type = 0, current_entity_id = 0;

static int show_entity(struct nk_context *ctx,
                       struct map_entity_group *template,
                       struct map_entity *entity)
{
    char buffer[ENTITY_STR_BUFSIZ];

    // get current type of entity
    if (current_entity_id != entity->id) {
        const char *entity_type = app_get_entity_type(entity->item);
        for (int i = 0; i < template->count; i++) {
            if (SDL_strncmp(entity_type, template->ids[i], ENTITY_STR_BUFSIZ) ==
                0) {
                current_selected_type = i;
                current_entity_id = entity->id;
                break;
            }
        }
    }

    nk_layout_row_dynamic(ctx, 25, 2);

    // show entity id
    nk_label(ctx, "id", NK_TEXT_LEFT);
    SDL_snprintf(buffer, SDL_arraysize(buffer), "%i", entity->id);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_READ_ONLY, buffer,
                                   ENTITY_STR_BUFSIZ, nk_filter_decimal);

    struct map_entity_item *item = entity->item;
    while (item != NULL) {
        nk_label(ctx, item->name, NK_TEXT_LEFT);

        // show combobox with entity types
        if (SDL_strncmp(item->name, ENTITY_TYPE_TAG, ENTITY_STR_BUFSIZ) == 0) {
            int new_selected_type =
                nk_combo(ctx, template->ids, template->count,
                         current_selected_type, 25, nk_vec2i(150, 150));

            if (current_selected_type != new_selected_type) {
                SDL_Rect rect = {0};
                map_entity_get_rect(entity, &rect);
                map_destroy_entity_items(entity);
                entity->item = map_create_entity_items(
                    template->entities[new_selected_type]);
                map_entity_set_rect(entity, &rect);
                current_selected_type = new_selected_type;
            }

            // show other items
        } else {
            switch (item->type) {

            case NUMBER: {
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
        if (app->selection.entities[0] != NULL && app->selection.count == 1) {
            show_entity(ctx, &app->template, app->selection.entities[0]);
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
