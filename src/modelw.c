#include "SDL.h"

#include "app.h"
#include "colors.h"
#include "map.h"
#include "modelw.h"

static SDL_FPoint offset;
static SDL_FPoint pan_start;
static float scale;

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

static void zoom(SDL_FPoint mouse_screen_coord, int mouse_wheel_y)
{
    SDL_FPoint mouse_model_before_zoom = {0, 0};
    screen_to_model(mouse_screen_coord, &mouse_model_before_zoom);

    if (mouse_wheel_y > 0) {
        scale *= 1.1f;

    } else if (mouse_wheel_y < 0) {
        scale *= 0.9f;
    }

    SDL_FPoint mouse_model_after_zoom = {0, 0};
    screen_to_model(mouse_screen_coord, &mouse_model_after_zoom);

    offset.x += (mouse_model_before_zoom.x - mouse_model_after_zoom.x);
    offset.y += (mouse_model_before_zoom.y - mouse_model_after_zoom.y);
}

static void pan(Uint32 evt_type, SDL_FPoint mouse_screen_coord)
{
    if (evt_type == SDL_MOUSEBUTTONDOWN) {
        pan_start.x = mouse_screen_coord.x;
        pan_start.y = mouse_screen_coord.y;

    } else if (evt_type == SDL_MOUSEMOTION) {
        offset.x -= (mouse_screen_coord.x - pan_start.x) / scale;
        offset.y -= (mouse_screen_coord.y - pan_start.y) / scale;
        pan_start.x = mouse_screen_coord.x;
        pan_start.y = mouse_screen_coord.y;
    }
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

static void pencil_tool(SDL_FPoint mouse_screen_coord, Uint8 button,
                        Uint8 state, struct app *app)
{
    if (button == SDL_BUTTON_LEFT) {
        SDL_FPoint mouse;
        screen_to_model(mouse_screen_coord, &mouse);

        int map_size_px = app->map->size * app->map->tile_size;
        SDL_FRect grid_rect = {0, 0, map_size_px, map_size_px};

        if (SDL_PointInFRect(&mouse, &grid_rect)) {
            int tile_x = mouse.x / app->map->tile_size;
            int tile_y = mouse.y / app->map->tile_size;

            app->map->tiles[tile_x][tile_y].x = app->tileset_selected.x;
            app->map->tiles[tile_x][tile_y].y = app->tileset_selected.y;
        }
        return;
    }

    if (button == SDL_BUTTON_RIGHT && state == SDL_PRESSED) {
        make_tile_shaped_tool_rect(&app->modelw.tool_rect, mouse_screen_coord,
                                   app);
        return;
    }

    if (button == SDL_BUTTON_RIGHT && state == SDL_RELEASED) {
        if (!SDL_FRectEmpty(&app->modelw.tool_rect.rect)) {
            SDL_FRect tool_rect_model_coord = app->modelw.tool_rect.rect;
            int i0 = tool_rect_model_coord.x / app->map->tile_size;
            int j0 = tool_rect_model_coord.y / app->map->tile_size;
            int i1 = (tool_rect_model_coord.x + tool_rect_model_coord.w) /
                     app->map->tile_size;
            int j1 = (tool_rect_model_coord.y + tool_rect_model_coord.h) /
                     app->map->tile_size;
            for (int i = i0; i < i1; i++) {
                for (int j = j0; j < j1; j++) {
                    app->map->tiles[i][j].x = app->tileset_selected.x;
                    app->map->tiles[i][j].y = app->tileset_selected.y;
                }
            }
        }
        reset_tool_rect(&app->modelw.tool_rect);
        return;
    }
}

static void eraser_tool(SDL_FPoint mouse_screen_coord, Uint8 button,
                        Uint8 state, struct app *app)
{
    if (button == SDL_BUTTON_LEFT) {
        SDL_FPoint mouse;
        screen_to_model(mouse_screen_coord, &mouse);

        int map_size_px = app->map->size * app->map->tile_size;
        SDL_FRect grid_rect = {0, 0, map_size_px, map_size_px};

        if (SDL_PointInFRect(&mouse, &grid_rect)) {
            int tile_x = mouse.x / app->map->tile_size;
            int tile_y = mouse.y / app->map->tile_size;

            app->map->tiles[tile_x][tile_y].x = -1;
            app->map->tiles[tile_x][tile_y].y = -1;
        }
        return;
    }

    if (button == SDL_BUTTON_RIGHT && state == SDL_PRESSED) {
        make_tile_shaped_tool_rect(&app->modelw.tool_rect, mouse_screen_coord,
                                   app);
        return;
    }

    if (button == SDL_BUTTON_RIGHT && state == SDL_RELEASED) {
        if (!SDL_FRectEmpty(&app->modelw.tool_rect.rect)) {
            SDL_FRect tool_rect_model_coord = app->modelw.tool_rect.rect;
            int i0 = tool_rect_model_coord.x / app->map->tile_size;
            int j0 = tool_rect_model_coord.y / app->map->tile_size;
            int i1 = (tool_rect_model_coord.x + tool_rect_model_coord.w) /
                     app->map->tile_size;
            int j1 = (tool_rect_model_coord.y + tool_rect_model_coord.h) /
                     app->map->tile_size;
            for (int i = i0; i < i1; i++) {
                for (int j = j0; j < j1; j++) {
                    app->map->tiles[i][j].x = -1;
                    app->map->tiles[i][j].y = -1;
                }
            }
        }
        reset_tool_rect(&app->modelw.tool_rect);
        return;
    }
}

static void tool_update(SDL_FPoint mouse_screen_coord, Uint8 button,
                        Uint8 state, struct app *app)
{
    switch (app->modelw.current_tool->type) {

    case PENCIL:
        pencil_tool(mouse_screen_coord, button, state, app);
        break;

    case ERASER:
        eraser_tool(mouse_screen_coord, button, state, app);
        break;

    default:
        break;
    }
}

static void evt_mouse_wheel(SDL_MouseWheelEvent *evt, struct app *app)
{
    SDL_FPoint mouse = {evt->mouseX, evt->mouseY};

    zoom(mouse, evt->y);
}

static void evt_mouse_down(SDL_MouseButtonEvent *evt, struct app *app)
{
    SDL_FPoint mouse = {evt->x, evt->y};

    if (evt->button == SDL_BUTTON_LEFT || evt->button == SDL_BUTTON_RIGHT) {
        tool_update(mouse, evt->button, evt->state, app);

    } else if (evt->button == SDL_BUTTON_MIDDLE) {
        if (evt->clicks == 2) {
            reset_pan_and_zoom(app);

        } else {
            pan(evt->type, mouse);
        }
    }
}

static void evt_mouse_up(SDL_MouseButtonEvent *evt, struct app *app)
{
    SDL_FPoint mouse = {evt->x, evt->y};

    if (evt->button == SDL_BUTTON_RIGHT) {
        tool_update(mouse, evt->button, evt->state, app);
    }
}

static void evt_mouse_motion(SDL_MouseMotionEvent *evt, struct app *app)
{
    SDL_FPoint mouse = {evt->x, evt->y};

    if (evt->state == SDL_BUTTON_LMASK || evt->state == SDL_BUTTON_RMASK) {
        Uint8 button = (evt->state >> 1) + 1; // inverse of SDL_BUTTON() macro
        tool_update(mouse, button, SDL_PRESSED, app);

    } else if (evt->state == SDL_BUTTON_MMASK) {
        pan(evt->type, mouse);
    }
}

void model_window_handle_event(SDL_Event *evt, struct app *app)
{
    switch (evt->type) {

    case SDL_MOUSEWHEEL:
        evt_mouse_wheel(&evt->wheel, app);
        break;

    case SDL_MOUSEBUTTONDOWN:
        evt_mouse_down(&evt->button, app);
        break;

    case SDL_MOUSEBUTTONUP:
        evt_mouse_up(&evt->button, app);
        break;

    case SDL_MOUSEMOTION:
        evt_mouse_motion(&evt->motion, app);
        break;
    }
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
    if (app->selected_entities != NULL) {
        entity = *app->selected_entities;
    }
    while (entity != NULL) {
        map_entity_get_rect(entity, &entity_rect);
        model_rect.x = entity_rect.x, model_rect.y = entity_rect.y,
        model_rect.w = entity_rect.w, model_rect.h = entity_rect.h;
        rect_model_to_screen(model_rect, &screen_rect);
        SDL_SetRenderDrawColor(renderer, RED.r, RED.g, RED.b, 70);
        SDL_RenderFillRectF(renderer, &screen_rect);
        entity = entity->next;
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
