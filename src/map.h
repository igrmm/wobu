#ifndef MAP_H
#define MAP_H

#include <SDL2/SDL.h>

#define TILES_MAX 100
#define JSON_STRING_BUFSIZ 1000000

struct map {
    int size;
    int tile_size;
    SDL_Point tiles[TILES_MAX][TILES_MAX];
};

struct map *map_create(void);
int map_serialize(struct map *map, const char *path);
int map_deserialize(struct map *map, const char *path);

#endif
