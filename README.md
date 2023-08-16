# robohaxx: stackcry

A savegame exploit / homebrew entrypoint for the game Robotech: Battlecry on the GameCube and Xbox.

Compatible with both USA and EUR revisions of the game, and all 7 retail Xbox kernels. (Untested)

["it's a stack overflow in the profile name on a 6th gen console game"](https://tenor.com/view/buzz-lightyear-factory-you-will-never-find-another-store-shelf-a-bunch-of-buzz-lightyears-gif-21719996) (Tenor GIF link)

## How to Use (Xbox)

1. Get the kernel version from your Xbox using the dashboard.
    * Go to Settings -> System Info, then read the value that says **K** (e.g. K:1.00.**5530**.01, bold is the important part).
    * *The part that says "D" is the dashboard version, and does not matter!*
2. Download the .zip file for your region and kernel version from [the robohaxx releases page](https://github.com/InvoxiPlayGames/robohaxx/releases/tag/release-xbox-1.0)
   and extract the contents, then copy it to an Xbox formatted USB drive using a tool like Xplorer360.
    * If using Rocky5's softmod tool, you also want to have the "Softmod Save" from [Rocky5's Xbox Softmodding Tool](https://github.com/Rocky5/Xbox-Softmodding-Tool/blob/master/README.md)
    copied to the USB drive as well. This file is in the "Softmod Package" folder in the downloaded Xbox Softmodding Tool ZIP file.
    * If not using the Rocky5 softmod tool, replace the "default.xbe" in the files you extracted with whatever you're using.
3. On the Xbox dashboard, connect your USB drive and copy the robohaxx savegame as well as the Xbox Softmodding Tool.
    * Ensure there are **NO** other savegames for Robotech: Battlecry on the hard drive.
    * While you're here, double check that the kernel version and region on the save file match your console.
4. Launch Robotech: Battlecry.
5. At the main menu, select the "Load Game" option.
6. After a few seconds, the Xbox Softmodding Tool (or anything else you decide to load) *should* load!
    * The light on your Xbox will blink and change colour. This is expected and normal.

## How to Use (GameCube)

1. Copy the .gci file for your region from [the robohaxx releases page](https://github.com/InvoxiPlayGames/robohaxx/releases/tag/release-1.0)
   to your Memory Card using [GCMM](https://github.com/suloku/gcmm/releases).
    * While you're here, make sure you have the latest Swiss boot GCI, too. (Or any other boot.dol.)
    * Ensure there are **NO** other savegames for Robotech: Battlecry on the memory card.
2. Launch Robotech: Battlecry.
3. At the main menu, select the "Load Game" option.
4. Aetfer a few seconds, Swiss (or any other homebrew) *should* load!

## Credits

### Xbox

Thank you to [agarmash](https://github.com/agarmash) for the clean writeup on his [Frogger Beyond exploit](https://github.com/agarmash/FroggerBeyondExploit),
as well as assistance and motivation in building this, and the modified "ernie" shellcode.

Thank you to the NKPatcher developer(s) for the base of the shellcode used in the exploit, and thanks to [Rocky5](https://github.com/Rocky5)
for the Xbox Softmodding Tool.

Shoutouts to the helpful folks in the XboxDev Discord server for pointers and guidance in putting this together.

### GameCube

Thank you to [FIX94](https://github.com/FIX94) for the gc-exploit-common-loader DOL loader, and for
example code for memory card loaders in other GameCube savegame exploits.

Thanks to [Essem](https://github.com/TheEssem) for testing on real GameCube hardware, and being the inspiration behind
this discovery [by just wanting a way to launch Swiss that isn't Animal Crossing.](https://wetdry.world/@esm/110792836912696997)

## Building from Source (GameCube)

The source has two components, the memory card loader and the GCI builder. The memory card loader will
build if you have devkitPPC installed (although any powerpc-eabi GCC will work, change the Makefile).

The GCI builder should build on any macOS/Linux system, and should be fine building under MingW64 on Windows.

The Makefile in the root of the repository will compile the memcard loader for EUR and USA regions,
compile the GCI builder, and then build the robohaxx GCI savefiles. Type `make` at a terminal.

If you only want to build for a specific region, type `make usa` or `make eur`.

## Building from Source (Xbox)

The Xbox source consists of the modified "ernie" shellcode and the XSV builder. The ernie shellcode requires
the NASM assembler, while the XSV builder requires a C build system installed.

First, open a Terminal **in the xbox_src directory.** To build the ernie shellcode, run `nasm ernie.asm`, and to
build the XSV builder, run `make`. It should build on macOS, Linux/WSL, and MingW64 on Windows.

The resulting game.xsv file can be created by typing `./xsv_builder ernie E 5530`, replacing E with U if
targeting the USA version of the game, and 5530 with a supported target kernel version. If using another payload,
replace "ernie" with the filename, and if using a custom set of offsets, provide the filename in place of the
kernel version.

Supported kernels are 3944, 4034, 4817, 5101, 5530, 5713 and 5838, which should cover all retail kernels.
Note that kernels 5530, 5713 and 5838 use the same ROP offsets - the output savegame will be identical.

Additional support is left over for the unofficial 4627 debug kernel. Unsupported/unofficial/debug kernels can
be supported - if you use a tool such as ROPper to find offsets to identical gadgets in xboxkrnl.exe, you can
create a file containing the raw addresses in binary little-endian format according to this structure:

```c
typedef struct _ROPStringGadgets {
    uint32_t pop_eax__ret_4; // ("pop eax; ret 4;")
    uint32_t pop_ecx__pop_ebx__ret_4; // ("pop ecx; pop ebx; ret 4;")
    uint32_t xor_eax_ecx__ret; // ("xor eax, ecx; ret;")
    uint32_t jmp_eax; // ("jmp eax;")
} ROPStringGadgets;
```

## License

The memory card loader, GCI builder and XSV builder are licensed under the GNU General Public License version 2. See attached
LICENSE file for more details.

The GameCube chain uses FIX94's [gc-exploit-common-loader](https://github.com/FIX94/gc-exploit-common-loader), licensed
under GPLv2.

The Xbox chain uses a variation of [NKPatcher](https://github.com/Rocky5/Xbox-Softmodding-Tool/tree/master/App%20Sources/NKPatcher/Main%20NKP11)'s 
shellcode in `ernie.asm`, licensed under GPLv2. If this attribution is incorrect, please get in touch.
