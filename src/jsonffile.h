#ifndef JSONFFILE_H
#define JSONFFILE_H

#include "../external/json.h/json.h"

#define JSON_BUFSIZ 1000000

struct json_value_s *json_from_file(const char *path);

#endif
