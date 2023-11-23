package serial

import (
	"bufio"
	"fmt"
	"os"
	"strings"
	"time"

	"github.com/tarm/serial"
)

type Serial struct {
	Port              *serial.Port
	Port_Path         string `json:"port_path"`
	IsConnected       bool   `json:"is_connected"`
	DoHeartbeat       bool
	HeartbeatInterval time.Duration
	Buffer            []SenderMessage
}

func NewSerial(comPort string) *Serial {
	s := &Serial{
		Port_Path:         comPort,
		HeartbeatInterval: 2 * time.Second,
		DoHeartbeat:       true,
	}
	return s
}

func (s *Serial) checkConnection() {
	for {
		if s.IsConnected {
			if s.HeartbeatInterval > 0 && s.DoHeartbeat {
				s.sendHeartbeat()
			}
		} else {
			s.tryConnect()
		}
	}
}

func (s *Serial) sendHeartbeat() {
	s.Write("EH")
	time.Sleep(time.Duration(s.HeartbeatInterval))
}

func (s *Serial) tryConnect() {
	c := &serial.Config{Name: s.Port_Path, Baud: 115200}
	p, err := serial.OpenPort(c)
	if err == nil {
		fmt.Printf("is now connected :)\n")
		s.IsConnected = true
		s.Port = p
	} else {
		fmt.Printf("is not connected :(\n")
		s.IsConnected = false
		time.Sleep(1 * time.Second)
	}
}

func (s *Serial) StartTerminal() {
	s.InitiateConnectionChecking()

	fmt.Printf("Waiting for connection on port %v\n", s.Port_Path)
	for !s.IsConnected {
		time.Sleep(s.HeartbeatInterval)
	}
	fmt.Println("Connected!")

	go func() {
		for {
			msg, err := s.Read()
			if err == nil {
				fmt.Println("DEVICE: " + msg)
			} else {
				fmt.Printf("Failed to read from device %s: %v\n", s.Port_Path, err)
			}
		}
	}()

	scanner := bufio.NewScanner(os.Stdin)
	for {
		fmt.Print("USER: ")
		scanner.Scan()
		userMsg := scanner.Text()

		err := s.Write(userMsg)
		if err != nil {
			fmt.Println("Error sending message:", err)
		}
	}
}

func (s *Serial) Read() (string, error) {
	reader := bufio.NewReader(s.Port)
	line, err := reader.ReadString('\n')
	if err != nil {
		return "", err
	}
	return strings.TrimSpace(line), nil
}

func (s *Serial) Write(message string) error {
	_, err := s.Port.Write([]byte(message + "\n"))
	return err
}

func (s *Serial) InitiateConnectionChecking() {
	go s.checkConnection()
}
