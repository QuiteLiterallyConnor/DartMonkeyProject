package main

import (
	"fmt"
	"log"
	"os"
	"time"

	"gorm.io/driver/mysql"
	"gorm.io/gorm"
)

// Token struct
type Token struct {
	Created_Time time.Time `json:"created_time" sql:"created_time"`
	Used_Time    time.Time `json:"used_time" sql:"used_time"`
	TokenID      string    `json:"tokenid" sql:"tokenid"`
	IsUsed       bool      `json:"isused" sql:"isused"`
}

func main() {
	// Retrieve environment variables
	user := os.Getenv("SQL_USER")
	pass := os.Getenv("SQL_PASSWORD")

	// Set up database connection string
	dsn := fmt.Sprintf("%s:%s@tcp(127.0.0.1:3306)/tokens?charset=utf8mb4&parseTime=True&loc=Local", user, pass)

	// Open database connection with GORM
	db, err := gorm.Open(mysql.Open(dsn), &gorm.Config{})
	if err != nil {
		log.Fatal(err)
	}

	// AutoMigrate the Token struct
	db.AutoMigrate(&Token{})

	// Generate phony data
	tokens := []Token{
		{Created_Time: time.Now(), TokenID: "token1", IsUsed: false},
		{Created_Time: time.Now(), TokenID: "token2", IsUsed: false},
	}

	// Insert tokens into the database
	for _, token := range tokens {
		result := db.Create(&token)
		if result.Error != nil {
			log.Fatal(result.Error)
		}
	}

	log.Println("Tokens inserted into the database successfully.")
}
