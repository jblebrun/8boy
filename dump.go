package main

import (
	"encoding/binary"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
)

func xy4(op string, inst uint16) string {
	return fmt.Sprintf("%s V%d, V%d (%d)", op, inst>>8&0xF, inst>>4&0xF, inst&0xF)
}

func xy(op string, inst uint16) string {
	return fmt.Sprintf("%s V%d, V%d", op, inst>>8&0xF, inst>>4&0xF)
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

func dump(dir, name string) error {
	fmt.Printf("const uint8_t %s[] PROGMEM = {\n", name)
	defer fmt.Println("};\n\n")
	f, err := os.Open(filepath.Join(dir, name))
	if err != nil {
		return err
	}

	pc := 0x200
	for {
		var inst [2]byte
		if err := binary.Read(f, binary.BigEndian, &inst); err != nil {
			if err == io.EOF {
				return nil
			}
			return err
		}
		fmt.Printf("    0x%02X, 0x%02X,   //0x%04X ", inst[0], inst[1], pc)
		inst16 := uint16(inst[1])
		inst16 |= uint16(inst[0]) << 8
		fmt.Println(disasm(inst16))
		pc += 2
	}
	return nil
}

func dumpAllRoms(dir string) {
	files, err := ioutil.ReadDir(dir)
	if err != nil {
		log.Fatal(err)
	}

	for _, file := range files {
		if err := dump(dir, file.Name()); err != nil {
			log.Println(err)
		}
	}

	fmt.Println("const uint8_t * const programs[] PROGMEM = {")
	for _, file := range files {
		fmt.Println("    " + file.Name() + ",")
	}
	fmt.Println("};")
}

func main() {
	dumpAllRoms(os.Args[1])
}
