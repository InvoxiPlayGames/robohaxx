OUTPUT_FORMAT("elf32-powerpc")
OUTPUT_ARCH(powerpc:common)

SECTIONS {
	/* Game functions (GRBP6S) */ 
    OSDisableInterrupts = 0x80026c6c;
    __OSStopAudioSystem = 0x80024958;
    GXDrawDone = 0x8001c8cc;
    DCFlushRange = 0x80024a70;
    CARDGetStatus = 0x80012ca4;
    CARDFastOpen = 0x80011b4c;
    CARDRead = 0x800124d0;
    CARDClose = 0x80011cac;
    CARDUnmount = 0x8001118c;
    _memcpy = 0x80003490;
    _memcmp = 0x80032350;
    
	/* Space right after profile name in memory after save file is loaded. */
    . = 0x80E37750;
    .robohaxx 0x80E37750+0 :
    {
        *.o(.start)
        *(.text)
        *(.rodata .rodata.*)
        *(.data)
        *(.bss)
    }
}
