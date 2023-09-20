package main

import (
	"bufio"
	"fmt"
	"net/http"
	"strings"
	"time"

	"github.com/gin-gonic/gin"
	"github.com/gorilla/websocket"
	"github.com/tarm/serial"
)

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true
	},
}

var isConnected = false
var HEARTBEAT_INTERVAL time.Duration = 3

func handleWebSocket(port **serial.Port, c *gin.Context) { // Change port parameter to a double pointer
	conn, err := upgrader.Upgrade(c.Writer, c.Request, nil)
	if err != nil {
		fmt.Println("Upgrade error:", err)
		return
	}
	defer conn.Close()

	// Notify frontend about the initial connection status
	conn.WriteMessage(websocket.TextMessage, []byte(fmt.Sprintf("Connection status: %v", isConnected)))

	// Start a goroutine for listening to messages from the frontend
	go func() {
		for {
			messageType, p, err := conn.ReadMessage()
			if err != nil {
				fmt.Println("Read error:", err)
				return
			}

			message := strings.TrimSpace(string(p))

			if messageType == websocket.TextMessage {
				(*port).Write([]byte(message + "\n"))
			}
		}
	}()

	// Loop for listening to messages from the Arduino if connected
	reader := bufio.NewReader(*port)
	for isConnected {
		line, _ := reader.ReadString('\n')
		if len(line) > 0 {
			err = conn.WriteMessage(websocket.TextMessage, []byte(strings.TrimSpace(line)))
			if err != nil {
				fmt.Println("Write error:", err)
				return
			}
		}
	}
}

func checkArduinoConnection(port **serial.Port) {
	for {
		if isConnected {
			(*port).Write([]byte("H\n"))
			time.Sleep(HEARTBEAT_INTERVAL * time.Second)
		} else {
			c := &serial.Config{Name: "COM3", Baud: 115200}
			p, err := serial.OpenPort(c)
			if err == nil {
				fmt.Printf("is now connected :)\n")
				isConnected = true
				*port = p
			} else {
				fmt.Printf("is not connected :(\n")
				isConnected = false
				time.Sleep(1 * time.Second)
			}
		}
	}
}

func serve_html() {
	r := gin.Default()
	r.ForwardedByClientIP = true

	r.Static("/public", "./public")

	var port *serial.Port
	go checkArduinoConnection(&port)

	r.GET("/", func(c *gin.Context) {
		http.ServeFile(c.Writer, c.Request, "./public/index.html")
	})

	r.GET("/ws", func(c *gin.Context) {
		handleWebSocket(&port, c)
	})

	fmt.Println("Server started at http://localhost:8080")
	r.Run(":8080")
}

func main() {
	serve_html()
}
