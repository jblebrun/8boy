Arduboy Chip8

## What is it?

It's a CHIP-8 (and SCHIP-8) emulator for Arduboy. CHIP-8 was a virtual machine that ran on an 8-bit microcontroller in the 1970s! Take that, Java.

The COSMAC VIP (and family) was an affordable computer kit based on the RCA COSMAC 1802 CPU. It came with a simple 16-key hex keyboard, the ability to show simple monochrome graphics on a television, some memory, and a beeper.

CHIP-8 was part of the ROM "operating system" included with the kit. This 512-byte OS ROM contained routines for interacting with the hardware, but (more importantly) it contained a simple virtual machine called CHIP-8. Programs could be written using this simpler machine language.

While many emulators for the CHIP-8 instruction set have been written since, one of the more popular ones was written for the HP-48 calculator. In addition to running CHIP-8 games, it included a few extensions to allow higher resolution, scrolling, and a bit more memory.

There are endless resources online about CHIP-8 (and SCHIP-8) emulators. Because of the ease of implementation, these platforms are popular emulation targets.


## How does this work?

You can specify roms to include in the Arduboy binary by including a rom in the `/roms` directory, and adding the filename to `/roms/menu`. 

The easiest way to build the project is to obtain the `aduino-cli` tools, and use the provided `Makefile`. Typing make will generate a new programs.h, compile the program, and upload it to an attached Arduboy (on an OS X machine, anyway). 

CHIP-8 games should use the filename format `name.ch8`, where name is a valid program identifier (since we just use the name as a variable name in the code). Similarly `name.sch8` indicates a super-chip8 game.

You can also include `name.map` for keymappings (see below), and `name.info` to include a small info string to display in the program loader menu.


## Implementation Notes
There are a few limitations to the Arduboy that make it non-trivial to port CHIP-8 games to this platform. Here's how they've been addressed:

### Memory

The CHIP-8 instruction set is capable of addressing up to 4kb of memory. The Arduboy has only 2.5kb of RAM (a large chunk of which is taken up by the screen buffer!). However, all of the games I've found so far don't actively use more than a few hundred bytes of memory at runtime. Since the program data is stored in flash, we just need a solution to figure out how to satisfy memory reads and writes as the programs execute.

A strategy that's been working well for every game so far is to use a "slab allocation" approach. That is, we maintain an array of 16-byte "slabs" and reserve spaces for as many of them as possible. Each slab contains 16 bytes of data, and an 8-byte "page identifier". Because the original COSMAC VIP OS was resident in pages 0 and 1, we assume that nothing will try to write there, and we use page number 0 as a "not used" marker. See the code for more details.


### Keys

The CHIP-8 runs on the COSMAC VIP, with a 16-key keypad. The Arduboy has 6 keys. Conveniently, most games only use a subset of the keys. Unfortunately, the subset tends to be different for each game. So, it's necessary to figure out which keys a game needs, then a mapping file can be included in the `/rom` directory. 

The mapping file is simply a text representation of 3 bytes as they would appear in an array initializer in C. The 3 bytes correspond to 6 nibbles in the order: U, D, L, R, A, B, mapping them to one of the 16 keys on the COSMAC VIP keypad.

