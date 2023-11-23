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
	Created_Time time.Time `json:"created_time" gorm:"created_time not null"`
	Used_Time    time.Time `json:"used_time" gorm:"used_time not null"`
	TokenID      string    `json:"tokenid" gorm:"tokenid not null"`
	IsUsed       bool      `json:"isused" gorm:"isused not null"`
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
		{Created_Time: time.Now(), Used_Time: time.Now(), TokenID: "token1", IsUsed: false},
		{Created_Time: time.Now(), Used_Time: time.Now(), TokenID: "token2", IsUsed: false},
	}

	// Insert tokens into the database
	for _, token := range tokens {
		result := db.Create(&token)
		if result.Error != nil {
			log.Fatal(result.Error)
		}
	}

	log.Println("Tokens inserted into the database successfully.")

	// Retrieve and print tokens from the database
	printTokens(db)
}

// printTokens retrieves and prints tokens from the database
func printTokens(db *gorm.DB) {
	var tokens []Token
	result := db.Find(&tokens)
	if result.Error != nil {
		log.Fatal(result.Error)
	}

	log.Println("Tokens in the database:")
	for _, token := range tokens {
		log.Printf("TokenID: %s, Created_Time: %v, Used_Time: %v, IsUsed: %t\n", token.TokenID, token.Created_Time, token.Used_Time, token.IsUsed)
	}
}
