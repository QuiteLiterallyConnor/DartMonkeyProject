package config

import (
	"encoding/json"
	"os"
)

type Config struct {
	Name       string            `json:"name"`
	ComPort    string            `json:"serial_path"`
	Webcams    map[string]Webcam `json:"webcams"`
	CameraPort string            `json:"camera_port"`
	ServerPort string            `json:"server_port"`
}

func readConfig() (Config, error) {
	file, err := os.Open("config_linux.json")
	if err != nil {
		return Config{}, err
	}
	defer file.Close()

	config := Config{}
	decoder := json.NewDecoder(file)
	err = decoder.Decode(&config)
	if err != nil {
		return Config{}, err
	}

	return config, nil
}
