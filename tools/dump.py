import os
import re
import sys

from typing import NamedTuple
from dataclasses import dataclass
from op import Op

@dataclass
class ProgramInfo():
    keymap: str = ""
    info: str = ""
    shiftquirk: bool = False

class Program(NamedTuple):
    name: str
    codename: str
    size: int
    super: bool
    info: ProgramInfo

def read_group(f, pc):
    next_group = f.read(16)
    for i in range(0, len(next_group), 2):
        print("  //0x{:04X}: {:20s} |    0x{:04X}: {}".format(
            pc+i, str(Op(next_group[i:])),
            pc+i+1, str(Op(next_group[i+1:]))
        ))

    print(end="  ")
    for b in next_group:
        print("0x{:02X}".format(b),end=', ')
    print("")
    return len(next_group)


def dump_program_to_array(base, filename, codename):
    print("const uint8_t {}[] PROGMEM = {{".format(codename))
    progfile = open(os.path.join(base, filename), 'rb')
    pc = 0x200
    while (amt := read_group(progfile, pc)) > 0:
        pc += amt

    print("};\n")
    return pc - 0x200


# Octo WASD configuration
DEFAULT_KEYMAP = "0x58, 0x79, 0x46"

def get_program_info(base, filename):
    pgm = ProgramInfo(keymap=DEFAULT_KEYMAP)

    try:
        f = open(os.path.join(base, "{}.info".format(filename)))
        for line in (l.strip() for l in f.readlines()):
            field, *rest = line.split("=", 2)
            if field == "shiftquirk":
                pgm.shiftquirk = True
            elif field == "info":
                pgm.info = rest[0]
            elif field == "keymap":
                pgm.keymap = rest[0]

    except FileNotFoundError:
        sys.stderr.write("No info found for {}\n".format(filename))

    return pgm


def get_program(base, fullname):
    filename, ext = os.path.splitext(fullname)
    codename = re.sub('[^a-zA-Z0-9_]', '_', filename)+"_"+ext[1:]
    info = get_program_info(base, filename)
    size = dump_program_to_array(base, fullname, codename)
    return Program(filename, codename, size, ext == ".sch8", info)


def dump_all_roms(base):
    menu = open(os.path.join(base, "menu"))

    programs = [get_program(base, name.strip()) for name in menu.readlines()]

    print("const uint8_t PROGRAM_COUNT = {};".format(len(programs)))
    for p in programs:
        print("const char name_{}[] PROGMEM = \"{}\";".format(p.codename, p.name))
    print("")
    for p in programs:
        print("const char info_{}[] PROGMEM = \"{}\";".format(p.codename, p.info.info))

    print("const Program programs[] PROGMEM = {")
    for p in programs:
        print("""\
    (Program){{
        .name=(char*)name_{0.codename},
        .code=(uint8_t*){0.codename},
        .size={0.size},
        .super={0.super:d},
        .info=(uint8_t*)info_{0.codename},
        .keymap={{{0.info.keymap}}},
        .shiftquirk={0.info.shiftquirk:d},
    }},""".format(p))
    print("};")

if __name__ == "__main__":
    print("#include \"program.h\"")
    dump_all_roms("roms")
