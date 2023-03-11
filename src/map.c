#include <stdio.h>
#include <stdlib.h>

#include "map.h"

struct map *map_create(void)
{
    struct map *map = malloc(sizeof *map);
    if (map == NULL)
        return NULL;

    map->tile_size = 32;
    map->size = 20;

    for (int i = 0; i < TILES_MAX; i++) {
        for (int j = 0; j < TILES_MAX; j++) {
            map->tiles[i][j].x = map->tiles[i][j].y = -1;
        }
    }

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

    char buffer[JSON_STRING_BUFSIZ + 1] = "";

    int mem = fread(buffer, 1, JSON_STRING_BUFSIZ, file);

    if (mem > JSON_STRING_BUFSIZ) {
        SDL_Log("Error: Json string buffer overflow.");
        return 0;
    }

    buffer[mem] = 0;

    SDL_Log("Map loaded: %ib - %ims", mem, (int)(SDL_GetTicks64() - now));
    SDL_Log("%s", buffer);

    return 1;
}
