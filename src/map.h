#ifndef MAP_H
#define MAP_H

#include "SDL.h"
#include "external/json.h"

#define TILES_MAX 100
#define ENTITY_STR_BUFSIZ 24

enum map_entity_type { NUMBER, STRING, BOOL };

union map_entity_value {
    int number;
    char string[ENTITY_STR_BUFSIZ];
    int bool;
};

struct map_entity_item {
    char name[ENTITY_STR_BUFSIZ];
    enum map_entity_type type;
    union map_entity_value value;
    struct map_entity_item *next;
};

struct map_entity {
    struct map_entity_item *item;
    struct map_entity *next;
};

struct map {
    int size;
    int tile_size;
    SDL_Point tiles[TILES_MAX][TILES_MAX];
};

struct map *map_create(void);
struct map_entity *map_create_entity(struct map_entity *entity_template);
void map_print_entity(struct map_entity *entity);
void map_reset_tiles(struct map *map);
void map_destroy_entities(struct map_entity *entities);
struct map_entity *map_deserialize_entities(struct json_value_s *json);
int map_serialize(struct map *map, const char *path);
int map_deserialize(struct map *map, const char *path);

#endif
