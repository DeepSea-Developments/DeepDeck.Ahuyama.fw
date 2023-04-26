#pragma once

#include <stdint.h>

typedef uint8_t uuid_t[16];

#define UUID_STR_LEN 37

void uuid_generate(uuid_t out);
int uuid_parse(const char *in, uuid_t uu);
void uuid_unparse(const uuid_t uu, char *out);
