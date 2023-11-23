package main

import (
	"flag"
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
	ID              uint      `gorm:"primaryKey;autoIncrement"`
	Created_Time    time.Time `json:"created_time" gorm:"created_time not null"`
	Used_Time       time.Time `json:"used_time" gorm:"used_time not null"`
	TokenID         string    `json:"token_id" gorm:"token_id not null"`
	IsUsed          bool      `json:"is_used" gorm:"is_used not null"`
	SessionDuration int64     `json:"session_duration" gorm:"session_duration not null"`
}

func main() {
	// Define the command-line flags
	addFlag := flag.Bool("add", false, "Add a new token")
	removeFlag := flag.String("remove", "", "Remove a token with the specified ID")
	listFlag := flag.Bool("list", false, "List all tokens")

	// Parse the flags
	flag.Parse()

	// Database connection
	db := connectDb("tokens")

	// Check which flag was passed
	switch {
	case *addFlag:
		addToken(db)
	case *removeFlag != "":
		removeToken(db, *removeFlag)
	case *listFlag:
		printTokens(db)
	default:
		fmt.Println("No valid operation specified. Use 'add', 'remove', or 'list'.")
	}
}

func addToken(db *gorm.DB) {
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

func getStoredTokens(db *gorm.DB) (t []Token) {
	result := db.Find(&t)
	if result.Error != nil {
		log.Fatal(result.Error)
	}
	return
}

func printTokens(db *gorm.DB) {
	tkns := getStoredTokens(db)
	for _, tkn := range tkns {
		fmt.Println(tkn)
	}
}
