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
	for {
		n, err := stdout.Read(buff)
		if err != nil {
			log.Printf("Error reading from stdout: %v", err)
			break
		}

		img, err := jpeg.Decode(bytes.NewReader(buff[:n]))
		if err != nil {
			log.Printf("Error decoding jpeg: %v", err)
			continue
		}

		buf := new(bytes.Buffer)
		if err := jpeg.Encode(buf, img, nil); err != nil {
			log.Printf("Error encoding image: %v", err)
			c.String(http.StatusInternalServerError, "Error encoding image")
			return
		}

		c.Writer.Write([]byte("--myboundary\r\n"))
		c.Writer.Write([]byte("Content-Type: image/jpeg\r\n"))
		c.Writer.Write([]byte("Content-Length: " + string(len(buf.Bytes())) + "\r\n\r\n"))
		c.Writer.Write(buf.Bytes())
		c.Writer.Write([]byte("\r\n"))

		time.Sleep(33 * time.Millisecond) // ~30 FPS
	}

	log.Println("Exiting MJPEGStream function")
}
