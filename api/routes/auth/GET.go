package auth

import (
    "net/http"
    "github.com/gin-gonic/gin"
)

func GET(c *gin.Context) {
    c.Status(http.StatusForbidden)
}
