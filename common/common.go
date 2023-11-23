package common

import (
	"DartMonkeyProject/config"
	"time"
)

type SystemInfo struct {
	IPv4              string          `json:"ipv4"`
	SystemStates      map[string]int  `json:"system_states"`
	PortPath          string          `json:"port_path"`
	IsConnected       bool            `json:"is_connected"`
	DoHeartbeat       bool            `json:"do_heartbeat"`
	HeartbeatInterval time.Duration   `json:"heartbeat_interval"`
	SerialBuffer      []SenderMessage `json:"serial_buffer"`
	Config            config.Config   `json:"config"`
}

type SenderMessage struct {
	Sender  string `json:"sender"`
	Message string `json:"message"`
}
