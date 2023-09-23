package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"net/http"
	"os"
	"os/exec"
	"strings"
	"time"

	"github.com/gin-gonic/gin"
	"github.com/gorilla/websocket"
	"github.com/tarm/serial"
)

type Config struct {
	Name       string `json:"name"`
	ComPort    string `json:"com_port"`
	CameraPort string `json:"camera_port"`
	ServerPort string `json:"server_port"`
}

func readConfig() ([]Config, error) {
	file, err := os.Open("config.json")
	if err != nil {
		return nil, err
	}
	defer file.Close()

	configs := []Config{}
	decoder := json.NewDecoder(file)
	err = decoder.Decode(&configs)
	if err != nil {
		return nil, err
	}

	return configs, nil
}

const HEARTBEAT_INTERVAL = 3 * time.Second

type Serial struct {
	Port        *serial.Port
	Port_Path   string
	IsConnected bool
}

func NewSerial(comPort string) *Serial {
	s := &Serial{
		Port_Path: comPort,
	}
	return s
}

func (s *Serial) checkConnection() {
	for {
		if s.IsConnected {
			s.sendHeartbeat()
		} else {
			s.tryConnect()
		}
	}
}

func (s *Serial) sendHeartbeat() {
	s.Port.Write([]byte("H\n"))
	time.Sleep(HEARTBEAT_INTERVAL)
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

type CameraServer struct {
	Port string
	Cmd  *exec.Cmd
}

func NewCameraServer(comPort string) *CameraServer {
	c := &CameraServer{
		Port: comPort,
	}
	return c
}

func (c *CameraServer) Start() error {
	if c.Cmd != nil && c.Cmd.Process != nil {
		return fmt.Errorf("server already running")
	}
	c.Cmd = exec.Command("python", "camera_server.py", c.Port)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	return c.Cmd.Start()
}

func (c *CameraServer) Stop() error {
	if c.Cmd != nil && c.Cmd.Process != nil {
		err := c.Cmd.Process.Kill()
		if err != nil {
			return err
		}
		c.Cmd.Wait() // Ignore the error as it will return a non-nil error since we have killed the process
		c.Cmd = nil
		return nil
	}
	return fmt.Errorf("server not running")
}

func (c *CameraServer) Restart() error {
	err := c.Stop()
	if err != nil {
		return fmt.Errorf("error stopping server: %v", err)
	}
	// Wait for a while to make sure everything is stopped before starting again
	time.Sleep(1 * time.Second)
	return c.Start()
}

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true
	},
}

type Server struct {
	Serial *Serial
	Camera *CameraServer
	Config Config // Store the Config in the Server
}

func NewServer(config Config) *Server {
	serial := NewSerial(config.ComPort)
	camera := NewCameraServer(config.CameraPort)
	return &Server{
		Serial: serial,
		Camera: camera,
		Config: config,
	}
}

func (s *Server) handleWebSocket(c *gin.Context) {
	conn, err := s.upgradeConnection(c)
	if err != nil {
		fmt.Println("Upgrade error:", err)
		return
	}
	defer conn.Close()

	s.notifyInitialConnectionStatus(conn)

	s.listenToFrontend(conn)

	s.listenToArduino(conn)
}

func (s *Server) upgradeConnection(c *gin.Context) (*websocket.Conn, error) {
	return upgrader.Upgrade(c.Writer, c.Request, nil)
}

func (s *Server) notifyInitialConnectionStatus(conn *websocket.Conn) {
	conn.WriteMessage(websocket.TextMessage, []byte(fmt.Sprintf("Connection status: %v", s.Serial.IsConnected)))
}

func (s *Server) listenToFrontend(conn *websocket.Conn) {
	go func() {
		for {
			messageType, p, err := conn.ReadMessage()
			if err != nil {
				fmt.Println("Read error:", err)
				return
			}
			if messageType == websocket.TextMessage {
				message := strings.TrimSpace(string(p))
				s.Serial.Write(message)
			}
		}
	}()
}

func (s *Server) listenToArduino(conn *websocket.Conn) {
	for s.Serial.IsConnected {
		line, err := s.Serial.Read()
		if err != nil {
			fmt.Println("Serial read error:", err)
			continue
		}
		if err := conn.WriteMessage(websocket.TextMessage, []byte(line)); err != nil {
			fmt.Println("Serial write error:", err)
			return
		}
	}
}

func (s *Server) ServeHTML() {
	r := gin.Default()
	r.ForwardedByClientIP = true
	r.Static("/public", "./public")

	s.Serial.InitiateConnectionChecking()

	r.GET("/", func(c *gin.Context) {
		http.ServeFile(c.Writer, c.Request, "./public/index.html")
	})

	r.GET("/ws", func(c *gin.Context) {
		s.handleWebSocket(c)
	})

	r.GET("/config", func(c *gin.Context) {
		c.JSON(http.StatusOK, s.Config)
	})

	fmt.Printf("Server %s started at http://localhost:%s\n", s.Config.Name, s.Config.ServerPort)
	r.Run(":" + s.Config.ServerPort)
}

func main() {
	configs, err := readConfig()
	if err != nil {
		fmt.Println("Error reading config:", err)
		return
	}

	for _, config := range configs {
		server := NewServer(config)
		go server.ServeHTML() // Start each server in its goroutine
		server.Camera.Start()
	}

	select {} // prevent main from exiting
}
