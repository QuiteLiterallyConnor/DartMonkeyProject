package main

import (
	"github.com/gin-gonic/gin"
)

type TestData struct {
	Name    string `json:"name"`
	Age     int    `json:"age"`
	Message string `json:"message"`
}

func main() {
	r := gin.Default()
	r.LoadHTMLGlob("./public/*.html")

	r.GET("/", func(c *gin.Context) {
		c.HTML(200, "index.html", nil)
	})

	r.GET("/data", func(c *gin.Context) {
		testData := TestData{
			Name:    "John",
			Age:     30,
			Message: "Hello from Gin!",
		}
		c.JSON(200, testData)
	})

	r.Run(":8080")
}
