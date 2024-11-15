package routes

import (
    "github.com/gin-gonic/gin"
)

func GET(c *gin.Context) {
    c.Data(200, "text/html", []byte("<h1>Kill Yourself</h1>"))
}
