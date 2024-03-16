#include "SDL.h" // IWYU pragma: keep //clangd

#define NK_IMPLEMENTATION
#include "nk.h"
#define NK_SDL_RENDERER_IMPLEMENTATION
#include "../external/Nuklear/demo/sdl_renderer/nuklear_sdl_renderer.h"

#include "app.h"

static SDL_Window *win;
static SDL_Renderer *renderer;
static struct nk_context *ctx;
static struct app app;
static int setup(void);
static void shutdown(void);
static int running = 0;

int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[])
{
    if (setup() < 0)
        shutdown();

    while (running) {
        /* Input */
        SDL_Event evt;
        nk_input_begin(ctx);
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT)
                running = 0;

            if (nk_item_is_any_active(ctx)) {
                nk_sdl_handle_event(&evt);
            } else {
                if (evt.type != SDL_MOUSEWHEEL) {
                    nk_sdl_handle_event(&evt);
                }
                app_handle_event(&app, &evt);
            }
        }
        nk_input_end(ctx);

        /* GUI */
        SDL_GL_GetDrawableSize(win, &app.screen_width, &app.screen_height);
        if (!app_run(&app, ctx)) {
            SDL_Log("Failed to run app.");
            running = 0;
        };

        /* Render */
        SDL_SetRenderDrawColor(renderer, 26.0f, 46.0f, 61.0f, 255.0f);
        SDL_RenderClear(renderer);
        app_render(&app, renderer);
        nk_sdl_render(NK_ANTI_ALIASING_ON);
        SDL_RenderPresent(renderer);
    }

    shutdown();
    return 0;
}

static int setup(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init Error: %s\n", SDL_GetError());
        return -1;
    }

    win =
        SDL_CreateWindow("wobu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         1200, 800, SDL_WINDOW_RESIZABLE);
    if (win == NULL) {
        SDL_Log("SDL_CreateWindow Error: %s\n", SDL_GetError());
        return -1;
    }

    renderer = SDL_CreateRenderer(win, -1, 0);
    if (renderer == NULL) {
        SDL_Log("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return -1;
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

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

    if (!app_init(&app, renderer)) {
        SDL_Log("Failed to create app.");
        return -1;
    }

    running = 1;

    return 0;
}

static void shutdown(void)
{
    app_deinit(&app);
    nk_sdl_shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
}
