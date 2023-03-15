#include <stdio.h>
#include <stdlib.h>

#include "external/json.h"

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

int map_serialize(struct map *map, const char *path)
{
    Uint32 now = SDL_GetTicks64();

    FILE *file = fopen(path, "w");

    if (file == NULL) {
        SDL_Log("Error opening file to write map.");
        return 0;
    }

    int mem = 0;

    mem += fprintf(file, "{\n    \"tiles\": [");

    int tile_x, tile_y, first_tile = 0;
    for (int i = 0; i < map->size; i++) {
        for (int j = 0; j < map->size; j++) {
            tile_x = map->tiles[i][j].x;
            tile_y = map->tiles[i][j].y;
            if (tile_x >= 0 && tile_y >= 0) {
                if (!first_tile) {
                    first_tile = 1;
                    mem +=
                        fprintf(file, "[%i, %i, %i, %i]", i, j, tile_x, tile_y);
                } else {
                    mem += fprintf(file, ",[%i, %i, %i, %i]", i, j, tile_x,
                                   tile_y);
                }
            }
        }
    }
    mem += fprintf(file, "]\n}");
    fclose(file);

    SDL_Log("Map saved: %ib - %ims", mem, (int)(SDL_GetTicks64() - now));

    return 1;
}

int map_deserialize(struct map *map, const char *path)
{

    Uint32 now = SDL_GetTicks64();

    FILE *file = fopen(path, "r");

    if (file == NULL) {
        SDL_Log("Error opening file to load map.");
        return 0;
    }

    SDL_Log("Map deserialization started.");

    char buffer[JSON_STRING_BUFSIZ + 1] = "";

    int mem = fread(buffer, 1, JSON_STRING_BUFSIZ, file);

    if (mem > JSON_STRING_BUFSIZ) {
        SDL_Log("Error: Json string buffer overflow.");
        return 0;
    }

    buffer[mem] = 0;

    SDL_Log("Map loaded from file into json string: %i bytes.", mem);

    // todo: json error handling

    struct json_value_s *json = json_parse(buffer, mem);
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
