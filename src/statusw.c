#include "SDL.h"

#include "app.h"
#include "statusw.h"

static void query_fps(struct fps_counter *fps_counter)
{
    Uint32 now = SDL_GetTicks64();
    if (now - fps_counter->timer >= 1000) {
        int frames = SDL_min(fps_counter->frames, 999);
        SDL_snprintf(fps_counter->fps, sizeof fps_counter->fps, "fps: %i",
                     frames);
        fps_counter->timer = now;
        fps_counter->frames = 0;
    }
    fps_counter->frames++;
}

int status_window(struct app *app, struct nk_context *ctx)
{
    query_fps(&app->fps_counter);

    int h = 20;
    int w = app->screen_width;
    int x = 0;
    int y = app->screen_height - h;

    if (nk_begin(ctx, "status", nk_rect(x, y, w, h), NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_dynamic(ctx, h, 1);
        nk_label(ctx, app->fps_counter.fps, NK_TEXT_ALIGN_LEFT);
    }
    nk_end(ctx);

    return 1;
}
