package webcam

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

type Webcam struct {
	Name   string `json:"name"`
	Path   string `json:"path"`
	Camera *device.Device
	Frames <-chan []byte
}

func (webcam *Webcam) InitWebcam() error {
	camera, err := device.Open(
		webcam.Path,
		device.WithPixFormat(v4l2.PixFormat{PixelFormat: v4l2.PixelFmtMJPEG, Width: 640, Height: 480}),
	)
	if err != nil {
		return err
	}

	if err := camera.Start(context.TODO()); err != nil {
		camera.Close()
		return err
	}

	webcam.Camera = camera
	webcam.Frames = camera.GetOutput()

	return nil
}

func (webcam *Webcam) Close() {
	webcam.Camera.Close()
}

func (webcam *Webcam) Stream(c *gin.Context) {
	w := c.Writer
	mimeWriter := multipart.NewWriter(w)
	w.Header().Set("Content-Type", fmt.Sprintf("multipart/x-mixed-replace; boundary=%s", mimeWriter.Boundary()))
	partHeader := make(textproto.MIMEHeader)
	partHeader.Add("Content-Type", "image/jpeg")

	for frame := range webcam.Frames {
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
