package main

import (
	"bufio"
	"fmt"
	"io"
	"log"
	"os"

	"github.com/jacobsa/go-serial/serial"
)

func main() {
	options := serial.OpenOptions{
		PortName:        "COM3",
		BaudRate:        115200, // Adjust to match the baud rate of your device
		DataBits:        8,
		StopBits:        1,
		MinimumReadSize: 4,
	}

	port, err := serial.Open(options)
	if err != nil {
		log.Fatalf("serial.Open: %v", err)
	}
	defer port.Close()

	go func() { // Concurrent go routine to read from the serial port
		buf := make([]byte, 128)
		for {
			n, err := port.Read(buf)
			if err != nil {
				if err == io.EOF {
					fmt.Println("Port closed")
					return
				}
				log.Fatalf("port.Read: %v", err)
			}
			fmt.Print(string(buf[:n]))
		}
	}()

	reader := bufio.NewReader(os.Stdin)
	for {
		fmt.Print("Enter message to send: ")
		text, _ := reader.ReadString('\n')
		_, err = port.Write([]byte(text))
		if err != nil {
			log.Fatalf("port.Write: %v", err)
		}
	}
}
