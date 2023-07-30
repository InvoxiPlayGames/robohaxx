OUTPUT_FORMAT("elf32-powerpc")
OUTPUT_ARCH(powerpc:common)

SECTIONS {
	/* Game functions (GRBE6S) */
    OSDisableInterrupts = 0x800255B0;
    __OSStopAudioSystem = 0x80023574;
    GXDrawDone = 0x8001AD28;
    DCFlushRange = 0x80023690;
    CARDGetStatus = 0x80011920;
    CARDFastOpen = 0x800107C8;
    CARDRead = 0x8001114C;
    CARDClose = 0x80010928;
    CARDUnmount = 0x8000FDFC;
    _memcpy = 0x80003490;
    _memcmp = 0x8002F4FC;
    
	/* Space right after profile name in memory after save file is loaded. */
    . = 0x80E20370;
    .robohaxx 0x80E20370+0 :
    {
        *.o(.start)
        *(.text)
        *(.rodata .rodata.*)
        *(.data)
        *(.bss)
    }
}
