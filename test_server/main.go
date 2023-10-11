package main

import (
	"bytes"
	"image/jpeg"
	"log"
	"net/http"
	"os/exec"
	"time"

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

func MJPEGStream(c *gin.Context) {
	log.Println("Entering MJPEGStream function")

	c.Writer.Header().Set("Content-Type", "multipart/x-mixed-replace; boundary=myboundary")

	cmd := exec.Command("ffmpeg", "-f", "v4l2", "-i", "/dev/video1", "-f", "image2pipe", "-vcodec", "mjpeg", "-")
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		log.Printf("Error getting stdout pipe for ffmpeg: %v", err)
		c.String(http.StatusInternalServerError, "Error starting ffmpeg")
		return
	}

	if err := cmd.Start(); err != nil {
		log.Printf("Error starting ffmpeg command: %v", err)
		c.String(http.StatusInternalServerError, "Error starting ffmpeg command")
		return
	}
	defer func() {
		if err := cmd.Process.Kill(); err != nil {
			log.Printf("Error killing ffmpeg process: %v", err)
		}
	}()

	buff := make([]byte, 512)
	var frameBuffer bytes.Buffer

	for {
		n, err := stdout.Read(buff)
		if err != nil {
			log.Printf("Error reading from stdout: %v", err)
			break
		}

		frameBuffer.Write(buff[:n])

		if bytes.HasSuffix(frameBuffer.Bytes(), []byte(EOI)) {
			img, err := jpeg.Decode(&frameBuffer)
			if err != nil {
				log.Printf("Error decoding jpeg: %v", err)
				frameBuffer.Reset() // Reset buffer if we encountered an error
				continue
			}

			buf := new(bytes.Buffer)
			if err := jpeg.Encode(buf, img, nil); err != nil {
				log.Printf("Error encoding image: %v", err)
				frameBuffer.Reset() // Reset buffer if we encountered an error
				continue
			}

			c.Writer.Write([]byte("--myboundary\r\n"))
			c.Writer.Write([]byte("Content-Type: image/jpeg\r\n"))
			c.Writer.Write([]byte("Content-Length: " + string(len(buf.Bytes())) + "\r\n\r\n"))
			c.Writer.Write(buf.Bytes())
			c.Writer.Write([]byte("\r\n"))

			frameBuffer.Reset() // Clear the frameBuffer for the next frame

			time.Sleep(33 * time.Millisecond) // ~30 FPS
		}
	}

	log.Println("Exiting MJPEGStream function")
}
