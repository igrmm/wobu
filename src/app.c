#include "SDL.h"
#include "SDL_image.h"

#include "app.h"
#include "calc.h"
#include "colors.h"
#include "map.h"
#include "menuw.h"
#include "modelw.h"
#include "statusw.h"
#include "tilesetw.h"
#include "toolsw.h"

static const int WINDOW_FLAGS = NK_WINDOW_BORDER | NK_WINDOW_SCALABLE |
                                NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE |
                                NK_WINDOW_CLOSABLE;

static struct tool tool_init(enum tool_type tool_type, SDL_Texture *texture,
                             int r, int g, int b)
{
    SDL_Color rect_color = {r, g, b, 255};
    struct tool tool;
    tool.type = tool_type;
    tool.rect_color = rect_color;
    tool.texture = texture;
    return tool;
}

int app_init(struct app *app, SDL_Renderer *renderer)
{
    SDL_Texture *tileset_texture = IMG_LoadTexture(renderer, "tileset.png");
    if (tileset_texture == NULL) {
        SDL_Log("SDL error in tileset_texture creation: %s", SDL_GetError());
        return 0;
    }

    SDL_Texture *pencil_texture =
        IMG_LoadTexture(renderer, "../assets/pencil.png");
    if (pencil_texture == NULL) {
        SDL_Log("SDL error in pencil_texture creation: %s", SDL_GetError());
        return 0;
    }
    app->modelw.tools[PENCIL] = tool_init(PENCIL, pencil_texture, 0, 255, 0);

    reset_tool_rect(&app->modelw.tool_rect);

    app->map = map_create();
    if (app->map == NULL) {
        SDL_Log("Map error: unable to alloc memory.");
        app_deinit(app);
        return 0;
    }
    SDL_Log("Map allocated memory: %i kB", (int)sizeof *app->map / 1024);

    // centralize bg
    SDL_GetRendererOutputSize(renderer, &app->screen_width,
                              &app->screen_height);
    reset_pan_and_zoom(app);

    app->modelw.current_tool = &app->modelw.tools[PENCIL];
    app->tileset_texture = tileset_texture;
    app->tileset_selected = nk_vec2(-1, -1);
    app->fps_counter.frames = 0;
    app->fps_counter.timer = SDL_GetTicks64();
    app->show_grid = 1;
    app->show_toolsw = 1;
    app->show_tilesetw = 1;

    return 1;
}

void app_handle_event(struct app *app, SDL_Event *evt)
{
    model_window_handle_event(evt, app);
}

int app_run(struct app *app, struct nk_context *ctx)
{
    if (app->show_toolsw)
        tools_window(app, ctx, WINDOW_FLAGS);

    if (app->show_tilesetw)
        tileset_window(app, ctx, WINDOW_FLAGS);

    status_window(app, ctx);
    menu_window(app, ctx);

    return 1;
}

void app_render(struct app *app, SDL_Renderer *renderer)
{
    model_window_render(renderer, app);
}

void app_deinit(struct app *app)
{
    for (int tool = 0; tool < NUMBER_OF_TOOLS; tool++)
        SDL_DestroyTexture(app->modelw.tools[tool].texture);

    if (app->tileset_texture != NULL)
        SDL_DestroyTexture(app->tileset_texture);

    if (app->map != NULL)
        free(app->map);

    IMG_Quit();
}
