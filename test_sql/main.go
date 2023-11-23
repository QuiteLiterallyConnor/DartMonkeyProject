package main

import (
	"database/sql"
	"fmt"
	"log"
	"os"

	_ "github.com/go-sql-driver/mysql"
)

func main() {
	// Retrieve environment variables
	user := os.Getenv("SQL_USER")
	pass := os.Getenv("SQL_PASSWORD")

	// Open database connection to the "tokens" database
	db, err := sql.Open("mysql", fmt.Sprintf("%s:%s@tcp(127.0.0.1:3306)/tokens", user, pass))
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()

	// Example query to show tables in the "tokens" database
	rows, err := db.Query("SHOW TABLES")
	if err != nil {
		log.Fatal(err)
	}
	defer rows.Close()

	fmt.Println("Tables in the 'tokens' database:")
	var tableName string
	for rows.Next() {
		// Get the table name
		err := rows.Scan(&tableName)
		if err != nil {
			log.Fatal(err)
		}
		fmt.Println(tableName)
	}

	// Check for errors from iterating over rows
	err = rows.Err()
	if err != nil {
		log.Fatal(err)
	}
}
