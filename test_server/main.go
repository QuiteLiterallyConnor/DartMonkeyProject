package main

import (
	"bytes"
	"image/jpeg"
	"log"
	"sync"
	"time"

	"github.com/gin-gonic/gin"
	"gocv.io/x/gocv"
)

var (
	mu       sync.Mutex
	imgBytes []byte
)

func main() {
	webcam, err := gocv.VideoCaptureDevice(0) // Open default webcam. Adjust the index if you have multiple cameras.
	if err != nil {
		log.Fatalf("Failed to open webcam: %v", err)
	}
	defer webcam.Close()

	go func() {
		frame := gocv.NewMat()
		defer frame.Close()

		for {
			if ok := webcam.Read(&frame); !ok {
				log.Println("Cannot read device")
				time.Sleep(100 * time.Millisecond)
				continue
			}
			if frame.Empty() {
				continue
			}

			buf := new(bytes.Buffer)
			if err := jpeg.Encode(buf, frame.ToImage(), nil); err != nil {
				log.Printf("Failed to encode image: %v", err)
			}

			mu.Lock()
			imgBytes = buf.Bytes()
			mu.Unlock()

			time.Sleep(33 * time.Millisecond) // 30 fps
		}
	}()

	r := gin.Default()
	r.GET("/mjpeg", func(c *gin.Context) {
		c.Header("Content-Type", "multipart/x-mixed-replace; boundary=frame")

		for {
			mu.Lock()
			currentImgBytes := make([]byte, len(imgBytes))
			copy(currentImgBytes, imgBytes)
			mu.Unlock()

			c.Writer.Write([]byte("--frame\r\n"))
			c.Writer.Write([]byte("Content-Type: image/jpeg\r\n\r\n"))
			c.Writer.Write(currentImgBytes)
			c.Writer.Write([]byte("\r\n"))

			time.Sleep(33 * time.Millisecond) // 30 fps
		}
	})

	r.GET("/", func(c *gin.Context) {
		c.HTML(200, "index.html", nil)
	})

	r.LoadHTMLGlob("*.html")
	r.Run(":8080")
}
