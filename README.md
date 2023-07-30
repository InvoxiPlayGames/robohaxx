# robohaxx: stackcry

A savegame exploit / homebrew entrypoint for the game Robotech: Battlecry on the GameCube.

Compatible with both USA and EUR revisions of the game. (Only USA has been tested on real GC hardware.)

["it's a stack overflow in the profile name on a 6th gen console game"](https://tenor.com/view/buzz-lightyear-factory-you-will-never-find-another-store-shelf-a-bunch-of-buzz-lightyears-gif-21719996) (Tenor GIF link)

## How to Use

1. Copy the .gci file for your region from [the robohaxx releases page](https://github.com/InvoxiPlayGames/robohaxx/releases)
   to your Memory Card using [GCMM](https://github.com/suloku/gcmm/releases).
    * While you're here, make sure you have the latest Swiss boot GCI, too. (Or any other boot.dol.)
    * Ensure there are **NO** other savegames for Robotech: Battlecry on the memory card.
2. Launch Robotech: Battlecry.
3. At the main menu, select the "Load Game" option.
4. Afetr a few seconds, Swiss (or any other homebrew) *should* load!

## Credits

Thank you to [FIX94](https://github.com/FIX94) for the gc-exploit-common-loader DOL loader, and for
example code for memory card loaders in other GC savegame exploits.

Thanks to [Essem](https://github.com/TheEssem) for testing on real hardware, and being the inspiration behind this 
[by just wanting a way to launch Swiss that isn't Animal Crossing.](https://wetdry.world/@esm/110792836912696997)

## Building from Source

The source has two components, the memory card loader and the GCI builder. The memory card loader will
build if you have devkitPPC installed (although any powerpc-eabi GCC will work, change the Makefile).

The GCI builder should build on any macOS/Linux system, and should be fine building under MingW64 on Windows.

The Makefile in the root of the repository will compile the memcard loader for EUR and USA regions,
compile the GCI builder, and then build the robohaxx GCI savefiles. Type `make` at a terminal.

If you only want to build for a specific region, type `make usa` or `make eur`.

## License

The memory card loader and GCI builder are licensed under the GNU General Public License version 2. See attached
LICENSE file for more details.

This chain uses FIX94's [gc-exploit-common-loader](https://github.com/FIX94/gc-exploit-common-loader), also licensed
under GPLv2.
