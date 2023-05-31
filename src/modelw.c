#include "SDL.h"

#include "app.h"
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

void reset_tool_rect(struct tool_rect *tool_rect)
{
    tool_rect->rect.x = tool_rect->rect.y = -1;
    tool_rect->rect.w = tool_rect->rect.h = 0;
    tool_rect->start.x = tool_rect->start.y = -1;
}

static void pencil_tool(SDL_FPoint mouse_screen_coord, struct app *app)
{
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
}

static void pencil_tool_alt(SDL_FPoint mouse_screen_coord, Uint8 state,
                            struct app *app)
{
    if (state == SDL_PRESSED) {
        make_tool_rect(&app->modelw.tool_rect, mouse_screen_coord);

    } else if (state == SDL_RELEASED) {
        reset_tool_rect(&app->modelw.tool_rect);
    }
}

static void evt_mouse_wheel(SDL_MouseWheelEvent *evt, struct app *app)
{
    SDL_FPoint mouse = {evt->mouseX, evt->mouseY};

    SDL_FPoint mouse_model_before_zoom = {0, 0};
    screen_to_model(mouse, &mouse_model_before_zoom);

    if (evt->y > 0) {
        scale *= 1.1f;

    } else if (evt->y < 0) {
        scale *= 0.9f;
    }

    SDL_FPoint mouse_model_after_zoom = {0, 0};
    screen_to_model(mouse, &mouse_model_after_zoom);

    offset.x += (mouse_model_before_zoom.x - mouse_model_after_zoom.x);
    offset.y += (mouse_model_before_zoom.y - mouse_model_after_zoom.y);
}

static void evt_mouse_down(SDL_MouseButtonEvent *evt, struct app *app)
{
    SDL_FPoint mouse = {evt->x, evt->y};
    Uint8 state = evt->state;

    if (evt->button == SDL_BUTTON_LEFT) {
        pencil_tool(mouse, app);

    } else if (evt->button == SDL_BUTTON_MIDDLE) {
        if (evt->clicks == 2) {
            reset_pan_and_zoom(app);

        } else {
            pan_start.x = mouse.x;
            pan_start.y = mouse.y;
        }

    } else if (evt->button == SDL_BUTTON_RIGHT) {
        pencil_tool_alt(mouse, state, app);
    }
}

static void evt_mouse_up(SDL_MouseButtonEvent *evt, struct app *app)
{
    SDL_FPoint mouse = {evt->x, evt->y};
    Uint8 state = evt->state;

    if (evt->button == SDL_BUTTON_RIGHT) {
        pencil_tool_alt(mouse, state, app);
    }
}

static void evt_mouse_motion(SDL_MouseMotionEvent *evt, struct app *app)
{
    SDL_FPoint mouse = {evt->x, evt->y};
    Uint32 button = evt->state;

    if (button == SDL_BUTTON_LMASK) {
        pencil_tool(mouse, app);

    } else if (button == SDL_BUTTON_MMASK) {
        offset.x -= (mouse.x - pan_start.x) / scale;
        offset.y -= (mouse.y - pan_start.y) / scale;
        pan_start.x = mouse.x;
        pan_start.y = mouse.y;

    } else if (button == SDL_BUTTON_RMASK) {
        pencil_tool_alt(mouse, SDL_PRESSED, app);
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

    if (app->modelw.tool_rect.rect.w > 0 || app->modelw.tool_rect.rect.h > 0) {
        SDL_Color c = app->modelw.current_tool->rect_color;
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_RenderDrawRectF(renderer, &app->modelw.tool_rect.rect);
    }
}
