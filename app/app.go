package app

import (
	"bufio"
	"context"
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"os"
	"regexp"
	"strconv"
	"strings"
	"sync"
	"time"

	"gorm.io/driver/mysql"
	"gorm.io/gorm"

	"github.com/gin-gonic/gin"
	"github.com/gorilla/websocket"
	"golang.ngrok.com/ngrok"
	"golang.ngrok.com/ngrok/config"

	"DartMonkeyProject/common"
	cnfg "DartMonkeyProject/config"
	_ "DartMonkeyProject/db"
	"DartMonkeyProject/serial"
	"DartMonkeyProject/webcam"
)

type Server struct {
	Serial              *serial.Serial `json:"serial"`
	Cameras             map[string]*webcam.Webcam
	Config              cnfg.Config `json:"config"`
	Connections         map[*websocket.Conn]struct{}
	Mu                  sync.Mutex
	System_States       map[string]int `json:"system_states"`
	Upgrader            websocket.Upgrader
	TokenUsage          map[string]time.Time
	WebSocketStartTimes map[*websocket.Conn]time.Time
}

func NewServer(config cnfg.Config) *Server {
	serial := serial.Serial{}
	cameras := make(map[string]*webcam.Webcam)
	var upgrader = websocket.Upgrader{
		CheckOrigin: func(r *http.Request) bool {
			return true
		},
	}

	s := &Server{
		Serial:              &serial,
		Cameras:             cameras,
		Config:              config,
		Connections:         make(map[*websocket.Conn]struct{}),
		System_States:       make(map[string]int),
		Upgrader:            upgrader,
		TokenUsage:          make(map[string]time.Time),
		WebSocketStartTimes: make(map[*websocket.Conn]time.Time),
	}

	return s
}

func (s *Server) closeExpiredWebSockets() {
	ticker := time.NewTicker(5 * time.Second) // Check every 5 seconds
	for {
		select {
		case <-ticker.C:
			s.Mu.Lock()
			for conn, startTime := range s.WebSocketStartTimes {
				if time.Since(startTime) > 30*time.Second {
					conn.Close() // Close the WebSocket connection
					delete(s.WebSocketStartTimes, conn)
				}
			}
			s.Mu.Unlock()
		}
	}
}

func (s *Server) handleWebSocket(c *gin.Context) {
	conn, err := s.upgradeConnection(c)
	if err != nil {
		fmt.Println("Upgrade error:", err)
		return
	}

	s.Mu.Lock()
	s.WebSocketStartTimes[conn] = time.Now()
	s.Mu.Unlock()

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
	data := common.SenderMessage{
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
		s.Serial.Buffer = append(s.Serial.Buffer, common.SenderMessage{"DEVICE", line})
		if strings.Contains(line, "%%%") {
			s.updateStoredSystemState(line)
			s.sendMessageToAllClientConnections("DEVICE", line)
		}

	}
}

func (s *Server) SystemInfo() common.SystemInfo {
	return common.SystemInfo{
		SystemStates:      s.System_States,
		PortPath:          s.Serial.Port_Path,
		IsConnected:       s.Serial.IsConnected,
		HeartbeatInterval: s.Serial.HeartbeatInterval/time.Second - 1,
		SerialBuffer:      s.Serial.Buffer,
		Config:            s.Config,
	}
}

func (s *Server) StartNgrokTunnel(r *gin.Engine) {
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

func (s *Server) AuthRequired() gin.HandlerFunc {
	return func(c *gin.Context) {
		value, err := c.Cookie("auth")
		if err != nil || value != "true" {
			c.JSON(http.StatusUnauthorized, gin.H{"error": "Unauthorized"})
			c.Abort()
			return
		}

		// Check if token is expired
		token := c.GetHeader("X-Auth-Token") // Assuming token is sent in header
		if !s.isTokenUnused(token) {
			c.JSON(http.StatusUnauthorized, gin.H{"error": "Token expired"})
			c.Abort()
			return
		}

		c.Next()
	}
}

func (s *Server) isTokenUnused(token string) bool {
	if usageTime, exists := s.TokenUsage[token]; exists {
		return time.Since(usageTime) < 30*time.Second
	}
	return true
}

func (s *Server) ServeHTML() {
	r := gin.Default()
	r.ForwardedByClientIP = true
	r.Static("public", "./public")
	r.LoadHTMLGlob("./public/*.html")

	s.Serial.InitiateConnectionChecking()

	r.GET("/", func(c *gin.Context) {
		c.HTML(http.StatusOK, "index.html", nil)
	})

	r.GET("/data", func(c *gin.Context) {
		c.JSON(http.StatusOK, s.SystemInfo())
	})

	r.GET("/controller", s.AuthRequired(), func(c *gin.Context) {
		c.HTML(http.StatusOK, "controller.html", nil)
	})

	r.GET("/controller/ws", s.AuthRequired(), func(c *gin.Context) {
		s.handleWebSocket(c)
	})

	go s.closeExpiredWebSockets()

	r.GET("/controller/stream/main", s.Cameras["Targetting_Main"].Stream)
	r.GET("/controller/stream/secondary", s.Cameras["Secondary"].Stream)

	// r.GET("/controller/stream", s.AuthRequired(), func(c *gin.Context) {
	// 	deviceName := c.Query("d")
	// 	if deviceName == "" {
	// 		c.JSON(http.StatusBadRequest, gin.H{"error": "Device name is required"})
	// 		return
	// 	}
	// 	webcam, ok := s.Cameras[deviceName]
	// 	if !ok {
	// 		c.JSON(http.StatusNotFound, gin.H{"error": "Device not found"})
	// 		return
	// 	}

	// 	webcam.Stream(c)
	// })

	r.POST("/submit-token", func(c *gin.Context) {
		token := c.PostForm("token")

		if isValidToken(token) && s.isTokenUnused(token) {
			s.TokenUsage[token] = time.Now()
			c.SetCookie("auth", "true", 3600, "/", "", false, true)
			c.Redirect(http.StatusFound, "/controller")
			useToken(token)
		} else {
			c.JSON(http.StatusUnauthorized, gin.H{"error": "Invalid or expired token"})
		}
	})

	fmt.Printf("Server %s started at http://localhost:%s\n", s.Config.Name, s.Config.ServerPort)
	// r.Run(fmt.Sprintf(":%s", s.Config.ServerPort))

	s.StartNgrokTunnel(r)
}

type Token struct {
	ID              uint      `gorm:"primaryKey;autoIncrement"`
	Created_Time    time.Time `json:"created_time" gorm:"created_time not null"`
	Used_Time       time.Time `json:"used_time" gorm:"used_time not null"`
	TokenID         string    `json:"token_id" gorm:"token_id not null"`
	IsUsed          bool      `json:"is_used" gorm:"is_used not null"`
	SessionDuration int64     `json:"session_duration" gorm:"session_duration not null"`
}

func useToken(token string) {
	db := connectDb("tokens")

	// Prepare the update data
	updateData := map[string]interface{}{
		"is_used":   true,
		"used_time": time.Now(),
	}

	// Update the token in the database
	result := db.Model(&Token{}).Where("token_id = ?", token).Updates(updateData)
	if result.Error != nil {
		log.Fatal(result.Error)
	}
	log.Printf("Token '%s' marked as used.\n", token)
}

func isValidToken(token string) bool {
	db := connectDb("tokens")

	var count int64
	db.Model(&Token{}).Where("token_id = ? AND is_used = false", token).Count(&count)

	return count > 0
}

func connectDb(dbName string) *gorm.DB {
	user := os.Getenv("SQL_USER")
	pass := os.Getenv("SQL_PASSWORD")

	dsn := fmt.Sprintf("%s:%s@tcp(127.0.0.1:3306)/%s?charset=utf8mb4&parseTime=True&loc=Local", user, pass, dbName)

	db, err := gorm.Open(mysql.Open(dsn), &gorm.Config{})
	if err != nil {
		log.Fatal(err)
	}

	db.AutoMigrate(&Token{})

	return db
}

func removeToken(tokenID string) {
	db := connectDb("tokens")
	result := db.Where("token_id = ?", tokenID).Delete(&Token{})
	if result.Error != nil {
		log.Fatal(result.Error)
	}
	log.Printf("Tokens with TokenID '%s' removed from the database.\n", tokenID)
}

func getStoredTokens(db *gorm.DB) (t []string) {
	var tokens []Token
	result := db.Find(&tokens)
	if result.Error != nil {
		log.Fatal(result.Error)
	}

	for _, token := range tokens {
		t = append(t, token.TokenID)
	}

	return
}
