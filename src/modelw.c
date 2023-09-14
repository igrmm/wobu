#include "SDL.h"

#include "app.h"
#include "colors.h"
#include "map.h"
#include "modelw.h"

static SDL_FPoint offset;
static SDL_FPoint pan_start;
static float scale;

static int entity_grabbed = 0;
static SDL_FPoint entity_grabbed_offset;

enum state {
    STATE_ZOOM,
    STATE_PAN,
    STATE_ZOOM_EXTENTS,
    STATE_ENTITY,
    STATE_SELECT,
    STATE_PAINT,
    NUM_STATES
};

static void state_zoom(SDL_Event *evt, struct app *app);
static void state_pan(SDL_Event *evt, struct app *app);
static void state_zoom_extents(SDL_Event *evt, struct app *app);
static void state_entity(SDL_Event *evt, struct app *app);
static void state_select(SDL_Event *evt, struct app *app);
static void state_paint(SDL_Event *evt, struct app *app);

static void (*state_table[NUM_STATES])(SDL_Event *evt, struct app *app) = {
    // clang-format off
    state_zoom,
    state_pan,
    state_zoom_extents,
    state_entity,
    state_select,
    state_paint
    // clang-format on
};

static void model_to_screen(SDL_FPoint model_coord, SDL_FPoint *screen_coord)
{
    screen_coord->x = (model_coord.x - offset.x) * scale;
    screen_coord->y = (model_coord.y - offset.y) * scale;
}

static void screen_to_model(SDL_FPoint screen_coord, SDL_FPoint *model_coord)
{
    model_coord->x = screen_coord.x / scale + offset.x;
    model_coord->y = screen_coord.y / scale + offset.y;
}

static void rect_model_to_screen(SDL_FRect rect_model_coord,
                                 SDL_FRect *rect_screen_coord)
{
    SDL_FPoint model_coord = {rect_model_coord.x, rect_model_coord.y};
    SDL_FPoint screen_coord = {0, 0};
    model_to_screen(model_coord, &screen_coord);
    rect_screen_coord->x = screen_coord.x;
    rect_screen_coord->y = screen_coord.y;
    rect_screen_coord->w = rect_model_coord.w * scale;
    rect_screen_coord->h = rect_model_coord.h * scale;
}

static void rect_screen_to_model(SDL_FRect rect_screen_coord,
                                 SDL_FRect *rect_model_coord)
{
    SDL_FPoint screen_coord = {rect_screen_coord.x, rect_screen_coord.y};
    SDL_FPoint model_coord = {0, 0};
    screen_to_model(screen_coord, &model_coord);
    rect_model_coord->x = model_coord.x;
    rect_model_coord->y = model_coord.y;
    rect_model_coord->w = rect_screen_coord.w / scale;
    rect_model_coord->h = rect_screen_coord.h / scale;
}

void reset_pan_and_zoom(struct app *app)
{
    int map_size = app->map->size * app->map->tile_size;
    offset.x = -(app->screen_width - map_size) * 0.5f;
    offset.y = -(app->screen_height - map_size) * 0.5f;
    scale = 1;
}

static void make_tool_rect(struct tool_rect *tool_rect,
                           SDL_FPoint mouse_screen_coord)
{
    // HORIZONTAL X AXIS
    if (tool_rect->start.x <= 0) {
        tool_rect->start.x = tool_rect->rect.x = mouse_screen_coord.x;

    } else if (mouse_screen_coord.x > tool_rect->start.x) {
        tool_rect->rect.x = tool_rect->start.x;
        tool_rect->rect.w = mouse_screen_coord.x - tool_rect->start.x;

    } else {
        tool_rect->rect.w = tool_rect->start.x - mouse_screen_coord.x;
        tool_rect->rect.x = mouse_screen_coord.x;
    }

    // VERTICAL Y AXIS
    if (tool_rect->start.y <= 0) {
        tool_rect->start.y = tool_rect->rect.y = mouse_screen_coord.y;

    } else if (mouse_screen_coord.y > tool_rect->start.y) {
        tool_rect->rect.y = tool_rect->start.y;
        tool_rect->rect.h = mouse_screen_coord.y - tool_rect->start.y;

    } else {
        tool_rect->rect.h = tool_rect->start.y - mouse_screen_coord.y;
        tool_rect->rect.y = mouse_screen_coord.y;
    }
}

static void make_tile_shaped_tool_rect(struct tool_rect *tool_rect,
                                       SDL_FPoint mouse_screen_coord,
                                       struct app *app)
{
    make_tool_rect(tool_rect, mouse_screen_coord);

    // make tool_rect in model coords
    SDL_FRect tool_rect_model_coord = {0, 0, 0, 0};
    rect_screen_to_model(tool_rect->rect, &tool_rect_model_coord);

    // make grid_rect in model coords
    int map_size_px = app->map->size * app->map->tile_size;
    SDL_FRect grid_rect_model_coord = {0, 0, map_size_px, map_size_px};

    SDL_FRect intersect = {0, 0, 0, 0};
    if (SDL_IntersectFRect(&grid_rect_model_coord, &tool_rect_model_coord,
                           &intersect)) {
        int tile_size = app->map->tile_size;

        // floor intersect origin to tile index (i*tile) and save the difference
        int x = (int)(intersect.x / tile_size) * tile_size;
        int diff_x = intersect.x - x;
        intersect.x = x;
        int y = (int)(intersect.y / tile_size) * tile_size;
        int diff_y = intersect.y - y;
        intersect.y = y;

        // ceil intersect sz to tile index (j*tile) n clamp if bigger than mapsz
        intersect.w = (int)((intersect.w + diff_x) / tile_size + 1) * tile_size;
        if ((intersect.x + intersect.w) > grid_rect_model_coord.w)
            intersect.w = grid_rect_model_coord.w - intersect.x;
        intersect.h = (int)((intersect.h + diff_y) / tile_size + 1) * tile_size;
        if ((intersect.y + intersect.h) > grid_rect_model_coord.h)
            intersect.h = grid_rect_model_coord.h - intersect.y;

        tool_rect->rect = intersect;

    } else {
        tool_rect->rect.x = tool_rect->rect.y = -1;
        tool_rect->rect.w = tool_rect->rect.h = 0;
    }
}

void reset_tool_rect(struct tool_rect *tool_rect)
{
    tool_rect->rect.x = tool_rect->rect.y = -1;
    tool_rect->rect.w = tool_rect->rect.h = 0;
    tool_rect->start.x = tool_rect->start.y = -1;
}

static void paint_tile_on_mouse(SDL_FPoint mouse_screen_coord, struct map *map,
                                int tileset_x, int tileset_y)
{
    SDL_FPoint mouse;
    screen_to_model(mouse_screen_coord, &mouse);

    int map_size_px = map->size * map->tile_size;
    SDL_FRect grid_rect = {0, 0, map_size_px, map_size_px};

    if (SDL_PointInFRect(&mouse, &grid_rect)) {
        int tile_x = mouse.x / map->tile_size;
        int tile_y = mouse.y / map->tile_size;

        map->tiles[tile_x][tile_y].x = tileset_x;
        map->tiles[tile_x][tile_y].y = tileset_y;
    }
}

static void paint_tiles_in_rect(struct map *map, SDL_FRect rect, int tileset_x,
                                int tileset_y)
{
    if (!SDL_FRectEmpty(&rect)) {
        SDL_FRect tool_rect_model_coord = rect;
        int i0 = tool_rect_model_coord.x / map->tile_size;
        int j0 = tool_rect_model_coord.y / map->tile_size;
        int i1 = (tool_rect_model_coord.x + tool_rect_model_coord.w) /
                 map->tile_size;
        int j1 = (tool_rect_model_coord.y + tool_rect_model_coord.h) /
                 map->tile_size;
        for (int i = i0; i < i1; i++) {
            for (int j = j0; j < j1; j++) {
                map->tiles[i][j].x = tileset_x;
                map->tiles[i][j].y = tileset_y;
            }
        }
    }
}

static void state_zoom(SDL_Event *evt, struct app *app)
{
    SDL_FPoint mouse_screen_coord = {evt->wheel.mouseX, evt->wheel.mouseY};
    SDL_FPoint mouse_model_before_zoom = {0, 0};

    screen_to_model(mouse_screen_coord, &mouse_model_before_zoom);

    if (evt->wheel.y > 0) {
        scale *= 1.1f;

    } else if (evt->wheel.y < 0) {
        scale *= 0.9f;
    }

    SDL_FPoint mouse_model_after_zoom = {0, 0};
    screen_to_model(mouse_screen_coord, &mouse_model_after_zoom);

    offset.x += (mouse_model_before_zoom.x - mouse_model_after_zoom.x);
    offset.y += (mouse_model_before_zoom.y - mouse_model_after_zoom.y);
}

static void state_pan(SDL_Event *evt, struct app *app)
{
    if (evt->type == SDL_MOUSEBUTTONDOWN) {
        pan_start = (SDL_FPoint){evt->button.x, evt->button.y};

    } else if (evt->type == SDL_MOUSEMOTION) {
        offset.x -= (evt->motion.x - pan_start.x) / scale;
        offset.y -= (evt->motion.y - pan_start.y) / scale;
        pan_start.x = evt->motion.x;
        pan_start.y = evt->motion.y;
    }
}

static void state_zoom_extents(SDL_Event *evt, struct app *app)
{
    reset_pan_and_zoom(app);
}

static void state_entity(SDL_Event *evt, struct app *app)
{
    if (evt->type == SDL_MOUSEMOTION && evt->motion.state == SDL_BUTTON_RMASK) {
        make_tile_shaped_tool_rect(&app->modelw.tool_rect,
                                   (SDL_FPoint){evt->motion.x, evt->motion.y},
                                   app);
        return;
    }

    if (evt->type == SDL_MOUSEBUTTONUP &&
        evt->button.button == SDL_BUTTON_RIGHT) {
        if (!SDL_FRectEmpty(&app->modelw.tool_rect.rect)) {
            SDL_Rect entity_rect = {
                app->modelw.tool_rect.rect.x, app->modelw.tool_rect.rect.y,
                app->modelw.tool_rect.rect.w, app->modelw.tool_rect.rect.h};
            struct map_entity *entity =
                map_create_entity(app->template.entities[0]);
            map_entity_set_rect(entity, &entity_rect);
            map_entities_add(entity, &app->map->entities);
            app->selection.entities[0] = entity;
            app->selection.count = 1;
        }
        reset_tool_rect(&app->modelw.tool_rect);
        return;
    }
}

static void state_select(SDL_Event *evt, struct app *app)
{
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    // GRAB/MOVE SINGLE ENTITY
    if (evt->type == SDL_MOUSEBUTTONDOWN &&
        evt->button.button == SDL_BUTTON_LEFT && keys[SDL_SCANCODE_LCTRL]) {
        if (entity_grabbed == 0 && app->selection.count == 1) {
            SDL_FRect entity_rect = {0};
            map_entity_get_frect(app->selection.entities[0], &entity_rect);
            SDL_FPoint mouse;
            screen_to_model((SDL_FPoint){evt->button.x, evt->button.y}, &mouse);
            if (SDL_PointInFRect(&mouse, &entity_rect)) {
                entity_grabbed = 1;
                int tile_size = app->map->tile_size;
                entity_grabbed_offset.x =
                    (int)((mouse.x - entity_rect.x) / tile_size) * tile_size;
                entity_grabbed_offset.y =
                    (int)((mouse.y - entity_rect.y) / tile_size) * tile_size;
            }
        }
        return;
    }

    if (evt->type == SDL_MOUSEMOTION && evt->motion.state == SDL_BUTTON_LMASK &&
        keys[SDL_SCANCODE_LCTRL]) {
        if (entity_grabbed == 1 && app->selection.count == 1) {
            SDL_FPoint mouse;
            screen_to_model((SDL_FPoint){evt->button.x, evt->button.y}, &mouse);
            int map_size_px = app->map->size * app->map->tile_size;
            SDL_FRect grid_rect = {0, 0, map_size_px, map_size_px};

            if (SDL_PointInFRect(&mouse, &grid_rect)) {
                int tile_size = app->map->tile_size;

                // round mouse to "tiles"
                mouse.x = (int)(mouse.x / tile_size) * tile_size;
                mouse.y = (int)(mouse.y / tile_size) * tile_size;

                SDL_Rect entity_rect = {0};
                map_entity_get_rect(app->selection.entities[0], &entity_rect);
                entity_rect.x = mouse.x - entity_grabbed_offset.x;
                entity_rect.y = mouse.y - entity_grabbed_offset.y;
                map_entity_set_rect(app->selection.entities[0], &entity_rect);
            }
        }
        return;
    }

    if (evt->type == SDL_MOUSEBUTTONUP &&
        evt->button.button == SDL_BUTTON_LEFT) {
        SDL_Rect intersect = {0};
        SDL_Rect entity_rect = {0};
        map_entity_get_rect(app->selection.entities[0], &entity_rect);
        int map_size_px = app->map->size * app->map->tile_size;
        SDL_Rect grid_rect = {0, 0, map_size_px, map_size_px};
        if (SDL_IntersectRect(&entity_rect, &grid_rect, &intersect)) {
            map_entity_set_rect(app->selection.entities[0], &intersect);
        }
        entity_grabbed = 0;
        return;
    }

    // SELECT ENTITIES
    if (evt->type == SDL_MOUSEBUTTONDOWN &&
        evt->button.button == SDL_BUTTON_LEFT && !keys[SDL_SCANCODE_LCTRL]) {
        SDL_FPoint mouse;
        screen_to_model((SDL_FPoint){evt->button.x, evt->button.y}, &mouse);

        int map_size_px = app->map->size * app->map->tile_size;
        SDL_FRect grid_rect = {0, 0, map_size_px, map_size_px};
        SDL_FRect entity_rect = {0};

        if (SDL_PointInFRect(&mouse, &grid_rect)) {
            struct map_entity *entity = app->map->entities.head;
            while (entity != NULL) {
                map_entity_get_frect(entity, &entity_rect);
                if (SDL_PointInFRect(&mouse, &entity_rect)) {
                    app->selection.entities[0] = entity;
                    app->selection.count = 1;
                    return;
                }
                entity = entity->next;
            }
        }
        return;
    }

    if (evt->type == SDL_MOUSEMOTION && evt->motion.state == SDL_BUTTON_RMASK &&
        !keys[SDL_SCANCODE_LCTRL]) {
        make_tile_shaped_tool_rect(&app->modelw.tool_rect,
                                   (SDL_FPoint){evt->motion.x, evt->motion.y},
                                   app);
        return;
    }

    if (evt->type == SDL_MOUSEBUTTONUP &&
        evt->button.button == SDL_BUTTON_RIGHT) {
        if (!SDL_FRectEmpty(&app->modelw.tool_rect.rect)) {
            struct map_entity *entity = app->map->entities.head;
            SDL_FRect entity_rect = {0};
            size_t i = 0;
            while (entity != NULL) {
                map_entity_get_frect(entity, &entity_rect);
                if (SDL_HasIntersectionF(&entity_rect,
                                         &app->modelw.tool_rect.rect)) {
                    app->selection.entities[i] = entity;
                    app->selection.count = ++i;
                    if (i >= SDL_arraysize(app->selection.entities)) {
                        SDL_Log("Warning: entity selection overflow.");
                    }
                }
                entity = entity->next;
            }
        }
        reset_tool_rect(&app->modelw.tool_rect);
        return;
    }

    // REMOVE SELECTION
    if (evt->type == SDL_KEYUP &&
        evt->key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
        app->selection.count = 0;
        return;
    }

    // DELETE SELECTED ENTITIES
    if (evt->type == SDL_KEYUP &&
        evt->key.keysym.scancode == SDL_SCANCODE_DELETE) {
        for (int i = 0; i < app->selection.count; i++) {
            if (app->selection.entities[i] != NULL) {
                map_entities_remove(app->selection.entities[i],
                                    &app->map->entities);
            }
        }
        app->selection.entities[0] = NULL;
        app->selection.count = 0;
        return;
    }
}

static void state_paint(SDL_Event *evt, struct app *app)
{
    int tileset_x = -1, tileset_y = -1; // default to eraser
    if (app->modelw.current_tool->type == PENCIL) {
        tileset_x = app->tileset_selected.x;
        tileset_y = app->tileset_selected.y;
    }

    if (evt->type == SDL_MOUSEBUTTONDOWN &&
        evt->button.button == SDL_BUTTON_LEFT) {
        paint_tile_on_mouse((SDL_FPoint){evt->button.x, evt->button.y},
                            app->map, tileset_x, tileset_y);
        return;
    }

    if (evt->type == SDL_MOUSEMOTION && evt->motion.state == SDL_BUTTON_LMASK) {
        paint_tile_on_mouse((SDL_FPoint){evt->motion.x, evt->motion.y},
                            app->map, tileset_x, tileset_y);
        return;
    }

    if (evt->type == SDL_MOUSEMOTION && evt->motion.state == SDL_BUTTON_RMASK) {
        make_tile_shaped_tool_rect(&app->modelw.tool_rect,
                                   (SDL_FPoint){evt->motion.x, evt->motion.y},
                                   app);
        return;
    }

    if (evt->type == SDL_MOUSEBUTTONUP &&
        evt->button.button == SDL_BUTTON_RIGHT) {
        paint_tiles_in_rect(app->map, app->modelw.tool_rect.rect, tileset_x,
                            tileset_y);
        reset_tool_rect(&app->modelw.tool_rect);
        return;
    }
}

static enum state handle_event(SDL_Event *evt, enum tool_type current_tool)
{
    if (evt->type == SDL_MOUSEWHEEL) {
        return STATE_ZOOM;
    }

    int mouse_middle_btn_1_click =
        (evt->type == SDL_MOUSEBUTTONDOWN &&
         evt->button.button == SDL_BUTTON_MIDDLE && evt->button.clicks == 1);
    int mouse_drag_middle_btn =
        (evt->type == SDL_MOUSEMOTION && evt->motion.state == SDL_BUTTON_MMASK);
    if (mouse_middle_btn_1_click || mouse_drag_middle_btn) {
        return STATE_PAN;
    }

    int mouse_middle_btn_2_click =
        (evt->type == SDL_MOUSEBUTTONDOWN &&
         evt->button.button == SDL_BUTTON_MIDDLE && evt->button.clicks == 2);
    if (mouse_middle_btn_2_click) {
        return STATE_ZOOM_EXTENTS;
    }

    if (current_tool == ENTITY) {
        return STATE_ENTITY;
    }

    if (current_tool == SELECT) {
        return STATE_SELECT;
    }

    return STATE_PAINT;
}

static void state_run(enum state state, SDL_Event *evt, struct app *app)
{
    (*state_table[state])(evt, app);
}

void model_window_handle_event(SDL_Event *evt, struct app *app)
{
    enum state state = handle_event(evt, app->modelw.current_tool->type);
    state_run(state, evt, app);
}

// TODO OPTMIZATION, CLIPPING
void model_window_render(SDL_Renderer *renderer, struct app *app)
{
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);

    // render tiles
    int tile_size = app->map->tile_size;
    int map_size = app->map->size;
    SDL_Rect src_rect = {-1, -1, tile_size, tile_size};
    SDL_FRect dst_rect = {0, 0, tile_size, tile_size};
    SDL_FPoint model_coord = {0, 0};
    SDL_FPoint screen_coord = {0, 0};

    for (int i = 0; i < TILES_MAX; i++) {
        for (int j = 0; j < TILES_MAX; j++) {
            src_rect.x = app->map->tiles[i][j].x;
            src_rect.y = app->map->tiles[i][j].y;
            if (src_rect.x >= 0 && src_rect.y >= 0) {

                // make coordinate convertion
                model_coord.x = i * tile_size;
                model_coord.y = j * tile_size;
                model_to_screen(model_coord, &screen_coord);
                dst_rect.x = screen_coord.x;
                dst_rect.y = screen_coord.y;
                dst_rect.w = dst_rect.h = tile_size * scale;

                SDL_RenderCopyF(renderer, app->tileset_texture, &src_rect,
                                &dst_rect);
            }
        }
    }

    // render grid
    if (app->show_grid) {
        int cols = map_size;
        SDL_FPoint col0_model, col0_screen, col1_model, col1_screen;
        for (int col = 0; col <= cols; col++) {
            col0_model.x = col * tile_size;
            col0_model.y = 0;
            col1_model.x = col * tile_size;
            col1_model.y = map_size * tile_size;

            model_to_screen(col0_model, &col0_screen);
            model_to_screen(col1_model, &col1_screen);

            SDL_RenderDrawLineF(renderer, col0_screen.x, col0_screen.y,
                                col1_screen.x, col1_screen.y);
        }

        int rows = map_size;
        SDL_FPoint row0_model, row0_screen, row1_model, row1_screen;
        for (int row = 0; row <= rows; row++) {
            row0_model.x = 0;
            row0_model.y = row * tile_size;
            row1_model.x = map_size * tile_size;
            row1_model.y = row * tile_size;

            model_to_screen(row0_model, &row0_screen);
            model_to_screen(row1_model, &row1_screen);

            SDL_RenderDrawLineF(renderer, row0_screen.x, row0_screen.y,
                                row1_screen.x, row1_screen.y);
        }
    }

    struct map_entity *entity = NULL;
    SDL_FRect screen_rect = {0}, model_rect = {0};
    SDL_Rect entity_rect = {0};

    // render entities
    entity = app->map->entities.head;
    while (entity != NULL) {
        map_entity_get_rect(entity, &entity_rect);
        model_rect.x = entity_rect.x, model_rect.y = entity_rect.y,
        model_rect.w = entity_rect.w, model_rect.h = entity_rect.h;
        rect_model_to_screen(model_rect, &screen_rect);
        SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
        SDL_RenderDrawRectF(renderer, &screen_rect);
        entity = entity->next;
    }

    // render selected entities
    for (int i = 0; i < app->selection.count; i++) {
        if (app->selection.entities[i] != NULL) {
            entity = app->selection.entities[i];
            map_entity_get_rect(entity, &entity_rect);
            model_rect.x = entity_rect.x, model_rect.y = entity_rect.y,
            model_rect.w = entity_rect.w, model_rect.h = entity_rect.h;
            rect_model_to_screen(model_rect, &screen_rect);
            SDL_SetRenderDrawColor(renderer, RED.r, RED.g, RED.b, 70);
            SDL_RenderFillRectF(renderer, &screen_rect);
        }
    }

    // render tool rect
    if (app->modelw.tool_rect.rect.w > 0 || app->modelw.tool_rect.rect.h > 0) {
        SDL_Color c = app->modelw.current_tool->rect_color;
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_FRect tool_rect_screen_coord = {0, 0, 0, 0};
        rect_model_to_screen(app->modelw.tool_rect.rect,
                             &tool_rect_screen_coord);
        SDL_RenderDrawRectF(renderer, &tool_rect_screen_coord);
    }
}
