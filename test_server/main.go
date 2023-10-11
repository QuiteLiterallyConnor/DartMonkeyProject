package main

import (
	"bytes"
	"log"
	"net/http"
	"os/exec"

	"github.com/gin-gonic/gin"
)

func main() {
	r := gin.Default()

	r.GET("/mjpeg", MJPEGStream)
	r.GET("/", func(c *gin.Context) {
		c.Header("Content-Type", "text/html")
		c.String(http.StatusOK, `<img src="/mjpeg" />`)
	})

	r.Run(":8080")
}

const (
	SOI = "\xff\xd8"
	EOI = "\xff\xd9"
)

var frameChan = make(chan []byte, 1)

func MJPEGStream(c *gin.Context) {
	c.Writer.Header().Set("Content-Type", "multipart/x-mixed-replace; boundary=myboundary")

	go readFramesFromFFmpeg()

	for frame := range frameChan {
		c.Writer.Write([]byte("--myboundary\r\n"))
		c.Writer.Write([]byte("Content-Type: image/jpeg\r\n"))
		c.Writer.Write([]byte("Content-Length: " + string(len(frame)) + "\r\n\r\n"))
		c.Writer.Write(frame)
		c.Writer.Write([]byte("\r\n"))
	}
}

func readFramesFromFFmpeg() {
	cmd := exec.Command("ffmpeg", "-f", "v4l2", "-i", "/dev/video1", "-f", "image2pipe", "-vcodec", "mjpeg", "-")
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		log.Printf("Error getting stdout pipe for ffmpeg: %v", err)
		return
	}

	if err := cmd.Start(); err != nil {
		log.Printf("Error starting ffmpeg command: %v", err)
		return
	}

	buff := make([]byte, 8*1024)
	var frameBuffer bytes.Buffer

	for {
		n, err := stdout.Read(buff)
		if err != nil {
			log.Printf("Error reading from stdout: %v", err)
			break
		}

		frameBuffer.Write(buff[:n])

		if bytes.HasSuffix(frameBuffer.Bytes()[len(frameBuffer.Bytes())-2:], []byte(EOI)) {
			select {
			case frameChan <- frameBuffer.Bytes():
				// frame sent to channel
			default:
				// previous frame still being processed, dropping current frame
			}

			frameBuffer.Reset() // Clear the frameBuffer for the next frame
		}
	}
}
