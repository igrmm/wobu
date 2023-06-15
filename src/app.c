#include "SDL.h"
#include "SDL_image.h"
#include "external/json.h"

#include "app.h"
#include "calc.h"
#include "colors.h"
#include "map.h"
#include "menuw.h"
#include "modelw.h"
#include "propertiesw.h"
#include "statusw.h"
#include "tilesetw.h"
#include "toolsw.h"

#define ENTITIES_JSTR_BUFSIZ 1024

static const int WINDOW_FLAGS = NK_WINDOW_BORDER | NK_WINDOW_SCALABLE |
                                NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE |
                                NK_WINDOW_CLOSABLE;

static void app_destroy_entities(struct map_entity *entities)
{
    // LOOP THROUGH ENTITIES
    struct map_entity *entity = entities;
    while (entity != NULL) {

        // LOOP THROUGH ITEMS OF CURRENT ENTITY AND DESTROY
        struct map_entity_item *item = entity->item;
        while (item != NULL) {
            struct map_entity_item *next_item = item->next;
            SDL_free(item);
            item = next_item;
        }

        // DESTROY CURRENT ENTITY
        struct map_entity *next_entity = entity->next;
        SDL_free(entity);
        entity = next_entity;
    }
}

static struct map_entity_list *app_deserialize_entities(const char *path)
{
    Uint32 now = SDL_GetTicks64();
    char entities_jstr[ENTITIES_JSTR_BUFSIZ];
    entities_jstr[0] = 0; // ALWAYS INITIALIZE C STRINGS

    SDL_RWops *file = SDL_RWFromFile(path, "r");

    if (file == NULL) {
        SDL_Log("Error opening file to load entities.");
        return 0;
    }

    // READ JSON STRING FROM FILE
    for (size_t i = 0; i < ENTITIES_JSTR_BUFSIZ; i++) {
        if (SDL_RWread(file, &entities_jstr[i], sizeof(char), 1) <= 0) {
            entities_jstr[i] = 0;
            break;
        }
    }
    SDL_RWclose(file);

    size_t len = SDL_strlen(entities_jstr);

    SDL_Log("Entities loaded from file into json string: %zu bytes.", len);

    // DESERIALIZE JSON STRING
    // todo: json error handling
    struct json_value_s *json = json_parse(entities_jstr, len);
    if (json == NULL) {
        SDL_Log("Failed parsing json when loading entities for app.");
        return NULL;
    }

    struct json_object_s *json_root = (struct json_object_s *)json->payload;
    struct json_object_element_s *entities_object = json_root->start;
    struct json_array_s *entities_array =
        json_value_as_array(entities_object->value);

    // LOOP THROUGH ENTITIES
    int number_of_entities = 1;
    struct json_array_element_s *ent_array_obj = entities_array->start;
    for (size_t i = 0; i < entities_array->length; i++) {
        struct json_object_s *ent_obj =
            (struct json_object_s *)ent_array_obj->value->payload;

        // LOOP THROUGH ENTITY ITEMS
        int number_of_items = 1;
        struct json_object_element_s *ent_item = ent_obj->start;
        while (ent_item != NULL) {
            SDL_Log("entity item K: %s", ent_item->name->string);

            switch (ent_item->value->type) {

            case json_type_string: {
                struct json_string_s *value_string =
                    json_value_as_string(ent_item->value);
                const char *value = value_string->string;
                SDL_Log("entity item V: %s", value);
            } break;

            case json_type_number: {
                struct json_number_s *value_number =
                    json_value_as_number(ent_item->value);
                int value = SDL_strtol(value_number->number, NULL, 10);
                SDL_Log("entity item V: %i", value);
            } break;

            default:
                break;
            }

            number_of_items++;
            if (number_of_items > 10) // max entity items = 10
                break;
            ent_item = ent_item->next;
        }

        number_of_entities++;
        if (number_of_entities > 10) // max entities = 10
            break;
        ent_array_obj = ent_array_obj->next;
    }

    SDL_Log("App entities deserialized with %i entities in %i ms.",
            --number_of_entities, (int)(SDL_GetTicks64() - now));

    SDL_free(json);

    return NULL;
}

static struct tool tool_init(enum tool_type tool_type, SDL_Texture *texture,
                             int r, int g, int b)
{
    SDL_Color rect_color = {r, g, b, 255};
    struct tool tool;
    tool.type = tool_type;
    tool.rect_color = rect_color;
    tool.texture = texture;
    return tool;
}

int app_init(struct app *app, SDL_Renderer *renderer)
{
    SDL_Texture *tileset_texture = IMG_LoadTexture(renderer, "tileset.png");
    if (tileset_texture == NULL) {
        SDL_Log("SDL error in tileset_texture creation: %s", SDL_GetError());
        return 0;
    }

    SDL_Texture *pencil_texture =
        IMG_LoadTexture(renderer, "../assets/pencil.png");
    if (pencil_texture == NULL) {
        SDL_Log("SDL error in pencil_texture creation: %s", SDL_GetError());
        return 0;
    }
    app->modelw.tools[PENCIL] = tool_init(PENCIL, pencil_texture, 0, 255, 0);

    SDL_Texture *eraser_texture =
        IMG_LoadTexture(renderer, "../assets/eraser.png");
    if (eraser_texture == NULL) {
        SDL_Log("SDL error in eraser creation: %s", SDL_GetError());
        return 0;
    }
    app->modelw.tools[ERASER] = tool_init(ERASER, eraser_texture, 255, 0, 0);

    reset_tool_rect(&app->modelw.tool_rect);

    app->map = map_create();
    if (app->map == NULL) {
        SDL_Log("Map error: unable to alloc memory.");
        app_deinit(app);
        return 0;
    }
    SDL_Log("Map allocated memory: %i kB", (int)sizeof *app->map / 1024);

    // centralize bg
    SDL_GetRendererOutputSize(renderer, &app->screen_width,
                              &app->screen_height);
    reset_pan_and_zoom(app);

    app->modelw.current_tool = &app->modelw.tools[PENCIL];
    app->tileset_texture = tileset_texture;
    app->tileset_selected = nk_vec2(-1, -1);
    app->fps_counter.frames = 0;
    app->fps_counter.timer = SDL_GetTicks64();
    app->show_grid = 1;
    app->show_toolsw = 1;
    app->show_tilesetw = 1;

    app_deserialize_entities("../assets/entities.json");

    return 1;
}

void app_handle_event(struct app *app, SDL_Event *evt)
{
    model_window_handle_event(evt, app);
}

int app_run(struct app *app, struct nk_context *ctx)
{
    if (app->show_toolsw)
        tools_window(app, ctx, WINDOW_FLAGS);

    if (app->show_tilesetw)
        tileset_window(app, ctx, WINDOW_FLAGS);

    if (app->show_propertiesw)
        properties_window(app, ctx, WINDOW_FLAGS);

    status_window(app, ctx);
    menu_window(app, ctx);

    return 1;
}

void app_render(struct app *app, SDL_Renderer *renderer)
{
    model_window_render(renderer, app);
}

void app_deinit(struct app *app)
{
    for (int tool = 0; tool < NUMBER_OF_TOOLS; tool++)
        SDL_DestroyTexture(app->modelw.tools[tool].texture);

    if (app->tileset_texture != NULL)
        SDL_DestroyTexture(app->tileset_texture);

    if (app->map != NULL)
        free(app->map);

    IMG_Quit();
}
