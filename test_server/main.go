package main

import (
	"github.com/gin-gonic/gin"
	"net/http"
)

const htmlPage = `
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>FLV Player</title>
</head>
<body>
    <video id="videoElement" controls width="600" height="400"></video>
    <script src="https://cdn.jsdelivr.net/npm/flv.js@1/dist/flv.min.js"></script>
    <script>
        if (flvjs.isSupported()) {
            var videoElement = document.getElementById('videoElement');
            var flvPlayer = flvjs.createPlayer({
                type: 'flv',
                url: 'http://127.0.0.1:7001/live/movie.flv'
            });
            flvPlayer.attachMediaElement(videoElement);
            flvPlayer.load();
        }
    </script>
</body>
</html>
`

func main() {
	router := gin.Default()

	router.GET("/", func(c *gin.Context) {
		c.Header("Content-Type", "text/html")
		c.String(http.StatusOK, htmlPage)
	})

	err := router.Run(":8080")
	if err != nil {
		panic(err)
	}
}
