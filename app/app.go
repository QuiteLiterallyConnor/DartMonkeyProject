package app

import (
	"bufio"
	"context"
	"encoding/json"
	"fmt"
	"net"
	"net/http"
	"os"
	"regexp"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/gin-gonic/gin"
	"github.com/gorilla/websocket"
	"golang.ngrok.com/ngrok"
	"golang.ngrok.com/ngrok/config"

	"DartMonkeyProject/config"
	"DartMonkeyProject/serial"
	"DartMonkeyProject/webcam"
)

type Server struct {
	IPv4          string         `json:"ipv4"`
	Serial        *serial.Serial `json:"serial"`
	Cameras       map[string]*webcam.Webcam
	Config        config.Config `json:"config"`
	Connections   map[*websocket.Conn]struct{}
	Mu            sync.Mutex
	System_States map[string]int `json:"system_states"`
	Upgrader      websocket.Upgrader
}


func NewServer(config config.Config) *Server {
	serial := serial.Serial{}
	cameras := make(map[string]*webcam.Webcam)
	var upgrader = websocket.Upgrader{
		CheckOrigin: func(r *http.Request) bool {
			return true
		},
	}

	s := &Server{
		Serial:        &serial,
		Cameras:       cameras,
		Config:        config,
		Connections:   make(map[*websocket.Conn]struct{}),
		System_States: make(map[string]int),
		Upgrader:      upgrader,
	}

	s.IPv4 = s.getLocalIP()

	return s
}

func (s *Server) getLocalIP() string {
	addrs, err := net.InterfaceAddrs()
	if err != nil {
		fmt.Printf("ERR: Failed to get local server ip address\n")
		return "Unknown"
	}
	for _, addr := range addrs {
		ip, _, err := net.ParseCIDR(addr.String())
		if err != nil {
			continue
		}
		ipv4 := ip.To4()
		if ipv4 != nil && !ipv4.IsLoopback() && !ipv4.IsGlobalUnicast() && !ipv4.IsUnspecified() {
			return ipv4.String()
		}
	}
	return "Unknown"
}

func (s *Server) handleWebSocket(c *gin.Context) {
	conn, err := s.upgradeConnection(c)
	if err != nil {
		fmt.Println("Upgrade error:", err)
		return
	}
	defer func() {
		s.Mu.Lock()
		delete(s.Connections, conn)
		s.Mu.Unlock()
		conn.Close()
	}()

	s.Mu.Lock()
	s.Connections[conn] = struct{}{}
	s.Mu.Unlock()

	s.listenToFrontend(conn)

	s.listenToArduino(conn)
}

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true
	},
}

func (s *Server) upgradeConnection(c *gin.Context) (*websocket.Conn, error) {
	return upgrader.Upgrade(c.Writer, c.Request, nil)
}

func (s *Server) updateVariable(key, value string) {
	switch key {
	case "HEARTBEAT_INTERVAL":
		i, err := strconv.Atoi(value)
		if err != nil {
			fmt.Println("Error parsing HEARTBEAT_INTERVAL value:", err)
			return
		}

		if i > 0 {
			s.Serial.HeartbeatInterval = time.Duration(i+1) * time.Second
		} else {
			s.Serial.DoHeartbeat = false
		}

	// Add more cases as required
	default:
		fmt.Println("Unknown key:", key)
	}
}

func (s *Server) updateSystemVariable(command string) {
	pattern := `%%%_SERVER:(\w+):([-+]?\w+)`
	re := regexp.MustCompile(pattern)
	matches := re.FindStringSubmatch(command)

	if len(matches) == 3 {
		key := matches[1]
		value := matches[2]
		s.updateVariable(key, value)
	} else {
		fmt.Println("Invalid command format")
	}
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

				fmt.Printf("USER: %v\n", message)

				if strings.HasPrefix(message, "%%%_SERVER") {
					s.updateSystemVariable(message)
					s.sendMessageToAllClientConnections("SERVER", fmt.Sprintf("ACK:%v", strings.ReplaceAll(message, "%%%_SERVER:", "")))
				} else {
					s.Serial.Write(message)
					s.sendMessageToAllClientConnections("USER", message)
				}
			}
		}
	}()
}

func (s *Server) sendMessageToClientConnection(conn *websocket.Conn, sender, message string) error {
	// gfmt.Printf("SENDING MESSAGE FROM %v to FRONTEND: %v\n", sender, message)
	return conn.WriteMessage(websocket.TextMessage, s.senderMessageToJsonBytes(sender, message))
}

func (s *Server) senderMessageToJsonBytes(sender, message string) []byte {
	data := SenderMessage{
		Sender:  sender,
		Message: message,
	}
	dataStr, _ := json.Marshal(data)
	return dataStr
}

func (s *Server) sendMessageToAllClientConnections(sender, message string) {
	s.Mu.Lock()
	for conn := range s.Connections {
		s.sendMessageToClientConnection(conn, sender, message)
	}
	s.Mu.Unlock()
}

func (s *Server) updateStoredSystemState(input string) bool {
	pattern := `%%%_(\w+):(-?\d+)`
	re := regexp.MustCompile(pattern)
	matches := re.FindStringSubmatch(input)
	var isUpdated bool

	if len(matches) == 3 {
		key := matches[1]
		value_str := matches[2]
		value, err := strconv.Atoi(value_str)
		if err != nil {
			return isUpdated
		}
		if existingValue, ok := s.System_States[key]; ok {
			if value != existingValue {
				isUpdated = true
				s.System_States[key] = value
			} else {
			}
		} else {
			s.System_States[key] = value
		}
	} else {
		// Handle other commands, ie %%%_ACK
	}

	return isUpdated
}

func (s *Server) listenToArduino(conn *websocket.Conn) {
	if s.Serial == nil || s.Serial.Port == nil {
		fmt.Println("Serial port is not initialized or connected")
		return
	}

	reader := bufio.NewReader(s.Serial.Port)

	for s.Serial.IsConnected {
		line, err := reader.ReadString('\n') // Assuming '\n' as the delimiter. Adjust as needed.
		if err != nil {
			continue
		}
		line = strings.TrimSpace(line)

		// Using regular expressions to replace unwanted number of percent signs with desired "%%%_"
		re := regexp.MustCompile(`%+_`)
		line = re.ReplaceAllString(line, "%%%_")

		// fmt.Printf("DEVICE: %v\n", line)
		s.Serial.Buffer = append(s.Serial.Buffer, SenderMessage{"DEVICE", line})
		if strings.Contains(line, "%%%") {
			s.updateStoredSystemState(line)
			s.sendMessageToAllClientConnections("DEVICE", line)
		}

	}
}

func (s *Server) SystemInfo() SystemInfo {
	return SystemInfo{
		IPv4:              s.IPv4,
		SystemStates:      s.System_States,
		PortPath:          s.Serial.Port_Path,
		IsConnected:       s.Serial.IsConnected,
		HeartbeatInterval: s.Serial.HeartbeatInterval/time.Second - 1,
		SerialBuffer:      s.Serial.Buffer,
		Config:            s.Config,
	}
}

func (s *Server) ServeHTML() {
	r := gin.Default()
	r.ForwardedByClientIP = true
	r.Static("/public", "./public")
	r.LoadHTMLGlob("./public/*.html")

	s.Serial.InitiateConnectionChecking()

	r.GET("/data", func(c *gin.Context) {
		c.JSON(http.StatusOK, s.SystemInfo())
	})

	r.GET("/controller", func(c *gin.Context) {
		c.HTML(http.StatusOK, "controller.html", nil)
	})

	r.GET("/controller/ws", func(c *gin.Context) {
		s.handleWebSocket(c)
	})

	r.GET("/controller/stream", func(c *gin.Context) {
		deviceName := c.Query("d")
		if deviceName == "" {
			c.JSON(http.StatusBadRequest, gin.H{"error": "Device name is required"})
			return
		}

		fmt.Printf("Webcams: %+v\n", s.Cameras)

		webcam, ok := s.Cameras[deviceName]

		fmt.Printf("webcam: %+v\n", webcam)

		if !ok {
			c.JSON(http.StatusNotFound, gin.H{"error": "Device not found"})
			return
		}

		webcam.Stream(c)
	})

	fmt.Printf("Server %s started at http://localhost:%s\n", s.Config.Name, s.Config.ServerPort)
	// r.Run(fmt.Sprintf(":%s", s.Config.ServerPort))

	token := os.Getenv("NGROK_AUTHTOKEN")

	ctx := context.Background()
	listener, err := ngrok.Listen(ctx,
		config.HTTPEndpoint(
			config.WithDomain("app.connorisseur.com"),
		),
		ngrok.WithAuthtoken(token),
	)
	if err != nil {
		fmt.Println("ngrok listen error:", err)
		return
	}

	fmt.Println("ngrok tunnel created:", listener.Addr().String())

	if err != nil {
		fmt.Println("ngrok listen error:", err)
		return
	}

	fmt.Printf("Server %s started at %s\n", s.Config.Name, listener.Addr().String())
	if err := http.Serve(listener, r); err != nil {
		fmt.Println("Server error:", err)
	}
}
