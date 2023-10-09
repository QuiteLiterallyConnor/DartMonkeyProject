package main

import (
	"fmt"
	"net/http"

	"github.com/gin-gonic/gin"
	"github.com/gorilla/websocket"
)

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true
	},
}

func wsEndpoint(c *gin.Context) {
	conn, err := upgrader.Upgrade(c.Writer, c.Request, nil)
	if err != nil {
		return
	}
	defer conn.Close()

	for {
		_, msg, err := conn.ReadMessage()
		if err != nil {
			break
		}

		fmt.Printf("%v\n", string(msg))

		switch string(msg) {
		case "left":
			conn.WriteMessage(websocket.TextMessage, []byte("XO5"))
		case "up":
			conn.WriteMessage(websocket.TextMessage, []byte("YO5"))
		case "right":
			conn.WriteMessage(websocket.TextMessage, []byte("XO-5"))
		case "down":
			conn.WriteMessage(websocket.TextMessage, []byte("YO-5"))
		}
	}
}

func main() {
	r := gin.Default()
	r.GET("/ws", wsEndpoint)
	r.LoadHTMLFiles("index.html")
	r.GET("/", func(c *gin.Context) {
		c.HTML(http.StatusOK, "index.html", nil)
	})

	r.Run(":8080")
}
