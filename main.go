package main

import (
	"DartMonkeyProject/app"
	"DartMonkeyProject/config"
	"DartMonkeyProject/serial"
	"fmt"
	"os"
)

func StartServer(config config.Config) {
	server := app.NewServer(config)
	server.Serial = serial.NewSerial(config.ComPort)
	for _, cam := range config.Webcams {
		cam.InitWebcam()
	}
	go server.ServeHTML()
}

func StartSerial(config config.Config) {
	serial := serial.NewSerial(config.ComPort)
	go serial.StartTerminal()
}

func StartAll(config config.Config) {
	server := app.NewServer(config)
	server.Serial = serial.NewSerial(config.ComPort)
	for name, cam := range config.Webcams {
		cam.InitWebcam()
		server.Cameras[name] = &cam
	}
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
