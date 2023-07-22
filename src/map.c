#include "SDL.h"
#include "external/json.h"

#include "jsonffile.h"
#include "map.h"

void map_reset_tiles(struct map *map)
{
    for (int i = 0; i < TILES_MAX; i++) {
        for (int j = 0; j < TILES_MAX; j++) {
            map->tiles[i][j].x = map->tiles[i][j].y = -1;
        }
    }
}

struct map *map_create(void)
{
    struct map *map = malloc(sizeof *map);
    if (map == NULL)
        return NULL;

    map->tile_size = 32;
    map->size = 20;

    map_reset_tiles(map);

    return map;
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

void map_destroy_entities(struct map_entity *entities)
{
    size_t mem = 0;
    // LOOP THROUGH ENTITIES
    struct map_entity *entity = entities;
    while (entity != NULL) {

        // LOOP THROUGH ITEMS OF CURRENT ENTITY AND DESTROY
        struct map_entity_item *item = entity->item;
        while (item != NULL) {
            struct map_entity_item *next_item = item->next;
            mem += sizeof(*item);
            SDL_free(item);
            item = next_item;
        }

        // DESTROY CURRENT ENTITY
        struct map_entity *next_entity = entity->next;
        mem += sizeof(*entity);
        SDL_free(entity);
        entity = next_entity;
    }
    SDL_Log("Entities destroyed, memory freed: %zu bytes", mem);
}

struct map_entity *map_deserialize_entities(struct json_value_s *json)
{
    size_t mem = 0;
    Uint32 now = SDL_GetTicks64();
    // DESERIALIZE JSON STRING
    // todo: json error handling
    struct json_object_s *json_root = (struct json_object_s *)json->payload;
    struct json_object_element_s *entities_object = json_root->start;
    struct json_array_s *entities_array =
        json_value_as_array(entities_object->value);

    struct map_entity *entities = NULL;
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

        entity = &(*entity)->next;

        number_of_entities++;
        if (number_of_entities >= 10) { // max entities = 10
            SDL_Log("Error: max number of entities reached.");
            return NULL;
        }
        ent_array_obj = ent_array_obj->next;
    }

    SDL_Log("Entities deserialized with %i entities in %i ms with %zu bytes.",
            number_of_entities, (int)(SDL_GetTicks64() - now), mem);

    SDL_free(json);

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

    // BEGINNING OF JSON STRING
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

    // ENDING OF JSON STRING
    if (!map_jstr_cat(map_jstr, "]\n}")) {
        goto serialization_error;
    }

    // WRITE JSON STRING TO FILE
    size_t len = SDL_strlen(map_jstr);
    for (size_t i = 0; i <= len; i++)
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
        int x = strtol(x_number->number, NULL, 10);

        struct json_array_element_s *y_object = x_object->next;
        struct json_number_s *y_number = json_value_as_number(y_object->value);
        int y = strtol(y_number->number, NULL, 10);

        struct json_array_element_s *tileset_x_object = y_object->next;
        struct json_number_s *tileset_x_number =
            json_value_as_number(tileset_x_object->value);
        int tileset_x = strtol(tileset_x_number->number, NULL, 10);

        struct json_array_element_s *tileset_y_object = tileset_x_object->next;
        struct json_number_s *tileset_y_number =
            json_value_as_number(tileset_y_object->value);
        int tileset_y = strtol(tileset_y_number->number, NULL, 10);

        map->tiles[x][y].x = tileset_x;
        map->tiles[x][y].y = tileset_y;

        tile_object = tile_object->next;
    }

    SDL_Log("Map deserialized with %i tiles in %i ms.", total_tiles,
            (int)(SDL_GetTicks64() - now));

    free(json);

    return 1;
}
