package main

import (
	"fmt"
	"log"
	"os"
	"os/exec"
	"strconv"
	"time"

	"github.com/gin-gonic/gin"
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

func main() {
	g := GStreamerServer{}
	g.init()
	go g.StartWebcamStream()

	// Initialize the Gin router
	router := gin.Default()

	// Serve static assets like HTML, JS, and CSS files from the 'public' directory
	router.Static("/public", "./public")

	// Serve HLS content (m3u8 and ts files) from the 'hls' directory
	router.Static("/hls", "./hls")

	// Serve the main page
	router.GET("/", func(c *gin.Context) {
		c.File("index.html")
	})

	// Define the server port
	serverPort := "8080"

	// Start the server
	fmt.Printf("Server started at http://localhost:%s\n", serverPort)
	router.Run(":" + serverPort)
}
