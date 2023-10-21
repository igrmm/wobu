#include "../external/json.h/json.h"
#include "SDL.h"

#include "jsonffile.h"

struct json_value_s *json_from_file(const char *path)
{
    char jstr[JSON_BUFSIZ];
    jstr[0] = 0; // ALWAYS INITIALIZE C STRINGS

    SDL_RWops *file = SDL_RWFromFile(path, "r");

    if (file == NULL) {
        SDL_Log("Error opening json file: %s", path);
        return NULL;
    }

    for (size_t i = 0; i < JSON_BUFSIZ; i++) {
        if (SDL_RWread(file, &jstr[i], sizeof(char), 1) <= 0) {
            jstr[i] = 0;
            break;
        }
    }

    SDL_RWclose(file);

    size_t len = SDL_strlen(jstr);

    SDL_Log("JSON loaded from file into json string: %zu bytes.", len);

    return json_parse(jstr, len);
}
