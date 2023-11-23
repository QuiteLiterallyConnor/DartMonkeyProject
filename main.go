package main

import (
	"fmt"
	"os"
)

func StartServer(config Config) {
	fmt.Printf("Called StartServer, getting NewServer\n")
	server := NewServer(config)
	fmt.Printf("Called NewServer, getting NewSerial\n")
	serial := NewSerial(config.ComPort)
	fmt.Printf("Called NewSerial, setting server serial to NewSerial\n")

	server.Serial = serial

	fmt.Printf("Init webcams\n")
	for name, cam := range config.Webcams {
		fmt.Printf("Initting camera: %v\n", name)
		cam.InitWebcam()
	}

	fmt.Printf("Calling \"go server.ServeHTML()\"\n")
	go server.ServeHTML()
}

func StartSerial(config Config) {
	serial := NewSerial(config.ComPort)
	go serial.StartTerminal()
}

func StartAll(config Config) {

	fmt.Printf("Called StartAll, getting NewServer\n")
	server := NewServer(config)
	fmt.Printf("Called NewServer, getting NewSerial\n")
	serial := NewSerial(config.ComPort)
	fmt.Printf("Called NewSerial, setting server serial to NewSerial\n")

	server.Serial = serial

	fmt.Printf("Init webcams\n")
	for name, cam := range config.Webcams {
		cam.InitWebcam()
		server.Cameras[name] = &cam
	}

	fmt.Printf("Calling \"go server.ServeHTML()\"\n")
	go server.ServeHTML()
}

func main() {
	config, err := readConfig()
	if err != nil {
		fmt.Println("Error reading config:", err)
		return
	}

	args := os.Args[1:]
	if len(args) < 1 {
		fmt.Printf("No arguments provided. Please specify 'serial', 'serve', or 'all'.\nExample: \"go run all\"\n")
		return
	}

	fmt.Printf("args: %+v\n", args)

	switch args[0] {
	case "serial":
		StartSerial(config)
	case "serve":
		NewServer(config)
	case "all":
		StartAll(config)
	default:
		fmt.Println("Invalid argument. Please specify 'serial', 'serve', or 'all'.")
	}

	select {}

}
