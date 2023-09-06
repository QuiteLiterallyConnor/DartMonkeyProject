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

func handleWebSocket(port *serial.Port, c *gin.Context) {
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
			if messageType == websocket.TextMessage && isConnected {
				msg := strings.TrimSpace(string(p)) + "\n"
				port.Write([]byte(msg))
			}
		}
	}()

	// Loop for listening to messages from the Arduino if connected
	reader := bufio.NewReader(port)
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
		if !isConnected {
			c := &serial.Config{Name: "COM3", Baud: 115200}
			p, err := serial.OpenPort(c)
			if err == nil {
				isConnected = true
				*port = p
			} else {
				time.Sleep(5 * time.Second)
			}
		}
	}
}

func serve_html() {
	r := gin.Default()
	r.ForwardedByClientIP = true

	var port *serial.Port
	go checkArduinoConnection(&port)

	r.GET("/", func(c *gin.Context) {
		http.ServeFile(c.Writer, c.Request, "index.html")
	})

	r.GET("/ws", func(c *gin.Context) {
		handleWebSocket(port, c)
	})

	fmt.Println("Server started at http://localhost:8080")
	r.Run(":8080")
}

func main() {
	serve_html()
}
