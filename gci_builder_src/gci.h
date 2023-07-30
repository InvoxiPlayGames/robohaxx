#ifndef _GCI_H
#define _GCI_H

#include <stdint.h>

typedef struct _GCIHeader_t {
    char gamecode[4]; 
    char maker[2];
    uint8_t unused_always_ff;
    uint8_t banner_flags;
    char filename[0x20];
    uint32_t file_timestamp;
    uint32_t image_offset;
    uint8_t icon_fmt[2];
    uint8_t anim_speed[2];
    uint8_t permissions;
    uint8_t copy_counter;
    uint16_t first_block;
    uint16_t num_blocks;
    uint16_t unused_always_ffff;
    uint32_t strings_offset;
} GCIHeader_t;

typedef struct _GCIStrings_t {
    char game_name[0x20];
    char save_name[0x20];
} GCIStrings_t;

#endif // _GCI_H
