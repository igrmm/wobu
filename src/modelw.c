#include "SDL.h"

#include "app.h"
#include "modelw.h"

static void model_to_screen(SDL_FPoint model_coord, SDL_FPoint *screen_coord,
                            SDL_FPoint offset, float scale)
{
    screen_coord->x = (model_coord.x - offset.x) * scale;
    screen_coord->y = (model_coord.y - offset.y) * scale;
}

static void screen_to_model(SDL_FPoint screen_coord, SDL_FPoint *model_coord,
                            SDL_FPoint offset, float scale)
{
    model_coord->x = screen_coord.x / scale + offset.x;
    model_coord->y = screen_coord.y / scale + offset.y;
}

void model_window_handle_event(SDL_Event *evt, struct app *app)
{
    struct modelw *modelw = &app->modelw;

    if (evt->type == SDL_MOUSEWHEEL) {
        SDL_FPoint mouse = {evt->wheel.mouseX, evt->wheel.mouseY};

        SDL_FPoint mouse_model_before_zoom = {0, 0};
        screen_to_model(mouse, &mouse_model_before_zoom, modelw->offset,
                        modelw->scale);

        if (evt->wheel.y > 0) {
            modelw->scale *= 1.1f;

        } else if (evt->wheel.y < 0) {
            modelw->scale *= 0.9f;
        }

        SDL_FPoint mouse_model_after_zoom = {0, 0};
        screen_to_model(mouse, &mouse_model_after_zoom, modelw->offset,
                        modelw->scale);

        modelw->offset.x +=
            (mouse_model_before_zoom.x - mouse_model_after_zoom.x);
        modelw->offset.y +=
            (mouse_model_before_zoom.y - mouse_model_after_zoom.y);
    }

    if (evt->button.button == SDL_BUTTON_MIDDLE) {
        SDL_FPoint mouse = {evt->button.x, evt->button.y};

        if (evt->type == SDL_MOUSEBUTTONDOWN) {
            modelw->pan_start.x = mouse.x;
            modelw->pan_start.y = mouse.y;

        } else if (evt->type == SDL_MOUSEMOTION) {
            modelw->offset.x -=
                (evt->button.x - modelw->pan_start.x) / app->modelw.scale;
            modelw->offset.y -=
                (evt->button.y - modelw->pan_start.y) / app->modelw.scale;
            modelw->pan_start.x = mouse.x;
            modelw->pan_start.y = mouse.y;
        }
    }

    if (evt->button.button == SDL_BUTTON_LEFT &&
        (evt->type == SDL_MOUSEBUTTONDOWN || evt->type == SDL_MOUSEMOTION)) {
        int map_size_px = app->map->size * app->map->tile_size;
        SDL_Rect grid_rect = {app->bg_scroll.x, app->bg_scroll.y, map_size_px,
                              map_size_px};
        SDL_Point mouse_pos = {evt->button.x, evt->button.y};

        if (SDL_PointInRect(&mouse_pos, &grid_rect)) {
            int tile_x = (mouse_pos.x - grid_rect.x) / app->map->tile_size;
            int tile_y = (mouse_pos.y - grid_rect.y) / app->map->tile_size;

            app->map->tiles[tile_x][tile_y].x = app->tileset_selected.x;
            app->map->tiles[tile_x][tile_y].y = app->tileset_selected.y;
        }
    }
}

// TODO OPTMIZATION, CLIPPING
void model_window_render(SDL_Renderer *renderer, struct app *app)
{
    struct modelw *modelw = &app->modelw;

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
                model_to_screen(model_coord, &screen_coord, app->modelw.offset,
                                app->modelw.scale);
                dst_rect.x = screen_coord.x;
                dst_rect.y = screen_coord.y;
                dst_rect.w = dst_rect.h = tile_size * app->modelw.scale;

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

            model_to_screen(col0_model, &col0_screen, modelw->offset,
                            modelw->scale);
            model_to_screen(col1_model, &col1_screen, modelw->offset,
                            modelw->scale);

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

            model_to_screen(row0_model, &row0_screen, modelw->offset,
                            modelw->scale);
            model_to_screen(row1_model, &row1_screen, modelw->offset,
                            modelw->scale);

            SDL_RenderDrawLineF(renderer, row0_screen.x, row0_screen.y,
                                row1_screen.x, row1_screen.y);
        }
    }
}
