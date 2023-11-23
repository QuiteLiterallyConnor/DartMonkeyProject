package main

import (
	"fmt"
	"log"
	"math/rand"
	"os"
	"time"

	"gorm.io/driver/mysql"
	"gorm.io/gorm"
)

// Token struct
type Token struct {
	ID           uint      `gorm:"primaryKey;autoIncrement"` // Unique ID field
	Created_Time time.Time `json:"created_time" gorm:"created_time not null"`
	Used_Time    time.Time `json:"used_time" gorm:"used_time not null"`
	TokenID      string    `json:"tokenid" gorm:"tokenid not null"`
	IsUsed       bool      `json:"isused" gorm:"isused not null"`
}

func main() {
	db := connectDb("tokens")

	token := Token{Created_Time: time.Now(), Used_Time: time.Now(), TokenID: generateRandomString(), IsUsed: false}

	result := db.Create(&token)
	if result.Error != nil {
		log.Fatal(result.Error)
	}

	fmt.Printf("Added token \"%s\"\n", token.TokenID)

	printTokens(db)
}

func generateRandomString() string {
	rand.Seed(time.Now().UnixNano())

	// Characters to choose from
	charSet := "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
	length := 12
	randomString := make([]byte, length)

	for i := range randomString {
		randomString[i] = charSet[rand.Intn(len(charSet))]
	}

	return string(randomString)
}

func connectDb(dbName string) *gorm.DB {
	user := os.Getenv("SQL_USER")
	pass := os.Getenv("SQL_PASSWORD")

	// Set up database connection string
	dsn := fmt.Sprintf("%s:%s@tcp(127.0.0.1:3306)/%s?charset=utf8mb4&parseTime=True&loc=Local", user, pass, dbName)

	// Open database connection with GORM
	db, err := gorm.Open(mysql.Open(dsn), &gorm.Config{})
	if err != nil {
		log.Fatal(err)
	}

	// AutoMigrate the Token struct
	db.AutoMigrate(&Token{})

	return db
}

func removeToken(db *gorm.DB, tokenID string) {
	result := db.Where("token_id = ?", tokenID).Delete(&Token{})
	if result.Error != nil {
		log.Fatal(result.Error)
	}
	log.Printf("Tokens with TokenID '%s' removed from the database.\n", tokenID)
}

func getStoredTokens(db *gorm.DB) (t []string) {
	var tokens []Token
	result := db.Find(&tokens)
	if result.Error != nil {
		log.Fatal(result.Error)
	}

	for _, token := range tokens {
		t = append(t, token.TokenID)
	}

	return
}

func printTokens(db *gorm.DB) {
	tkns := getStoredTokens(db)
	for _, tkn := range tkns {
		fmt.Println(tkn)
	}
}
