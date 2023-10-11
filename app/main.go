package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"log"
	"net"
	"net/http"
	"os"
	"os/exec"
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
)

type GStreamerServer struct {
	compressionLevel int
	maxFiles         int
	bufferDuration   int
	segmentDuration  int
	gstPath          string
}

func (g *GStreamerServer) init() {
	g.compressionLevel = 50
	g.segmentDuration = 1
	g.bufferDuration = 60
	g.maxFiles = int(g.bufferDuration / g.segmentDuration)
	g.gstPath = "E:\\gstreamer\\1.0\\msvc_x86_64\\bin\\gst-launch-1.0.exe"
}

func (g *GStreamerServer) StartWebcamStream() {
	go g.CallStreamerExecutable()
	go g.CleanUpHLSFiles()
}

func (g *GStreamerServer) CallStreamerExecutable() {
	quantizerValue := 20 + (g.compressionLevel * 30 / 100)

	cmd := exec.Command(
		g.gstPath,
		"mfvideosrc",
		"!",
		"videoconvert",
		"!",
		"tee", "name=t",
		"t.",
		"!",
		"queue",
		"!",
		"videoconvert",
		"!",
		"x264enc", "bitrate=500", "quantizer="+strconv.Itoa(quantizerValue), "speed-preset=ultrafast", "tune=zerolatency",
		"!",
		"mpegtsmux",
		"!",
		"hlssink", "location=./hls/segment%05d.ts", "playlist-location=./hls/playlist.m3u8", "max-files="+strconv.Itoa(g.maxFiles), "target-duration="+strconv.Itoa(g.segmentDuration),
	)

	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr

	if err := cmd.Start(); err != nil {
		log.Fatal(err)
	}
}

func (g *GStreamerServer) CleanUpHLSFiles() {
	for {
		time.Sleep(time.Second * time.Duration(g.bufferDuration))
		files, _ := os.ReadDir("./hls")
		if len(files) > g.maxFiles {
			for i := 0; i < len(files)-g.maxFiles; i++ {
				os.Remove("./hls/" + files[i].Name())
			}
		}
	}
}

type Config struct {
	Name       string `json:"name"`
	ComPort    string `json:"com_port"`
	CameraPort string `json:"camera_port"`
	ServerPort string `json:"server_port"`
}

func readConfig() ([]Config, error) {
	file, err := os.Open("config_windows.json")
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

const HEARTBEAT_INTERVAL = 2 * time.Second

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
	c.Cmd.Stdout = os.Stdout
	c.Cmd.Stderr = os.Stderr
	return c.Cmd.Start()
}

func (c *CameraServer) Stop() error {
	if c.Cmd != nil && c.Cmd.Process != nil {
		err := c.Cmd.Process.Kill()
		if err != nil {
			return err
		}
		c.Cmd.Wait()
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
	time.Sleep(1 * time.Second)
	return c.Start()
}

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true
	},
}

type Server struct {
	Serial      *Serial
	Camera      *CameraServer
	Config      Config
	Connections map[*websocket.Conn]struct{}
	Mu          sync.Mutex
}

func NewServer(config Config) *Server {
	serial := NewSerial(config.ComPort)
	camera := NewCameraServer(config.CameraPort)
	return &Server{
		Serial:      serial,
		Camera:      camera,
		Config:      config,
		Connections: make(map[*websocket.Conn]struct{}),
	}
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

var SYSTEM_STATES = make(map[string]int)

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
		if existingValue, ok := SYSTEM_STATES[key]; ok {
			if value != existingValue {
				isUpdated = true
				SYSTEM_STATES[key] = value
			} else {
			}
		} else {
			SYSTEM_STATES[key] = value
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
		if strings.HasPrefix(line, "%%%") {

			if s.updateStoredSystemState(line) || line == "%%%_HEARTBEAT" {
				s.Mu.Lock()
				for conn := range s.Connections {
					if err := conn.WriteMessage(websocket.TextMessage, []byte(line)); err != nil {
						fmt.Println("WebSocket write error:", err)
						delete(s.Connections, conn)
					}
				}
				s.Mu.Unlock()
			}
		}

	}

}

func (s *Server) ServeHTML() {
	r := gin.Default()
	r.ForwardedByClientIP = true
	r.Static("/public", "./public")
	a := NewAdmin()

	s.Serial.InitiateConnectionChecking()

	r.GET("/admin", func(c *gin.Context) {
		c.File("./public/admin.html")
	})
	r.GET("/admin/ws", a.handleConnections)

	r.GET("/admin/config", func(c *gin.Context) {
		c.JSON(http.StatusOK, s.Config)
	})

	r.GET("/controller", func(c *gin.Context) {
		http.ServeFile(c.Writer, c.Request, "./public/controller.html")
	})

	r.GET("/controller/ws", func(c *gin.Context) {
		s.handleWebSocket(c)
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
