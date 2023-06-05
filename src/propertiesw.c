#include "SDL.h"

#include "app.h"
#include "propertiesw.h"

int properties_window(struct app *app, struct nk_context *ctx, const int flags)
{
    const char *combo[] = {"no selection"};
    static int selected = 0;

    if (nk_begin(ctx, "properties", nk_rect(20, 365, 200, 200), flags)) {
        nk_layout_row_dynamic(ctx, 25, 1);
        selected = nk_combo(ctx, combo, NK_LEN(combo), selected, 25,
                            nk_vec2i(150, 150));
    } else {
        app->show_propertiesw = 0;
    }
    nk_end(ctx);

    return 1;
}
