package main

import (
	"bytes"
	"fmt"
	"image/jpeg"
	"net/http"
	"os/exec"
	"time"

	"github.com/gin-gonic/gin"
)

func MJPEGStream(c *gin.Context) {
	c.Writer.Header().Set("Content-Type", "multipart/x-mixed-replace; boundary=myboundary")

	cmd := exec.Command("ffmpeg", "-f", "v4l2", "-i", "/dev/video1", "-f", "image2pipe", "-vcodec", "mjpeg", "-")
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		c.String(http.StatusInternalServerError, "Error starting ffmpeg")
		return
	}

	if err := cmd.Start(); err != nil {
		c.String(http.StatusInternalServerError, "Error starting ffmpeg command")
		return
	}
	defer cmd.Process.Kill()

	fmt.Printf("Successfully started FFMPEG\n")

	buff := make([]byte, 512)
	for {
		n, err := stdout.Read(buff)
		if err != nil {
			break
		}
		img, err := jpeg.Decode(bytes.NewReader(buff[:n]))
		if err != nil {
			continue
		}

		buf := new(bytes.Buffer)

		fmt.Printf("img: %v\n", img)

		if err := jpeg.Encode(buf, img, nil); err != nil {
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

func main() {
	r := gin.Default()

	r.GET("/mjpeg", MJPEGStream)
	r.GET("/", func(c *gin.Context) {
		c.Header("Content-Type", "text/html")
		c.String(http.StatusOK, `
			<img id="webcam" src="/mjpeg" />
			<script>
				const img = document.getElementById('webcam');
				img.addEventListener('load', () => {
					setTimeout(() => {
						img.src = "/mjpeg?rand=" + Math.random();
					}, 50);
				});
			</script>
		`)
	})

	r.Run(":8080")
}
