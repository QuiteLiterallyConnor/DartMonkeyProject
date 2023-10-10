package main

import (
	"bytes"
	"image/jpeg"
	"net/http"
	"time"

	"github.com/gin-gonic/gin"
	"gocv.io/x/gocv"
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
	c.Writer.Header().Set("Content-Type", "multipart/x-mixed-replace; boundary=myboundary")

	video, err := gocv.OpenVideoCapture(0) // 0 is the default camera index
	if err != nil {
		c.String(http.StatusInternalServerError, "Error opening video capture")
		return
	}
	defer video.Close()

	img := gocv.NewMat()
	defer img.Close()

	for {
		if ok := video.Read(&img); !ok {
			c.String(http.StatusInternalServerError, "Cannot read from video capture")
			return
		}
		if img.Empty() {
			continue
		}

		buf := new(bytes.Buffer)
		if err := jpeg.Encode(buf, img.ToImage(), nil); err != nil {
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
