#ifndef MAP_H
#define MAP_H

struct map {
    int size;
    int tile_size;
};

struct map *map_create(void);

#endif
