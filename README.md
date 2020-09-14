# Arduboy Chip8

## Run it!

Quick version (if you are on a machine with `make` support`):

1. Install arduino-cli (via `homebrew`, `apt`, etc).
2. Install go
3. Run `make install`

## Run SDL-based renderer on (only tested on macOS)

1. Install go
2. Run `make sdl-run`


## What is it?

It's a CHIP-8 (and SCHIP-8) emulator for Arduboy. CHIP-8 was a virtual machine that ran on an 8-bit microcontroller in the 1970s! Take that, Java.

The COSMAC VIP (and family) was an affordable computer kit based on the RCA COSMAC 1802 CPU. It came with a simple 16-key hex keyboard, the ability to show simple monochrome graphics on a television, some memory, and a beeper. It was the successor to the COSMAC ELF, which was quite similar, but without the video chip. If you're interested in older platforms like this, I *highly* recommend checking out [Lee Hart's ElectroniKits](http://www.sunrise-ev.com/projects.htm). He has really well done kits that pay homage to both of these COSMAC platforms, but shrunken to fit in an Altoids tin!

CHIP-8 was part of the ROM "operating system" included with the COSMAC VIP kit. This 512-byte OS ROM contained routines for interacting with the hardware, but (more importantly) it contained a simple virtual machine called CHIP-8. Programs could be written using this simpler machine language. 

While many emulators for the CHIP-8 instruction set have been written since, one of the more popular ones was written for the HP-48 calculator by Erik Bryntse. In addition to running CHIP-8 games, it included a few extensions to allow higher resolution, scrolling, and a bit more memory, often referred to a "Super Chip-8" (or SCHIP-8).

There are endless resources online about CHIP-8 (and SCHIP-8) emulators. Because of the ease of implementation, these platforms are popular emulation targets.

Some great resources include:
* http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
* http://benryves.com/bin/vinegar/
* http://mattmik.com/files/chip8/mastering/chip8.html
* http://www.pong-story.com/chip8/
* Original COSMAC VIP manual: http://cosmacelf.com/forumarchive/files/VIP/Cosmac%20VIP%20Manual.pdf
* https://chip-8.github.io/ - Lots of good info on extensions and implementation quirks


## How does this work?

You can specify roms to include in the Arduboy binary by including a rom in the `/roms` directory, and adding the filename to `/roms/menu`. 

The easiest way to build the project is to obtain the `arduino-cli` tools, and use the provided `Makefile`. Typing make will generate a new programs.h, compile the program, and upload it to an attached Arduboy (on an OS X machine, anyway). 

CHIP-8 games should use the filename format `name.ch8`, where name is a valid program identifier (since we just use the name as a variable name in the code). Similarly `name.sch8` indicates a super-chip8 game.

You can also include `name.map` for keymappings (see below), and `name.info` to include a small info string to display in the program loader menu.

The program loader makes a feeble attempt at disassembly, which it will show in comments next to the generated code. Note that it is easily confused: data will be interpreted by code since without flow analysis, there's no way to tell that the bytes are data. Similarly, if data is odd-sized, and program instructions become aligned on odd bytes instead of even, the disassembly will be garbage.


## Implementation Notes

The graphics, input, sound, and random implementations are delegated to an
implementation that's provided when the engine is construucted.

Since there were some tight restrictions on memory mapping on the ArduBoy (see
below), memory reads and writes are also abstracted to component to be provided
at construction time. The implementation includes a default memory
implementation that simply maps reads and writes into a statically-allocated 4k
array.  If you're deploying to a platform with more than 4k of RAM, this
implementation is probably sufficient. It wasn't sufficient for the ArduBoy,
which is why this component ws abstracted out (see below).

### Be careful about implementations!

This is an old platform, which has seen many implementations, and has lots written about it. Here are some things I've noticed along the way:

* Some older documentation incorrectly describes the behavior of register VF during subtraction operations. Quite a few of the popular resources have this wrong, but the COSMAC VIP manual does give the correct information: The VF register will be set to 1 if VX is greater than *or equal to* VY. That is, if a borrow does not occur.

* In some implementations, the register load/store from memory operations leave the I register pointing to the end of memory space that was worked with. But some games don't operate correctly if this is the behavior.

* Some programs have been written to expect certain quirks in behavior, so including configuration options is necessary to support all of the various programs you might find.

* I was sloppy, and for a long time didn't notice that the random implementation is rand(0xFF) & value -- which leads to almost correct but subtle weird behavior in some games.

* Broadly: there are a lot of subtle details to get wrong, and it's easy for things to more or less seem to work correctly. Writing some unit tests for the operations probably would have saved me a lot of time debugging just to find out that a `^` should have been an `&`, or a `(a & !b)` should have been `(a & b)`. Especially in that pesky drawing routine!

### ArduBoy 

There are a few limitations to the Arduboy that make it non-trivial to port CHIP-8 games to this platform. This implementation includes some hacks that work for the programs I've encountered so far. 

#### Memory

The CHIP-8 instruction set is capable of addressing up to 4kb of memory. The Arduboy has only 2.5kb of RAM (a large chunk of which is taken up by the screen buffer!). However, all of the games I've found so far don't actively use more than a few hundred bytes of memory at runtime. Since the program data is stored in flash, we just need a solution to figure out how to satisfy memory reads and writes as the programs execute.

A strategy that's been working well for every game so far is to use a "slab allocation" approach. That is, we maintain an array of 16-byte "slabs" and reserve spaces for as many of them as possible. Each slab contains 16 bytes of data, and an 8-byte "page identifier". Because the original COSMAC VIP OS was resident in pages 0 and 1, we assume that nothing will try to write there, and we use page number 0 as a "not used" marker. See the code for more details.


#### Keys

The CHIP-8 runs on the COSMAC VIP, with a 16-key keypad. The Arduboy has 6 keys. Conveniently, most games only use a subset of the keys. Unfortunately, the subset tends to be different for each game. So, it's necessary to figure out which keys a game needs, then a mapping file can be included in the `/rom` directory. 

The mapping file is simply a text representation of 3 bytes as they would appear in an array initializer in C. The 3 bytes correspond to 6 nibbles in the order: U, D, L, R, A, B, mapping them to one of the 16 keys on the COSMAC VIP keypad. (which are labeled by one of the 16 hexidecimal digits 0-F).

For example, if the map file contains the text: `0x28, 0x46, 0xAB` then:
* Up -> keypad key 2
* Down -> keypad key 8
* Left -> keypad key 4
* Right -> keypad key 6
* A -> keypad key A
* B -> keypad key B


## Debugging

An instruction tracer and error dumper is included. It's not on by default. If
you want to dump instructions, you can either use re-compile the program with
`SerialTracer(true)` and then use `cat` to print serial messages, or use a
two-way terminal (`picocom` is a nice option), connect to the port, and press
any key to toggle instruction tracing.
