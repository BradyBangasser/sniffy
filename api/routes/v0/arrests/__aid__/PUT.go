package inmates

import (
    "net/http"
    "github.com/gin-gonic/gin"
)

func PUT(c *gin.Context) {
    c.String(http.StatusOK, "ok")
}
