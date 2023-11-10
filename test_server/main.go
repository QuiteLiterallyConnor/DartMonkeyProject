package main

import (
	"context"
	"fmt"
	"log"
	"mime/multipart"
	"net/textproto"

	"github.com/gin-gonic/gin"
	"github.com/vladimirvivien/go4vl/device"
	"github.com/vladimirvivien/go4vl/v4l2"
)

// Webcam class
type Webcam struct {
	camera *device.Device
	frames <-chan []byte
}

func NewWebcam(deviceName string) (*Webcam, error) {
	camera, err := device.Open(
		deviceName,
		device.WithPixFormat(v4l2.PixFormat{PixelFormat: v4l2.PixelFmtMJPEG, Width: 640, Height: 480}),
	)
	if err != nil {
		return nil, err
	}

	if err := camera.Start(context.TODO()); err != nil {
		camera.Close()
		return nil, err
	}

	return &Webcam{
		camera: camera,
		frames: camera.GetOutput(),
	}, nil
}

func (webcam *Webcam) Close() {
	webcam.camera.Close()
}

func (webcam *Webcam) Stream(c *gin.Context) {
	w := c.Writer
	mimeWriter := multipart.NewWriter(w)
	w.Header().Set("Content-Type", fmt.Sprintf("multipart/x-mixed-replace; boundary=%s", mimeWriter.Boundary()))
	partHeader := make(textproto.MIMEHeader)
	partHeader.Add("Content-Type", "image/jpeg")

	for frame := range webcam.frames {
		partWriter, err := mimeWriter.CreatePart(partHeader)
		if err != nil {
			log.Printf("failed to create multi-part writer: %s", err)
			return
		}

		if _, err := partWriter.Write(frame); err != nil {
			log.Printf("failed to write image: %s", err)
		}
	}
}

func main() {
	port := ":9090"
	devName := "/dev/video1"

	webcam, err := NewWebcam(devName)
	if err != nil {
		log.Fatalf("failed to initialize webcam: %s", err)
	}
	defer webcam.Close()

	router := gin.Default()
	router.GET("/stream", webcam.Stream)

	log.Printf("Serving images on %s/stream", port)
	log.Fatal(router.Run(port))
}
