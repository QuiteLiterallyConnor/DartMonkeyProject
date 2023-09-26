package main

import (
	"fmt"
	"net/http"
	"strings"
	"sync"

	"github.com/gin-gonic/gin"
	"github.com/gorilla/websocket"
)

type User struct {
	IP       string
	Location string
}

var (
	users     = make(map[*websocket.Conn]User)
	usersLock = sync.RWMutex{}
	upgrader  = websocket.Upgrader{}
)

func main() {
	router := gin.Default()
	router.LoadHTMLFiles("users.tmpl", "placeholder.html")

	router.GET("/", func(c *gin.Context) {
		c.HTML(http.StatusOK, "users.tmpl", nil)
	})
	router.GET("/ws", usersWs)
	router.GET("/page1", placeholderPage1)
	router.GET("/page2", placeholderPage2)
	router.GET("/ws1", placeholderWs1)
	router.GET("/ws2", placeholderWs2)

	router.Run(":8080")
}

func usersWs(c *gin.Context) {
	ws, err := upgrader.Upgrade(c.Writer, c.Request, nil)
	if err != nil {
		return
	}
	defer ws.Close()

	// Send initial list
	sendUsersList(ws)

	for {
		_, _, err := ws.ReadMessage()
		if err != nil {
			break
		}
	}
}

func placeholderWs1(c *gin.Context) {
	ws, err := upgrader.Upgrade(c.Writer, c.Request, nil)
	if err != nil {
		return
	}
	addUser(ws, "/page1")
	defer removeUser(ws)
	handleWs(ws)
}

func placeholderWs2(c *gin.Context) {
	ws, err := upgrader.Upgrade(c.Writer, c.Request, nil)
	if err != nil {
		return
	}
	addUser(ws, "/page2")
	defer removeUser(ws)
	handleWs(ws)
}

func placeholderPage1(c *gin.Context) {
	c.HTML(http.StatusOK, "placeholder.html", nil)
}

func placeholderPage2(c *gin.Context) {
	c.HTML(http.StatusOK, "placeholder.html", nil)
}

func addUser(ws *websocket.Conn, location string) {
	ip := strings.Split(ws.RemoteAddr().String(), ":")[0]
	user := User{IP: ip, Location: location}

	usersLock.Lock()
	users[ws] = user
	usersLock.Unlock()

	broadcastUsersList()

	// Debugging output
	fmt.Printf("User connected: IP - %s, Location - %s\n", user.IP, user.Location)
}

func removeUser(ws *websocket.Conn) {
	usersLock.Lock()
	user, exists := users[ws]
	delete(users, ws)
	usersLock.Unlock()

	broadcastUsersList()

	// Debugging output
	if exists {
		fmt.Printf("User disconnected: IP - %s, Location - %s\n", user.IP, user.Location)
	}
}

func sendUsersList(ws *websocket.Conn) {
	usersLock.RLock()
	defer usersLock.RUnlock()

	var userList []User
	for _, user := range users {
		userList = append(userList, user)
	}

	ws.WriteJSON(gin.H{"Users": userList})
}

func broadcastUsersList() {
	usersLock.RLock()
	defer usersLock.RUnlock()

	var userList []User
	for _, user := range users {
		userList = append(userList, user)
	}

	for userWs := range users {
		userWs.WriteJSON(gin.H{"Users": userList})
	}
}

func handleWs(ws *websocket.Conn) {
	defer ws.Close()
	for {
		_, _, err := ws.ReadMessage()
		if err != nil {
			break
		}
	}
}
