// blocks.c - Block Registry Implementation
#include <stdint.h>
#include "blocks.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

BlockRegistry g_blocks = {0};

// Hardcoded fallback — works with NO assets at all
void blocks_init_defaults(void) {
    g_blocks.count = 0;

#define DEF_BLOCK(ID, NAME, FLAGS, HARD, DROP, T_TOP, T_SIDE, T_BOT, LIGHT, FRIC) \
    do { \
        BlockDef* b = &g_blocks.defs[g_blocks.count++]; \
        b->id = ID; \
        strncpy(b->name, NAME, 31); \
        b->flags    = FLAGS; \
        b->hardness = HARD; \
        b->drop_id  = DROP; \
        b->tex_top  = T_TOP; \
        b->tex_side = T_SIDE; \
        b->tex_bottom = T_BOT; \
        b->light_emit = LIGHT; \
        b->friction   = FRIC; \
    } while(0)

    // ID  NAME       FLAGS                                                          HARD  DROP    TOP  SIDE BOT  LIGHT FRIC
    DEF_BLOCK(0,  "air",      0,                                                    0.0f, 0,      0,   0,   0,   0,    255);
    DEF_BLOCK(1,  "dirt",     BLOCK_FLAG_SOLID|BLOCK_FLAG_PLACEABLE|BLOCK_FLAG_BREAKABLE, 0.5f, 1, 1, 1, 1, 0, 200);
    DEF_BLOCK(2,  "grass",    BLOCK_FLAG_SOLID|BLOCK_FLAG_PLACEABLE|BLOCK_FLAG_BREAKABLE, 0.6f, 1, 2, 3, 1, 0, 200);
    DEF_BLOCK(3,  "stone",    BLOCK_FLAG_SOLID|BLOCK_FLAG_PLACEABLE|BLOCK_FLAG_BREAKABLE, 1.5f, 12, 4, 4, 4, 0, 210);
    DEF_BLOCK(4,  "wood_log", BLOCK_FLAG_SOLID|BLOCK_FLAG_PLACEABLE|BLOCK_FLAG_BREAKABLE, 2.0f, 4, 5, 6, 5, 0, 180);
    DEF_BLOCK(5,  "leaves",   BLOCK_FLAG_SOLID|BLOCK_FLAG_TRANSPARENT|BLOCK_FLAG_BREAKABLE, 0.2f, 0, 7, 7, 7, 0, 100);
    DEF_BLOCK(6,  "sand",     BLOCK_FLAG_SOLID|BLOCK_FLAG_PLACEABLE|BLOCK_FLAG_BREAKABLE, 0.5f, 6, 8, 8, 8, 0, 160);
    DEF_BLOCK(7,  "water",    BLOCK_FLAG_LIQUID|BLOCK_FLAG_TRANSPARENT,                   0.0f, 0, 9, 9, 9, 0, 50);
    DEF_BLOCK(8,  "bedrock",  BLOCK_FLAG_SOLID,                                           -1.f, 8, 10,10,10, 0, 255);
    DEF_BLOCK(9,  "coal_ore", BLOCK_FLAG_SOLID|BLOCK_FLAG_BREAKABLE,                      3.0f, 9, 11,11,11, 0, 210);
    DEF_BLOCK(10, "iron_ore", BLOCK_FLAG_SOLID|BLOCK_FLAG_BREAKABLE,                      3.0f,10, 12,12,12, 0, 210);
    DEF_BLOCK(11, "torch",    BLOCK_FLAG_TRANSPARENT|BLOCK_FLAG_PLACEABLE|BLOCK_FLAG_BREAKABLE, 0.0f, 11, 13,13,13, 14, 255);
    DEF_BLOCK(12, "cobble",   BLOCK_FLAG_SOLID|BLOCK_FLAG_PLACEABLE|BLOCK_FLAG_BREAKABLE, 2.0f,12, 14,14,14, 0, 210);

#undef DEF_BLOCK
}

// Minimal JSON parser — no cJSON dependency needed
// Format expected: [{"id":1,"name":"dirt","hardness":0.5,...}, ...]
bool blocks_load_json(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) {
        printf("[Blocks] '%s' not found, using defaults.\n", path);
        return false;
    }

    // For now read entire file — max 32KB expected
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    rewind(f);

    if (sz <= 0 || sz > 32768) {
        fclose(f);
        printf("[Blocks] JSON too large or empty, using defaults.\n");
        return false;
    }

    char* buf = (char*)malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return false; }
    fread(buf, 1, (size_t)sz, f);
    buf[sz] = '\0';
    fclose(f);

    // Simple field extractor — finds "key":value pairs
    // This is intentionally minimal to avoid libc++ / cJSON deps on NDK
    g_blocks.count = 0;
    char* p = buf;
    while (*p && g_blocks.count < MAX_BLOCK_TYPES) {
        // Find start of object '{'
        while (*p && *p != '{') p++;
        if (!*p) break;
        p++; // skip '{'

        BlockDef b = {0};
        b.flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_PLACEABLE | BLOCK_FLAG_BREAKABLE;
        b.friction = 200;

        // Parse until '}'
        char* obj_end = p;
        int depth = 1;
        while (*obj_end && depth > 0) {
            if (*obj_end == '{') depth++;
            else if (*obj_end == '}') depth--;
            obj_end++;
        }

        // Scan key:value pairs
        char key[32], val[64];
        char* scan = p;
        while (scan < obj_end) {
            // Find '"key"'
            while (scan < obj_end && *scan != '"') scan++;
            if (scan >= obj_end) break;
            scan++;
            int ki = 0;
            while (*scan && *scan != '"' && ki < 31) key[ki++] = *scan++;
            key[ki] = '\0';
            scan++; // closing quote

            // Skip ':'
            while (scan < obj_end && *scan != ':') scan++;
            scan++;
            while (scan < obj_end && (*scan == ' ' || *scan == '\t')) scan++;

            // Read value
            int vi = 0;
            if (*scan == '"') {
                scan++;
                while (*scan && *scan != '"' && vi < 63) val[vi++] = *scan++;
                if (*scan == '"') scan++;
            } else {
                while (*scan && *scan != ',' && *scan != '}' && vi < 63) {
                    val[vi++] = *scan++;
                }
            }
            val[vi] = '\0';

            // Map to struct
            if      (!strcmp(key, "id"))        b.id        = (uint8_t)atoi(val);
            else if (!strcmp(key, "name"))       strncpy(b.name, val, 31);
            else if (!strcmp(key, "hardness"))   b.hardness  = (float)atof(val);
            else if (!strcmp(key, "drop_id"))    b.drop_id   = (uint8_t)atoi(val);
            else if (!strcmp(key, "tex_top"))    b.tex_top   = (uint8_t)atoi(val);
            else if (!strcmp(key, "tex_side"))   b.tex_side  = (uint8_t)atoi(val);
            else if (!strcmp(key, "tex_bottom")) b.tex_bottom= (uint8_t)atoi(val);
            else if (!strcmp(key, "light"))      b.light_emit= (uint8_t)atoi(val);
            else if (!strcmp(key, "solid"))      {
                if (atoi(val)) b.flags |= BLOCK_FLAG_SOLID;
                else           b.flags &= ~BLOCK_FLAG_SOLID;
            }
        }

        g_blocks.defs[g_blocks.count++] = b;
        p = obj_end;
    }

    free(buf);
    printf("[Blocks] Loaded %d block types from JSON.\n", g_blocks.count);
    return (g_blocks.count > 0);
}

const BlockDef* block_get(uint8_t id) {
    for (int i = 0; i < g_blocks.count; i++) {
        if (g_blocks.defs[i].id == id) return &g_blocks.defs[i];
    }
    return &g_blocks.defs[0]; // air as fallback
}

bool block_is_solid(uint8_t id) {
    return (block_get(id)->flags & BLOCK_FLAG_SOLID) != 0;
}
bool block_is_transparent(uint8_t id) {
    return (block_get(id)->flags & BLOCK_FLAG_TRANSPARENT) != 0;
}
bool block_is_liquid(uint8_t id) {
    return (block_get(id)->flags & BLOCK_FLAG_LIQUID) != 0;
}
