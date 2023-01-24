#include <stdlib.h>

#include <SDL2/SDL.h>

#define NK_IMPLEMENTATION
#include "nk.h"
#define NK_SDL_RENDERER_IMPLEMENTATION
#include "external/nuklear_sdl_renderer.h"

#include "app.h"

int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[])
{
    struct app *app;
    SDL_Window *win;
    SDL_Renderer *renderer;
    int running = 1;

    struct nk_context *ctx;

    SDL_Init(SDL_INIT_VIDEO);

    win = SDL_CreateWindow("wobu", SDL_WINDOWPOS_CENTERED,
                           SDL_WINDOWPOS_CENTERED, 1200, 800, SDL_WINDOW_SHOWN);

    if (win == NULL) {
        SDL_Log("Error SDL_CreateWindow %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    renderer = SDL_CreateRenderer(
        win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (renderer == NULL) {
        SDL_Log("Error SDL_CreateRenderer %s", SDL_GetError());
        SDL_DestroyWindow(win);
        exit(EXIT_FAILURE);
    }

    ctx = nk_sdl_init(win, renderer);
    {
        struct nk_font_atlas *atlas;
        struct nk_font_config config = nk_font_config(0);
        struct nk_font *font;

        nk_sdl_font_stash_begin(&atlas);
        font = nk_font_atlas_add_default(atlas, 13, &config);
        nk_sdl_font_stash_end();
        nk_style_set_font(ctx, &font->handle);
    }

    int exit_status = EXIT_SUCCESS;

    app = app_create(renderer, ctx);
    if (app == NULL) {
        SDL_Log("Failed to create app.");
        exit_status = EXIT_FAILURE;
        goto cleanup;
    }

    while (running) {
        /* Input */
        SDL_Event evt;
        nk_input_begin(ctx);
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT)
                goto cleanup;

            nk_sdl_handle_event(&evt);
        }
        nk_input_end(ctx);

        /* GUI */
        if (!app_run(app)) {
            SDL_Log("Failed to run app.");
            exit_status = EXIT_FAILURE;
            goto cleanup;
        };

        /* Render */
        SDL_SetRenderDrawColor(renderer, 26.0f, 46.0f, 61.0f, 255.0f);
        SDL_RenderClear(renderer);
        nk_sdl_render(NK_ANTI_ALIASING_ON);
        SDL_RenderPresent(renderer);
    }

cleanup:
    nk_sdl_shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    app_destroy(app);
    return exit_status;
}
