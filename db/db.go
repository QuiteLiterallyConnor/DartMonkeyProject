package db

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
	ID           uint      `gorm:"primaryKey;autoIncrement"` // Unique ID field
	Created_Time time.Time `json:"created_time" gorm:"created_time not null"`
	Used_Time    time.Time `json:"used_time" gorm:"used_time not null"`
	TokenID      string    `json:"tokenid" gorm:"tokenid not null"`
	IsUsed       bool      `json:"isused" gorm:"isused not null"`
}

func isValidToken(token string) bool {
	db := connectDb("tokens")
	tokens := getStoredTokens(db)

	for _, tkn := range tokens {
		if tkn == token {
			return true
		}
	}

	return false
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

// removeTokens removes all entries with a specified TokenID
func removeToken(db *gorm.DB, tokenID string) {
	result := db.Where("token_id = ?", tokenID).Delete(&Token{})
	if result.Error != nil {
		log.Fatal(result.Error)
	}
	log.Printf("Tokens with TokenID '%s' removed from the database.\n", tokenID)
}

// printTokens retrieves and prints tokens from the database
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
