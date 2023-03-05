#include <stdlib.h>

#include "map.h"

struct map *map_create(void)
{
    struct map *map = malloc(sizeof *map);
    if (map == NULL)
        return NULL;

    map->tile_size = 32;
    map->size = 20;

    return map;
}
