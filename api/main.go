package main

import (
    "fmt"
    "api/database"
    "api/routes"
    "github.com/gin-gonic/gin"
)

func main() {
    r := gin.Default()
    routes.CreateRouter(r)
    err := database.Connect()

    if err != nil {
        fmt.Println(err)
        return
    }

    r.Run()
}
