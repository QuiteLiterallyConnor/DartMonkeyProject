package main

import (
	"DartMonkeyProject/app"
	"DartMonkeyProject/config"
	"DartMonkeyProject/serial"
	"fmt"
	"os"
)

func StartServer(config config.Config) {
	fmt.Printf("Called StartServer, getting NewServer\n")
	server := app.NewServer(config)
	fmt.Printf("Called NewServer, getting NewSerial\n")
	serial := serial.NewSerial(config.ComPort)
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

func StartSerial(config config.Config) {
	serial := serial.NewSerial(config.ComPort)
	go serial.StartTerminal()
}

func StartAll(config config.Config) {

	fmt.Printf("Called StartAll, getting NewServer\n")
	server := app.NewServer(config)
	fmt.Printf("Called NewServer, getting NewSerial\n")
	serial := serial.NewSerial(config.ComPort)
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
	var config config.Config
	if err := config.ReadConfig(); err != nil {
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
		StartServer(config)
	case "all":
		StartAll(config)
	default:
		fmt.Println("Invalid argument. Please specify 'serial', 'serve', or 'all'.")
	}

	select {}

}
