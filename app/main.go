package main

import (
	"bufio"
	"context"
	"encoding/json"
	"fmt"
	"log"
	"mime/multipart"
	"net"
	"net/http"
	"net/textproto"
	"os"
	"regexp"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/gin-gonic/gin"
	"github.com/gorilla/websocket"
	"github.com/mssola/user_agent"
	"github.com/oschwald/geoip2-golang"
	"github.com/tarm/serial"
	"github.com/vladimirvivien/go4vl/device"
	"github.com/vladimirvivien/go4vl/v4l2"
	"golang.ngrok.com/ngrok"
	"golang.ngrok.com/ngrok/config"
)

type Webcam struct {
	camera *device.Device
	frames <-chan []byte
}

func NewWebcam(deviceName string) (*Webcam, error) {
	camera, err := device.Open(
		deviceName,
		device.WithPixFormat(v4l2.PixFormat{PixelFormat: v4l2.PixelFmtMJPEG, Width: 640, Height: 480}),
	)
	if err != nil {
		return nil, err
	}

	if err := camera.Start(context.TODO()); err != nil {
		camera.Close()
		return nil, err
	}

	return &Webcam{
		camera: camera,
		frames: camera.GetOutput(),
	}, nil
}

func (webcam *Webcam) Close() {
	webcam.camera.Close()
}

func (webcam *Webcam) Stream(c *gin.Context) {
	w := c.Writer
	mimeWriter := multipart.NewWriter(w)
	w.Header().Set("Content-Type", fmt.Sprintf("multipart/x-mixed-replace; boundary=%s", mimeWriter.Boundary()))
	partHeader := make(textproto.MIMEHeader)
	partHeader.Add("Content-Type", "image/jpeg")

	for frame := range webcam.frames {
		partWriter, err := mimeWriter.CreatePart(partHeader)
		if err != nil {
			log.Printf("failed to create multi-part writer: %s", err)
			return
		}

		if _, err := partWriter.Write(frame); err != nil {
			log.Printf("failed to write image: %s", err)
		}
	}
}

type Config struct {
	Name       string `json:"name"`
	ComPort    string `json:"com_port"`
	WebcamPort string `json:"webcam_port"`
	CameraPort string `json:"camera_port"`
	ServerPort string `json:"server_port"`
}

func readConfig() ([]Config, error) {
	file, err := os.Open("config_linux.json")
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

type Admin struct {
	clients  map[*websocket.Conn]*http.Request
	upgrader websocket.Upgrader
}

func NewAdmin() *Admin {
	return &Admin{
		clients: make(map[*websocket.Conn]*http.Request),
		upgrader: websocket.Upgrader{
			CheckOrigin: func(r *http.Request) bool {
				return true // Adjust as needed for security
			},
		},
	}
}

type ClientInfo struct {
	Name           string `json:"name"`
	IP             string `json:"ip"`
	Device         string `json:"device"`
	Browser        string `json:"browser"`
	OS             string `json:"os"`
	Country        string `json:"country"`
	City           string `json:"city"`
	Sessions       int    `json:"sessions"`
	WindowLocation string `json:"windowLocation"`
}

func (a *Admin) handleConnections(c *gin.Context) {
	ws, err := a.upgrader.Upgrade(c.Writer, c.Request, nil)
	if err != nil {
		log.Printf("Failed to set websocket upgrade: %+v", err)
		return
	}
	defer ws.Close()

	log.Printf("New WebSocket connection from %s", c.ClientIP())

	a.clients[ws] = c.Request
	a.broadcastConnectedClients()

	for {
		messageType, _, err := ws.ReadMessage()
		if err != nil {
			log.Printf("WebSocket connection from %s closed", c.ClientIP())
			delete(a.clients, ws)
			a.broadcastConnectedClients()
			break
		}
		ws.WriteMessage(messageType, []byte("Your message was received!"))
	}
}

func (a *Admin) broadcastConnectedClients() {
	clientInfos := make(map[string]ClientInfo)

	for _, request := range a.clients {
		validIP, country, city, err := getIPCityCountry(request.RemoteAddr)
		if err != nil {
			log.Printf("Error getting IP info: %v", err)
			continue
		}

		info, exists := clientInfos[validIP]
		if exists {
			info.Sessions++
			clientInfos[validIP] = info
			continue
		}

		ua := user_agent.New(request.Header.Get("User-Agent"))
		browser, _ := ua.Browser()
		device, os := getDeviceOS(ua, request.Header.Get("User-Agent"))

		clientInfos[validIP] = ClientInfo{
			IP:       validIP,
			Device:   device,
			Browser:  browser,
			OS:       os,
			Country:  country,
			City:     city,
			Sessions: 1,
		}
	}

	data := map[string]map[string]ClientInfo{"clients": clientInfos}
	jsonData, err := json.Marshal(data)
	if err != nil {
		log.Println("Error marshaling data:", err)
		return
	}

	for client := range a.clients {
		if err := client.WriteMessage(websocket.TextMessage, jsonData); err != nil {
			log.Printf("WebSocket error: %v", err)
			client.Close()
			delete(a.clients, client)
		}
	}
}

func getDeviceOS(ua *user_agent.UserAgent, userAgentHeader string) (string, string) {
	device := "unknown"
	os := ua.OS()
	if strings.Contains(strings.ToLower(userAgentHeader), "mobile") {
		device = "mobile"
	} else if strings.Contains(strings.ToLower(userAgentHeader), "tablet") {
		device = "tablet"
	} else if strings.Contains(strings.ToLower(userAgentHeader), "windows") {
		device = "desktop"
	}
	return device, os
}

func extractValidIP(raw_ip string) (string, error) {
	ipv4Pattern := `^(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})`
	ipv6Pattern := `^(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:))`

	ipv4Regex := regexp.MustCompile(ipv4Pattern)
	ipv6Regex := regexp.MustCompile(ipv6Pattern)

	if ipv4Match := ipv4Regex.FindString(raw_ip); ipv4Match != "" {
		return ipv4Match, nil
	}

	if ipv6Match := ipv6Regex.FindString(raw_ip); ipv6Match != "" {
		return ipv6Match, nil
	}

	return "", fmt.Errorf("No valid IPv4 or IPv6 found in %s", raw_ip)
}

func getIPCityCountry(rawIP string) (string, string, string, error) {
	if strings.HasPrefix(rawIP, "[::1]") {
		return rawIP, "Server", "Server", nil
	}

	ip, err := extractValidIP(rawIP)
	if err != nil {
		return rawIP, "Unknown", "Unknown", err
	}

	if isLocalIP(ip) {
		fmt.Printf("%s is a local device", ip)
		return ip, "Local Network", "Local Network", nil
	} else {
		fmt.Printf("%s is not a local device", ip)
	}

	db, err := geoip2.Open("GeoLite2-City.mmdb")
	if err != nil {
		log.Println("Error opening GeoIP database:", err)
		return ip, "Unknown", "Unknown", err
	}
	defer db.Close()

	record, err := db.City(net.ParseIP(ip))
	if err != nil {
		log.Println("Error looking up IP:", err)
		return ip, "Unknown", "Unknown", err
	}

	return ip, record.City.Names["en"], record.Country.IsoCode, nil
}

func isLocalIP(ip string) bool {
	localIPv4Ranges := []net.IPNet{
		{IP: net.IPv4(10, 0, 0, 0), Mask: net.CIDRMask(8, 32)},
		{IP: net.IPv4(172, 16, 0, 0), Mask: net.CIDRMask(12, 32)},
		{IP: net.IPv4(192, 168, 0, 0), Mask: net.CIDRMask(16, 32)},
	}

	ipAddress := net.ParseIP(ip)

	if ipAddress.To4() != nil {
		for _, localRange := range localIPv4Ranges {
			if localRange.Contains(ipAddress) {
				return true
			}
		}
	} else {
		if ipAddress.IsLoopback() || ipAddress.IsLinkLocalUnicast() || ipAddress.IsLinkLocalMulticast() {
			return true
		}
	}

	return false
}

type Serial struct {
	Port              *serial.Port
	Port_Path         string `json:"port_path"`
	IsConnected       bool   `json:"is_connected"`
	DoHeartbeat       bool
	HeartbeatInterval time.Duration
	Buffer            []SenderMessage
}

type SenderMessage struct {
	Sender  string `json:"sender"`
	Message string `json:"message"`
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

func (s *Serial) Read() (string, error) {
	reader := bufio.NewReader(s.Port)
	line, err := reader.ReadString('\n')
	if err != nil {
		return "", err
	}
	return strings.TrimSpace(line), nil
}

func (s *Serial) Write(message string) error {
	// fmt.Printf("SENT TO DEVICE: %v\n", message)
	_, err := s.Port.Write([]byte(message + "\n"))
	return err
}

func (s *Serial) InitiateConnectionChecking() {
	go s.checkConnection()
}

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true
	},
}

type Server struct {
	IPv4          string  `json:"ipv4"`
	Serial        *Serial `json:"serial"`
	Camera        *Webcam
	Config        Config `json:"config"`
	Connections   map[*websocket.Conn]struct{}
	Mu            sync.Mutex
	System_States map[string]int `json:"system_states"`
}

func NewServer(config Config) *Server {
	serial := NewSerial(config.ComPort)
	camera, err := NewWebcam(config.WebcamPort)
	if err != nil {
		fmt.Printf("Failed to init webcam with device %v\n", config.WebcamPort)
	}
	return &Server{
		IPv4:          getLocalIP(),
		Serial:        serial,
		Camera:        camera,
		Config:        config,
		Connections:   make(map[*websocket.Conn]struct{}),
		System_States: make(map[string]int),
	}
}

func getLocalIP() string {
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
	return conn.WriteMessage(websocket.TextMessage, senderMessageToJsonBytes(sender, message))
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

func trimMessage(input string) string {
	regex := regexp.MustCompile(`^%+`)
	return regex.ReplaceAllString(input, "")
}

func senderMessageToJsonBytes(sender, message string) []byte {
	data := SenderMessage{
		Sender:  sender,
		Message: message,
	}
	dataStr, _ := json.Marshal(data)
	return dataStr
}

type SystemInfo struct {
	IPv4              string          `json:"ipv4"`
	SystemStates      map[string]int  `json:"system_states"`
	PortPath          string          `json:"port_path"`
	IsConnected       bool            `json:"is_connected"`
	DoHeartbeat       bool            `json:"do_heartbeat"`
	HeartbeatInterval time.Duration   `json:"heartbeat_interval"`
	SerialBuffer      []SenderMessage `json:"serial_buffer"`
	Config            Config          `json:"config"`
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
	r.Static("/hls", "./hls")
	r.LoadHTMLGlob("./public/*.html")
	a := NewAdmin()

	s.Serial.InitiateConnectionChecking()

	r.GET("/data", func(c *gin.Context) {
		c.JSON(http.StatusOK, s.SystemInfo())
	})

	r.GET("/admin", func(c *gin.Context) {
		c.HTML(http.StatusOK, "admin.html", nil)
	})
	r.GET("/admin/ws", a.handleConnections)

	r.GET("/controller", func(c *gin.Context) {
		c.HTML(http.StatusOK, "controller.html", nil)
	})

	r.GET("/controller/ws", func(c *gin.Context) {
		s.handleWebSocket(c)
	})

	r.GET("/stream", s.Camera.Stream)

	fmt.Printf("Server %s started at http://localhost:%s\n", s.Config.Name, s.Config.ServerPort)
	// r.Run(fmt.Sprintf(":%s", s.Config.ServerPort))

	ctx := context.Background()
	listener, err := ngrok.Listen(ctx,
		config.HTTPEndpoint(
			config.WithDomain("current-ibex-strictly.ngrok-free.app"),
		),
		ngrok.WithAuthtokenFromEnv(),
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

func main() {
	configs, err := readConfig()
	if err != nil {
		fmt.Println("Error reading config:", err)
		return
	}

	for _, config := range configs {
		server := NewServer(config)
		go server.ServeHTML() // Pass the context to ServeHTML
		defer server.Camera.Close()

	}

	select {} // prevent main from exiting
}
