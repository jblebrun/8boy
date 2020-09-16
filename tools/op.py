class Op():
    """A class to perform basic diassembly of Chip8 instructions"""
    def __init__(self, src):
        if len(src) < 2:
            self.op = None
            return
        b1 = src[0]
        b2 = src[1]
        self.op = b1 >> 4 & 0xF
        self.x = b1 & 0xF
        self.y = b2 >> 4
        self.imm12 = ((b1 & 0xF) << 8) | b2
        self.b = b2

        self.n = b2 & 0xF

    sys = {
        0xE0: "CLS",
        0xEE: "RET",
        0xF8: "SCR",
        0xFC: "SCL",
        0xFD: "HALT",
        0xFE: "LO",
        0xFF: "HI"
    }

    aluop = {
        0: "LD",
        1: "OR",
        2: "AND",
        3: "XOR",
        4: "ADD",
        5: "SUB",
        6: "SHR",
        7: "SUBN",
        0xE: "SHL"
    }

    fop = {
        0x07: "LD     V{:1X}, DT",
        0x0A: "LD     V{:1X}, K",
        0x15: "LD DT, V{:1X}",
        0x18: "LD ST, V{:1X}",
        0x1e: "ADD I, V{:1X}",
        0x29: "LD  F, V{:1X}",
        0x33: "LD  B, V{:1X}",
        0x55: "LD [I], V{:1X}",
        0x65: "LD     V{:1X}, [I]",
        0x75: "LD  R, V{:1X}",
        0x85: "LD     V{:1X}, R",
    }

    def __x(self, op):
        return "{:6s} V{:1X}".format(op, self.x)
    def __x8(self, op):
        return "{:6s} V{:1X}, 0x{:02X}".format(op, self.x, self.b)
    def __xy(self, op):
        return "{:6s} V{:1X}, V{:1X}".format(op, self.x, self.y)
    def __xyn(self, op):
        return "{:6s} V{:1X}, V{:1X}, 0x{:1X}".format(op, self.x, self.y, self.n)
    def __imm12(self, op):
        return "{:6s} 0x{:04X}".format(op, self.imm12)

    def __repr__(self):
        if self.op == 0:
            return self.sys.get(self.b, "")
        if self.op == 1:
            return self.__imm12("JMP")
        if self.op == 2:
            return self.__imm12("CALL")
        if self.op == 3:
            return self.__xy("SE")
        if self.op == 4:
            return self.__xy("SNE")
        if self.op == 5:
            return self.__x8("SE")
        if self.op == 6:
            return self.__x8("LD")
        if self.op == 6:
            return self.__x8("ADD")
        if self.op == 8:
            return self.__xy(self.aluop.get(self.n, "??"))
        if self.op == 9:
            return self.__x8("SNE")
        if self.op == 0xA:
            return self.__imm12("LD I,")
        if self.op == 0xB:
            return self.__imm12("JP V0,")
        if self.op == 0xC:
            return self.__x8("RND")
        if self.op == 0xD:
            return self.__xyn("DRW")
        if self.op == 0xE:
            if self.b == 0x9e:
                return "SKP   V{:1X}".format(self.x)
            if self.b == 0xa1:
                return "SKNP  V{:1X}".format(self.x)
        if self.op == 0xF:
            return self.fop.get(self.b, "").format(self.x)
        return ""


