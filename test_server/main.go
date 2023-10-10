package main

import (
	"bytes"
	"image/jpeg"
	"net/http"
	"time"

	"github.com/faiface/pixel/pixelgl"
	"github.com/gin-gonic/gin"
)

func main() {
	go func() {
		pixelgl.Run(func() {
			r := gin.Default()
			r.GET("/mjpeg", MJPEGStream)
			r.GET("/", func(c *gin.Context) {
				c.Header("Content-Type", "text/html")
				c.String(http.StatusOK, `<img src="/mjpeg" />`)
			})
			r.Run(":8080")
		})
	}()
	select {} // This will prevent the main function from exiting immediately
}

func MJPEGStream(c *gin.Context) {
	c.Writer.Header().Set("Content-Type", "multipart/x-mixed-replace; boundary=myboundary")

	cfg := pixelgl.DefaultCameraConfig
	camera, err := pixelgl.NewCamera(cfg)
	if err != nil {
		c.String(http.StatusInternalServerError, "Error initializing camera")
		return
	}
	defer camera.Close()

	for {
		frame := camera.Update()
		if frame == nil {
			c.String(http.StatusInternalServerError, "Error reading frame from camera")
			return
		}

		buf := new(bytes.Buffer)
		if err := jpeg.Encode(buf, frame, nil); err != nil {
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
}
