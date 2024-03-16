#include "../external/json.h/json.h"
#include "SDL.h"

#include "jsonffile.h"
#include "map.h"

static int id;

void map_reset_tiles(struct map *map)
{
    for (int i = 0; i < TILES_MAX; i++) {
        for (int j = 0; j < TILES_MAX; j++) {
            map->tiles[i][j].x = map->tiles[i][j].y = -1;
        }
    }
}

int map_make_id(void) { return ++id; }

struct map *map_create(void)
{
    struct map *map = SDL_calloc(1, sizeof(*map));
    if (map == NULL)
        return NULL;

    map->tile_size = 32;
    map->size = 20;

    map_reset_tiles(map);
    map->entities.head = map->entities.tail = NULL;

    return map;
}

static int rect_get_or_set(int get, struct map_entity *entity, SDL_Rect *rect)
{
    if (entity == NULL || rect == NULL) {
        return 0;
    }

    int x_set = 0, y_set = 0, w_set = 0, h_set = 0;
    struct map_entity_item *item = entity->item;

    while (item != NULL) {
        if (SDL_strncmp(item->name, "rect_cmp_x", ENTITY_STR_BUFSIZ) == 0 &&
            !x_set) {
            get ? (rect->x = item->value.number)
                : (item->value.number = rect->x);
            x_set = 1;
        }
        if (SDL_strncmp(item->name, "rect_cmp_y", ENTITY_STR_BUFSIZ) == 0 &&
            !y_set) {
            get ? (rect->y = item->value.number)
                : (item->value.number = rect->y);
            y_set = 1;
        }
        if (SDL_strncmp(item->name, "rect_cmp_w", ENTITY_STR_BUFSIZ) == 0 &&
            !w_set) {
            get ? (rect->w = item->value.number)
                : (item->value.number = rect->w);
            w_set = 1;
        }
        if (SDL_strncmp(item->name, "rect_cmp_h", ENTITY_STR_BUFSIZ) == 0 &&
            !h_set) {
            get ? (rect->h = item->value.number)
                : (item->value.number = rect->h);
            h_set = 1;
        }
        if (x_set && y_set && w_set && h_set) {
            break;
        }
        item = item->next;
    }

    return x_set && y_set && w_set && h_set;
}

int map_entity_get_frect(struct map_entity *entity, SDL_FRect *frect)
{
    SDL_Rect rect = {0};
    int ret = rect_get_or_set(1, entity, &rect);
    frect->x = rect.x, frect->y = rect.y, frect->w = rect.w, frect->h = rect.h;

    return ret;
}

int map_entity_get_rect(struct map_entity *entity, SDL_Rect *rect)
{
    return rect_get_or_set(1, entity, rect);
}

int map_entity_set_rect(struct map_entity *entity, SDL_Rect *rect)
{
    return rect_get_or_set(0, entity, rect);
}

void map_print_entity(struct map_entity *entity)
{
    if (entity == NULL) {
        return;
    }

    if (entity->item == NULL) {
        return;
    }

    SDL_Log("------- PRINTING ENTITY -------");
    size_t mem = sizeof(*entity);

    struct map_entity_item *item = entity->item;
    while (item != NULL) {
        mem += sizeof(*item);
        SDL_Log("NAME: %s", item->name);

        switch (item->type) {

        case NUMBER:
            SDL_Log("VALUE: %i", item->value.number);
            break;

        case STRING:
            SDL_Log("VALUE: %s", item->value.string);
            break;

        default:
            SDL_Log("Map entity item type not supported.");
            return;
        }

        item = item->next;
    }
    SDL_Log("Memory allocated: %zu bytes", mem);
}

struct map_entity_item *
map_create_entity_items(struct map_entity *entity_template)
{
    struct map_entity_item *items = NULL;
    struct map_entity_item **item = &items;
    struct map_entity_item *item_template = entity_template->item;

    while (item_template != NULL) {
        *item = SDL_malloc(sizeof(struct map_entity_item));
        (*item)->next = NULL;
        SDL_snprintf((*item)->name, sizeof((*item)->name), "%s",
                     item_template->name);

        switch (item_template->type) {

        case NUMBER: {
            (*item)->type = NUMBER;
            (*item)->value.number = item_template->value.number;
        } break;

        case STRING: {
            (*item)->type = STRING;
            SDL_snprintf((*item)->value.string, sizeof((*item)->value.string),
                         "%s", item_template->value.string);
        } break;

        default:
            SDL_Log("Map entity item type not supported.");
            return NULL;
        }

        item = &(*item)->next;
        item_template = item_template->next;
    }

    return items;
}

struct map_entity *map_create_entity(struct map_entity *entity_template)
{
    struct map_entity *entity = SDL_malloc(sizeof(struct map_entity));
    entity->item = map_create_entity_items(entity_template);
    entity->id = map_make_id();
    entity->next = NULL;
    entity->prev = NULL;

    return entity;
}

static int map_jstr_cat(char *map_jstr, const char *fmt, ...)
{
    char buffer[512];
    buffer[0] = 0;

    va_list args;
    va_start(args, fmt);
    size_t len = SDL_vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (len >= sizeof(buffer)) {
        SDL_Log("map_jstr_cat error: buffer overflow");
        return 0;
    }

    len = SDL_strlcat(map_jstr, buffer, JSON_BUFSIZ);
    if (len >= JSON_BUFSIZ) {
        SDL_Log("map_jstr_cat error: map_jstr overflow");
        return 0;
    }

    return 1;
}

size_t map_destroy_entity_items(struct map_entity *entity)
{
    size_t mem = 0;
    struct map_entity_item *item = entity->item;
    struct map_entity_item *next_item;
    while (item != NULL) {
        next_item = item->next;
        mem += sizeof(*item);
        SDL_free(item);
        item = next_item;
    }
    return mem;
}

size_t map_destroy_entity(struct map_entity *entity)
{
    size_t mem = map_destroy_entity_items(entity);
    mem += sizeof(*entity);
    SDL_free(entity);
    return mem;
}

void map_destroy_entities(struct map_entity *entities)
{
    size_t mem = 0;
    struct map_entity *entity = entities;
    struct map_entity *next_entity;
    while (entity != NULL) {
        next_entity = entity->next;
        mem += map_destroy_entity(entity);
        entity = next_entity;
    }
    SDL_Log("Entities destroyed, memory freed: %zu bytes", mem);
}

void map_entities_add(struct map_entity *entity, struct map_entities *entities)
{
    if (entities->head == NULL) {
        entities->head = entity;
    }

    if (entities->tail != NULL) {
        entities->tail->next = entity;
        entity->prev = entities->tail;
    }

    entities->tail = entity;
}

void map_entities_remove(struct map_entity *entity,
                         struct map_entities *entities)
{
    if (entities->tail == entity && entities->head != entity) {
        entity->prev->next = NULL;
        entities->tail = entities->tail->prev;
    }

    if (entities->head == entity && entities->tail != entity) {
        entity->next->prev = NULL;
        entities->head = entities->head->next;
    }

    if (entity->next != NULL && entity->prev != NULL) {
        entity->prev->next = entity->next;
        entity->next->prev = entity->prev;
    }

    if (entities->head == entity && entities->tail == entity) {
        entities->head = entities->tail = NULL;
    }

    map_destroy_entity(entity);
}

struct map_entity *map_deserialize_entities(struct json_array_s *entities_array,
                                            struct map_entity **tail)
{
    size_t mem = 0;
    Uint32 now = SDL_GetTicks64();
    // DESERIALIZE JSON STRING
    // todo: json error handling
    struct map_entity *entities = NULL, *prev = NULL;
    struct map_entity **entity = &entities;

    // LOOP THROUGH ENTITIES
    int number_of_entities = 0;
    struct json_array_element_s *ent_array_obj = entities_array->start;
    for (size_t i = 0; i < entities_array->length; i++) {
        struct json_object_s *ent_obj =
            (struct json_object_s *)ent_array_obj->value->payload;

        // LOOP THROUGH ENTITY ITEMS
        int number_of_items = 0;
        struct json_object_element_s *ent_item = ent_obj->start;

        *entity = SDL_malloc(sizeof(struct map_entity));
        mem += sizeof(**entity);
        (*entity)->next = NULL;
        (*entity)->prev = prev;
        struct map_entity_item **item = &(*entity)->item;

        while (ent_item != NULL) {
            *item = SDL_malloc(sizeof(struct map_entity_item));
            mem += sizeof(**item);
            (*item)->next = NULL;
            SDL_snprintf((*item)->name, sizeof((*item)->name), "%s",
                         ent_item->name->string);

            switch (ent_item->value->type) {

            case json_type_string: {
                struct json_string_s *value_string =
                    json_value_as_string(ent_item->value);
                const char *value = value_string->string;
                (*item)->type = STRING;
                SDL_snprintf((*item)->value.string,
                             sizeof((*item)->value.string), "%s", value);
            } break;

            case json_type_number: {
                struct json_number_s *value_number =
                    json_value_as_number(ent_item->value);
                int value = SDL_strtol(value_number->number, NULL, 10);
                (*item)->type = NUMBER;
                (*item)->value.number = value;
            } break;

            default:
                SDL_Log("json value type not supported: %zu [name=%s]",
                        ent_item->value->type, ent_item->name->string);
                return NULL;
            }

            item = &(*item)->next;

            number_of_items++;
            if (number_of_items >= 10) { // max entity items = 10
                SDL_Log("Error: max number of map entity items reached.");
                return NULL;
            }
            ent_item = ent_item->next;
        }

        prev = *entity;
        entity = &(*entity)->next;

        number_of_entities++;
        if (number_of_entities >= 10) { // max entities = 10
            SDL_Log("Error: max number of entities reached.");
            return NULL;
        }
        ent_array_obj = ent_array_obj->next;
    }

    if (tail != NULL) {
        *tail = prev;
    }

    SDL_Log("Entities deserialized with %i entities in %i ms with %zu bytes.",
            number_of_entities, (int)(SDL_GetTicks64() - now), mem);

    return entities;
}

int map_serialize(struct map *map, const char *path)
{
    Uint32 now = SDL_GetTicks64();
    char map_jstr[JSON_BUFSIZ];
    map_jstr[0] = 0; // ALWAYS INITIALIZE C STRINGS

    SDL_RWops *file = SDL_RWFromFile(path, "w");

    if (file == NULL) {
        SDL_Log("Error opening file to write map.");
        goto serialization_error;
    }

    // BEGINNING OF JSON STRING + TILES
    if (!map_jstr_cat(map_jstr, "{\n    \"tiles\": [")) {
        goto serialization_error;
    }

    // WRITE TILES TO JSON STRING
    int tile_x = 0, tile_y = 0, first_tile = 1, total_tiles = 0;

    for (int i = 0; i < map->size; i++) {
        for (int j = 0; j < map->size; j++) {

            tile_x = map->tiles[i][j].x;
            tile_y = map->tiles[i][j].y;

            if (tile_x >= 0 && tile_y >= 0) {
                if (first_tile) {
                    first_tile = 0;
                    if (!map_jstr_cat(map_jstr, "[%i, %i, %i, %i]", i, j,
                                      tile_x, tile_y)) {
                        goto serialization_error;
                    }
                } else {
                    if (!map_jstr_cat(map_jstr, ",[%i, %i, %i, %i]", i, j,
                                      tile_x, tile_y)) {
                        goto serialization_error;
                    }
                }
                total_tiles++;
            }
        }
    }

    // ENDING OF TILES
    if (!map_jstr_cat(map_jstr, "],\n")) {
        goto serialization_error;
    }

    // BEGIN OF ENTITIES
    if (!map_jstr_cat(map_jstr, "    \"entities\": [")) {
        goto serialization_error;
    }

    struct map_entity *entity = map->entities.head;
    while (entity != NULL) {
        if (!map_jstr_cat(map_jstr, "\n        {")) {
            goto serialization_error;
        }
        struct map_entity_item *item = entity->item;
        while (item != NULL) {
            if (!map_jstr_cat(map_jstr, "\n            \"%s\": ", item->name)) {
                goto serialization_error;
            }
            switch (item->type) {

            case NUMBER:
                if (!map_jstr_cat(map_jstr, "%i", item->value.number)) {
                    goto serialization_error;
                }
                break;
            case STRING:
                if (!map_jstr_cat(map_jstr, "\"%s\"", item->value.string)) {
                    goto serialization_error;
                }
                break;
            case BOOL:
                break;
            }
            if (item->next != NULL && !map_jstr_cat(map_jstr, ",")) {
                goto serialization_error;
            }
            item = item->next;
        }
        if (!map_jstr_cat(map_jstr, "\n        }")) {
            goto serialization_error;
        }
        if (entity->next != NULL && !map_jstr_cat(map_jstr, ",")) {
            goto serialization_error;
        }
        entity = entity->next;
    }

    // ENDING OF JSON STRING + ENTITIES
    if (!map_jstr_cat(map_jstr, "\n    ]\n}")) {
        goto serialization_error;
    }

    // WRITE JSON STRING TO FILE
    size_t len = SDL_strlen(map_jstr);
    for (size_t i = 0; i < len; i++)
        SDL_RWwrite(file, &map_jstr[i], sizeof(char), 1);
    SDL_RWclose(file);
    SDL_Log("Map saved to file: %zu bytes - %i tiles - %i ms", len, total_tiles,
            (int)(SDL_GetTicks64() - now));

    return 1;

serialization_error:
    SDL_Log("Map serialization error.");
    return 0;
}

int map_deserialize(struct map *map, const char *path)
{
    Uint32 now = SDL_GetTicks64();
    // DESERIALIZE JSON STRING
    // todo: json error handling
    struct json_value_s *json = json_from_file(path);
    if (json == NULL) {
        SDL_Log("Error parsing json map.");
        return 0;
    }

    struct json_object_s *json_root = (struct json_object_s *)json->payload;
    struct json_object_element_s *layer_object = json_root->start;
    struct json_array_s *layer_array = json_value_as_array(layer_object->value);

    int total_tiles = (int)layer_array->length;
    if (total_tiles < 0 || total_tiles > (TILES_MAX * TILES_MAX)) {
        SDL_Log("Error: Map size buffer overflow: %i tiles", total_tiles);
        return 0;
    }

    map_reset_tiles(map);

    struct json_array_element_s *tile_object = layer_array->start;
    for (int i = 0; i < total_tiles; i++) {
        struct json_array_s *tile_array =
            json_value_as_array(tile_object->value);

        struct json_array_element_s *x_object = tile_array->start;
        struct json_number_s *x_number = json_value_as_number(x_object->value);
        int x = SDL_strtol(x_number->number, NULL, 10);

        struct json_array_element_s *y_object = x_object->next;
        struct json_number_s *y_number = json_value_as_number(y_object->value);
        int y = SDL_strtol(y_number->number, NULL, 10);

        struct json_array_element_s *tileset_x_object = y_object->next;
        struct json_number_s *tileset_x_number =
            json_value_as_number(tileset_x_object->value);
        int tileset_x = SDL_strtol(tileset_x_number->number, NULL, 10);

        struct json_array_element_s *tileset_y_object = tileset_x_object->next;
        struct json_number_s *tileset_y_number =
            json_value_as_number(tileset_y_object->value);
        int tileset_y = SDL_strtol(tileset_y_number->number, NULL, 10);

        map->tiles[x][y].x = tileset_x;
        map->tiles[x][y].y = tileset_y;

        tile_object = tile_object->next;
    }

    struct json_array_s *entities_array =
        json_value_as_array(layer_object->next->value);
    map->entities.head =
        map_deserialize_entities(entities_array, &map->entities.tail);
    id = 0;
    struct map_entity *entity = map->entities.head;
    while (entity != NULL) {
        entity->id = map_make_id();
        entity = entity->next;
    }

    SDL_Log("Map deserialized with %i tiles in %i ms.", total_tiles,
            (int)(SDL_GetTicks64() - now));

    SDL_free(json);

    return 1;
}
