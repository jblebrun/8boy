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
		fmt.Printf("    0x%02X, 0x%02X,   //0x%04X\n", inst[0], inst[1], pc)
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
