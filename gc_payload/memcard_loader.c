/*
	memcard_loader, a memory card boot.dol loader payload, for the robohaxx exploit
    Copyright 2023 Emma / InvoxiPlayGames
	This code is licensed to you under the terms of the GNU GPL, version 2;
	see file LICENSE or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

#include "loader.h"

#define WORK_ADDR 0x80001800
#define VirtToPhys(x) (x & 0x017FFFFF)

#define CARD_A 0
#define CARD_B 1

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef volatile unsigned int vu32;
typedef volatile unsigned short vu16;

// structure for an opened file from the card
typedef struct _CARDFileInfo {
    int channel;
    int file_num;
    int offset;
    int length;
    u16 block;
} CARDFileInfo;
// structure for the file status returned by CARDGetStatus
typedef struct _CARDFileStatus {
    char name[0x20];
    int length;
    char unk[0x2C];
} CARDFileStatus;

// funcs from the game that we need
extern u32 OSDisableInterrupts(void);
extern void __OSStopAudioSystem(void);
extern void GXDrawDone(void);
extern u32 DCFlushRange(void *address, u32 size);
extern int CARDGetStatus(int card, int file_num, CARDFileStatus *status);
extern int CARDFastOpen(int card, int file_num, CARDFileInfo *info);
extern int CARDRead(CARDFileInfo *info, void *address, int length, int offset);
extern int CARDClose(CARDFileInfo *info);
extern int CARDUnmount(int card);
extern void *_memcpy(void *dest, void *src, u32 size);
extern int _memcmp(void *buf1, void *buf2, u32 len);

// DMA to ARAM, this is inlined just to make my life easier
// it's only used in one loop, should be fine
static inline void ARAM_DMA(u32 direction, u32 mram, u32 aram, u32 len) {
    *(vu16 *)0xCC005020 = (mram >> 16);
    *(vu16 *)0xCC005022 = (mram & 0xFFFF);
    *(vu16 *)0xCC005024 = (aram >> 16);
    *(vu16 *)0xCC005026 = (aram & 0xFFFF);
    *(vu16 *)0xCC005028 = (direction << 15) | (direction >> 16);
    *(vu16 *)0xCC00502A = (len & 0xFFFF);
    while (*(vu16 *)0xCC00500A & 0x200);
}

void _start() {
    CARDFileInfo fileInfo;
    CARDFileStatus fileStatus;
    u32 readOffset = 0;
    int i = 0;
    
    // get game to shut up
    OSDisableInterrupts();
    __OSStopAudioSystem();
    GXDrawDone();
    
    // set gameid in RAM to DOLX00 so we can load the boot.dol save
    *(vu32 *)0x80000000 = 0x444F4C58;
    *(vu16 *)0x80000004 = 0x3030;
    DCFlushRange((void*)0x80000000, 0x20);
    
    // find boot.dol on the memory card and open it
    for (i = 0; i < 127; i++) {
        if (CARDGetStatus(CARD_A, i, &fileStatus) != 0)
            continue;
        if (_memcmp(fileStatus.name, "boot.dol", 9) == 0)
            break;
    }
    CARDFastOpen(CARD_A, i, &fileInfo);
    // continually read it until we get an error, copying into ARAM
    while (CARDRead(&fileInfo, (void*)WORK_ADDR, 0x200, readOffset) == 0) {
        DCFlushRange((void*)WORK_ADDR, 0x200);
        ARAM_DMA(0, VirtToPhys(WORK_ADDR), readOffset, 0x200);
        readOffset += 0x200;
    }
    // unmount the card, we're done
    CARDClose(&fileInfo);
    CARDUnmount(CARD_A);
    
    // copy the DOL loader into the spare interrupt vectors
    _memcpy((void*)WORK_ADDR, (void*)loader, loader_size);
    DCFlushRange((void*)WORK_ADDR, loader_size);
    // jump! (if WORK_ADDR is changed, change this too)
    __asm__ volatile(
        "lis 3, 0x8000\n"
        "ori 3, 3, 0x1800\n"
        "mtlr 3\n"
        "blr\n"
    );
    
    // gcc, we shouldn't be here
    __builtin_unreachable();
}
