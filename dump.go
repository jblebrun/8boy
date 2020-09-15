package main

import (
	"bufio"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	"strings"
)

func xy4(op string, inst uint16) string {
	return fmt.Sprintf("%s V%X, V%X (%d)", op, inst>>8&0xF, inst>>4&0xF, inst&0xF)
}

func xy(op string, inst uint16) string {
	return fmt.Sprintf("%s V%X, V%d", op, inst>>8&0xF, inst>>4&0xF)
}

func x8(op string, inst uint16) string {
	return fmt.Sprintf("%s V%X 0x%X", op, inst>>8&0xF, inst&0xFF)
}

func imm12(op string, inst uint16) string {
	return fmt.Sprintf("%s 0x%X", op, inst&0xFFF)
}

func x(f string, inst uint16) string {
	return fmt.Sprintf(f, inst>>8&0xF)
}

func disasm(inst uint16) string {
	switch inst >> 12 {
	case 0:
		switch inst & 0xFF {
		case 0xE0:
			return "CLS"
		case 0xEE:
			return "RET"
		case 0xFB:
			return "SCR"
		case 0xFC:
			return "SCL"
		case 0xFD:
			return "HALT"
		case 0xFE:
			return "LO"
		case 0xFF:
			return "HI"
		}
	case 1:
		return imm12("JP", inst)
	case 2:
		return imm12("CALL", inst)
	case 3:
		return x8("SE", inst)
	case 4:
		return x8("SNE", inst)
	case 5:
		return xy("SE", inst)
	case 6:
		return x8("LD", inst)
	case 7:
		return x8("ADD", inst)
	case 8:
		switch inst & 0xF {
		case 0:
			return xy("LD", inst)
		case 1:
			return xy("OR", inst)
		case 2:
			return xy("AND", inst)
		case 3:
			return xy("XOR", inst)
		case 4:
			return xy("ADD", inst)
		case 5:
			return xy("SUB", inst)
		case 6:
			return xy("SHR", inst)
		case 7:
			return xy("SUBN", inst)
		case 0xe:
			return xy("SHL", inst)
		}
	case 9:
		return xy("SNE", inst)
	case 0xA:
		return imm12("LD I,", inst)
	case 0xB:
		return imm12("JP V0,", inst)
	case 0xC:
		return x8("RND", inst)
	case 0xD:
		return xy4("DRW", inst)
	case 0xE:
		switch inst & 0xFF {
		case 0x9E:
			return x("SKP V%d", inst)
		case 0xA1:
			return x("SKNP V%d", inst)
		}
	case 0xF:
		switch inst & 0xFF {
		case 0x07:
			return x("LD V%d, DT", inst)
		case 0x0A:
			return x("LD V%d, K", inst)
		case 0x15:
			return x("LD DT, V%d", inst)
		case 0x18:
			return x("LD ST, V%d", inst)
		case 0x1e:
			return x("ADD I, V%d", inst)
		case 0x29:
			return x("LD F, V%d", inst)
		case 0x33:
			return x("LD B, V%d", inst)
		case 0x55:
			return x("LD [I], V%d", inst)
		case 0x65:
			return x("LD V%d, [I]", inst)

		}
	default:
	}
	return ""
}

func emitgroup(instrs []uint8) {
	fmt.Printf("    ")
	for i := 0; i < len(instrs); i++ {
		fmt.Printf("0x%02X, ", instrs[i])
	}
	fmt.Println("")
}

func readgroup(f *bufio.Reader, pc *int, instrs []uint8) error {
	// Emit up to 8 at a time.
	for i := 0; i < 16; i += 2 {
		n, err := io.ReadFull(f, instrs[i:i+2])
		if err != nil {
			if err == io.EOF || err == io.ErrUnexpectedEOF {
				// emit the rest
				emitgroup(instrs[:i+n])
				return err
			} else {
				return err
			}
		}

		inst16 := uint16(instrs[i+1])
		inst16 |= uint16(instrs[i]) << 8

		instOff := uint16(instrs[i+1]) << 8
		peek, err := f.Peek(1)
		if err == nil && len(peek) > 0 {
			instOff |= uint16(peek[0])
		}
		fmt.Printf("    //0x%04X: %-20s |         0x%04X: %s\n", *pc, disasm(inst16), *pc+1, disasm(instOff))
		*pc += 2
	}
	emitgroup(instrs[:16])
	return nil
}

func dump(dir, filename, codename string) (int, error) {
	fmt.Printf("const uint8_t %s[] PROGMEM = {\n", codename)
	defer fmt.Println("};\n\n")
	f, err := os.Open(filepath.Join(dir, filename))
	if err != nil {
		return 0, err
	}
	defer f.Close()

	r := bufio.NewReader(f)

	pc := 0x200
	for {
		instrs := make([]uint8, 16)
		err := readgroup(r, &pc, instrs)
		if err == io.EOF {
			return pc, nil
		}
		if err == io.ErrUnexpectedEOF {
			return pc, nil
		}
		if err != nil {
			return 0, err
		}
	}
	return pc, nil
}

type programInfo struct {
	keymap     string
	info       string
	shiftquirk bool
}

type program struct {
	name     string
	codename string
	size     int
	super    bool
	info     programInfo
}

const defaultKeymap = "0x58, 0x79, 0x46"

func loadInfo(dir, filename string) *programInfo {
	basename := strings.TrimSuffix(filename, filepath.Ext(filename))
	info, err := ioutil.ReadFile(filepath.Join(dir, basename+".info"))
	if err != nil {
		log.Println("no info for", filename)
		return &programInfo{keymap: defaultKeymap}
	}

	var pgmInfo programInfo
	lines := strings.Split(string(info), "\n")
	for _, line := range lines {
		fields := strings.SplitN(line, "=", 2)
		switch fields[0] {
		case "shiftquirk":
			pgmInfo.shiftquirk = true
		case "info":
			pgmInfo.info = fields[1]
		case "keymap":
			pgmInfo.keymap = fields[1]
		}
	}
	return &pgmInfo
}

func getProgram(dir, filename string) *program {
	ext := filepath.Ext(filename)
	nicename := strings.TrimSuffix(filename, ext)
	codename := strings.Replace(filename, ".", "_", -1)
	size, err := dump(dir, filename, codename)
	if err != nil {
		log.Println(err)
		return nil
	}
	info := loadInfo(dir, nicename)
	pgm := &program{name: nicename, codename: codename, size: size, super: ext == ".schi8", info: *info}
	return pgm
}

func dumpAllRoms(dir string) {
	menufile, err := ioutil.ReadFile(filepath.Join(dir, "menu"))
	if err != nil {
		log.Fatal(err)
	}
	files := strings.Split(string(menufile), "\n")

	fmt.Println(`struct Program {
    char *name;
    uint8_t *code;
    uint16_t size;
    bool super;
    uint8_t *info;
    uint8_t keymap[3];
    bool shiftquirk;
};`)
	var ps []*program
	for _, filename := range files {
		ext := filepath.Ext(filename)
		if ext == ".ch8" || ext == ".sch8" {
			prog := getProgram(dir, filename)
			if prog != nil {
				ps = append(ps, prog)
			}
		}
	}

	fmt.Printf("const uint8_t PROGRAM_COUNT = %d;\n", len(ps))
	for _, p := range ps {
		fmt.Printf("const char name_%s[] PROGMEM = \"%s\";\n", p.codename, p.name)
	}

	for _, p := range ps {
		fmt.Printf("const uint8_t info_%s[] PROGMEM = \"%s\";\n", p.codename, p.info.info)
	}

	fmt.Println("const Program programs[] PROGMEM = {")
	for _, p := range ps {
		superField := "0"
		if p.super {
			superField = "1"
		}
		fmt.Printf(`    (Program){ 
        .name=(char*)name_%s,
        .code=(uint8_t*)%s, 
        .size=%d, 
        .super=%s, 
        .info=(uint8_t*)info_%s, 
		.keymap={%s},
		.shiftquirk=%v,
    },
`, p.codename, p.codename, p.size, superField, p.codename, p.info.keymap, p.info.shiftquirk)
	}
	fmt.Println("};")
}

func main() {
	dumpAllRoms(os.Args[1])
}
