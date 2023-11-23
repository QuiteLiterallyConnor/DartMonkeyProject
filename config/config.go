package config

import (
	"DartMonkeyProject/webcam"
	"encoding/json"
	"os"
)

type Config struct {
	Name       string                   `json:"name"`
	ComPort    string                   `json:"serial_path"`
	Webcams    map[string]webcam.Webcam `json:"webcams"`
	CameraPort string                   `json:"camera_port"`
	ServerPort string                   `json:"server_port"`
}

func (c *Config) ReadConfig() error {
	file, err := os.Open("config_linux.json")
	if err != nil {
		return err
	}
	defer file.Close()

	decoder := json.NewDecoder(file)
	err = decoder.Decode(&c)
	if err != nil {
		return err
	}

	return nil
}
