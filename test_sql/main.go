package main

import (
    "database/sql"
    "fmt"
    "log"

    _ "github.com/go-sql-driver/mysql"
)

func main() {
    // Open database connection
    db, err := sql.Open("mysql", "users:User1234!@tcp(127.0.0.1:3306)/")
    if err != nil {
        log.Fatal(err)
    }
    defer db.Close()

    // Execute the query
    rows, err := db.Query("SHOW DATABASES")
    if err != nil {
        log.Fatal(err)
    }
    defer rows.Close()

    fmt.Println("Databases:")
    var dbName string
    for rows.Next() {
        // Get the database name
        err := rows.Scan(&dbName)
        if err != nil {
            log.Fatal(err)
        }
        fmt.Println(dbName)
    }

    // Check for errors from iterating over rows
    err = rows.Err()
    if err != nil {
        log.Fatal(err)
    }
}
