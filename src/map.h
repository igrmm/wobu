#ifndef MAP_H
#define MAP_H

#include "../external/json.h/json.h"
#include "SDL.h" // IWYU pragma: keep //clangd

#define TILES_MAX 100
#define ENTITY_STR_BUFSIZ 24
#define ENTITY_GROUP_MAX 100

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
    int id;
    struct map_entity_item *item;
    struct map_entity *prev, *next;
};

struct map_entity_group {
    struct map_entity *entities[ENTITY_GROUP_MAX];
    const char *ids[ENTITY_GROUP_MAX];
    int count;
};

struct map_entities {
    struct map_entity *head, *tail;
};

struct map {
    int size;
    int tile_size;
    SDL_Point tiles[TILES_MAX][TILES_MAX];
    struct map_entities entities;
};

int map_make_id(void);
struct map *map_create(void);
struct map_entity_item *
map_create_entity_items(struct map_entity *entity_template);
struct map_entity *map_create_entity(struct map_entity *entity_template);
int map_entity_get_rect(struct map_entity *entity, SDL_Rect *rect);
int map_entity_get_frect(struct map_entity *entity, SDL_FRect *frect);
int map_entity_set_rect(struct map_entity *entity, SDL_Rect *rect);
void map_print_entity(struct map_entity *entity);
void map_reset_tiles(struct map *map);
size_t map_destroy_entity_items(struct map_entity *entity);
size_t map_destroy_entity(struct map_entity *entity);
void map_destroy_entities(struct map_entity *entities);
void map_entities_add(struct map_entity *entity, struct map_entities *entities);
void map_entities_remove(struct map_entity *entity,
                         struct map_entities *entities);
struct map_entity *map_deserialize_entities(struct json_array_s *entities_array,
                                            struct map_entity **tail);
int map_serialize(struct map *map, const char *path);
int map_deserialize(struct map *map, const char *path);

#endif
