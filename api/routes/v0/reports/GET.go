package reports

import (
	"api/database"
	"errors"
	"net/http"
	"strconv"

	"github.com/gin-gonic/gin"
)

func GET(c *gin.Context) {
    var fac_id, minutes uint32
    smin, ok := c.GetQuery("minutes")

    if !ok {
        minutes = 30
    } else {
        minutes, err := strconv.Atoi(smin)
        if err != nil {
            minutes = 30
        }
    }

    facstr, ok := c.GetQuery("facid")

    if !ok {
        c.AbortWithError(http.StatusBadRequest, errors.New("facid must be defined"))
        return
    }



    dbc, err := database.GetDBC()

    if err != nil {
        c.AbortWithError(http.StatusInternalServerError, errors.New("Failed to interface with the database"))
        return
    }

    rows, err := dbc.Query("SELECT ")
}
