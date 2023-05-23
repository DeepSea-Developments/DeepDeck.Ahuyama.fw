#pragma once

#include <stdint.h>

typedef uint8_t uuid_t[16];

#define UUID_STR_LEN 37
#define SHORT_UUID_STR_LEN 7 ///7. but we don´t want to erase-flash.

void uuid_generate(uuid_t out);
int uuid_parse(const char *in, uuid_t uu);
void uuid_unparse(const uuid_t uu, char *out);
void short_uuid_unparse(const uuid_t uu, char *out);
