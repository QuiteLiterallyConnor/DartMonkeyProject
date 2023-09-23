package main

import (
	"encoding/json"
	"fmt"
	"log"
	"net"
	"net/http"
	"regexp"
	"strings"

	"github.com/gin-gonic/gin"
	"github.com/gorilla/websocket"
	"github.com/mssola/user_agent"
	"github.com/oschwald/geoip2-golang"
)

var clients = make(map[*websocket.Conn]*http.Request)
var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true // Adjust as needed for security
	},
}

type ClientInfo struct {
	Name     string `json:"name"`
	IP       string `json:"ip"`
	Device   string `json:"device"`
	Browser  string `json:"browser"`
	OS       string `json:"os"`
	Country  string `json:"country"`
	City     string `json:"city"`
	Sessions int    `json:"sessions"`
}

func handleConnections(c *gin.Context) {
	ws, err := upgrader.Upgrade(c.Writer, c.Request, nil)
	if err != nil {
		log.Printf("Failed to set websocket upgrade: %+v", err)
		return
	}
	defer ws.Close()

	// Log the new connection
	log.Printf("New WebSocket connection from %s", c.ClientIP())

	clients[ws] = c.Request

	// Broadcast the number of connected clients to all clients
	broadcastConnectedClients()

	for {
		messageType, _, err := ws.ReadMessage()
		if err != nil {
			log.Printf("WebSocket connection from %s closed", c.ClientIP())
			delete(clients, ws)
			broadcastConnectedClients()
			break
		}
		// Handle or echo the message as needed
		ws.WriteMessage(messageType, []byte("Your message was received!"))
	}
}

func broadcastConnectedClients() {
	clientInfos := make(map[string]ClientInfo)

	for client := range clients {
		request := clients[client]
		ip := request.RemoteAddr // Use request.RemoteAddr to get the client's IP
		valid_ip, country, city, _ := getIPCityCountry(ip)

		if _, exists := clientInfos[valid_ip]; exists {
			clientInfos[valid_ip] = ClientInfo{
				IP:       valid_ip,
				Device:   clientInfos[valid_ip].Device,
				Browser:  clientInfos[valid_ip].Browser,
				OS:       clientInfos[valid_ip].OS,
				Country:  clientInfos[valid_ip].Country,
				City:     clientInfos[valid_ip].City,
				Sessions: clientInfos[valid_ip].Sessions + 1,
			}
			continue
		}

		userAgentHeader := request.Header.Get("User-Agent")
		ua := user_agent.New(userAgentHeader)
		browser, _ := ua.Browser()
		os := ua.OS()
		device := "unknown" // Default value

		if strings.Contains(strings.ToLower(userAgentHeader), "mobile") {
			device = "mobile"
		} else if strings.Contains(strings.ToLower(userAgentHeader), "tablet") {
			device = "tablet"
		} else if strings.Contains(strings.ToLower(userAgentHeader), "windows") {
			device = "desktop"
		}

		// Get the country based on the user's IP address

		clientInfos[valid_ip] = ClientInfo{
			IP:       valid_ip,
			Device:   device,
			Browser:  browser,
			OS:       os,
			Country:  country,
			City:     city,
			Sessions: 1,
		}
	}

	data := map[string]map[string]ClientInfo{
		"clients": clientInfos,
	}

	jsonData, err := json.Marshal(data)
	if err != nil {
		log.Println("Error marshaling data:", err)
		return
	}

	for client := range clients {
		err := client.WriteMessage(websocket.TextMessage, jsonData)
		if err != nil {
			log.Printf("WebSocket error: %v", err)
			client.Close()
			delete(clients, client)
		}
	}
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

		if strings.HasPrefix(ip, "[::1]") {
			return true
		}
	}

	return false
}

func extractValidIP(ipWithPort string) (string, error) {
	ipv4Pattern := `^(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})`
	ipv6Pattern := `^(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:))`

	ipv4Regex := regexp.MustCompile(ipv4Pattern)
	ipv6Regex := regexp.MustCompile(ipv6Pattern)

	if ipv4Match := ipv4Regex.FindString(ipWithPort); ipv4Match != "" {
		return ipv4Match, nil
	}

	if ipv6Match := ipv6Regex.FindString(ipWithPort); ipv6Match != "" {
		return ipv6Match, nil
	}

	return "", fmt.Errorf("No valid IPv4 or IPv6 found")
}

func getIPCityCountry(raw_ip string) (string, string, string, error) {
	if strings.HasPrefix(raw_ip, "[::1]") {
		return raw_ip, "Admin Machine", "Admin Machine", nil
	}

	ip, err := extractValidIP(raw_ip)
	if err != nil {
		return raw_ip, "Unknown", "Unknown", err
	}

	if isLocalIP(ip) {
		return ip, "Admin Network", "Admin Network", nil
	} else {
		fmt.Printf("%s is not a local ip\n", ip)
	}

	db, err := geoip2.Open("GeoLite2-City.mmdb") // Provide the path to your GeoIP2 database file.
	if err != nil {
		log.Println("Error opening GeoIP database:", err)
		return ip, "Unknown", "Unknown", err
	}
	defer db.Close()

	ipAddress := net.ParseIP(ip)
	if ipAddress == nil {
		log.Println("Invalid IP address:", ip)
		return ip, "Unknown", "Unknown", err
	}

	record, err := db.City(ipAddress)
	if err != nil {
		log.Println("Error looking up IP:", err)
		return ip, "Unknown", "Unknown", err
	}

	return ip, record.City.Names["en"], record.Country.IsoCode, nil
}

func main() {
	r := gin.Default()

	r.Static("/public", "./public")
	r.GET("/", func(c *gin.Context) {
		c.File("./public/admin.html")
	})

	r.GET("/ws", handleConnections)

	r.Run(":8081")
}
