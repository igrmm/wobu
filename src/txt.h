#ifndef TXT_H
#define TXT_H

#include "SDL.h" // IWYU pragma: keep //clangd
#include "SDL_image.h"
#include "SDL_ttf.h"

struct txt_font;
struct txt_codepoint_cache;

int txt_get_codepoints(Uint32 *cp, size_t cp_maxlen, const char *str);
void txt_cache_codepoint(struct txt_codepoint_cache *cache, const char *str);
struct txt_codepoint_cache *txt_create_codepoint_cache(void);
struct txt_font *txt_create_font(struct txt_codepoint_cache *cache,
                                 TTF_Font *ttf, SDL_Renderer *ren);
void txt_destroy_font(struct txt_font *font);
int txt(const char *str, float x, float y, SDL_Renderer *ren,
        struct txt_font *font);

#endif
