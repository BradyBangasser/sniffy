package arrests

import (
    "net/http"
    "api/types"
    "time"
    "encoding/hex"
    "database/sql"
    "api/database"
    "github.com/gin-gonic/gin"
)

type Arrest struct {
    ID int `json:"id"`
    Date string `json:"ts"`
    Released string `json:"rts"`
    PID string `json:"pid"`
    Bond int `json:"bond"`
}

func GET(c *gin.Context) {
    q_start := time.Now()
    arrests := types.QueryResults{}
    arrests.TTL = time.Now().String()
    dbc, err := database.GetDBC()
    ct := c.GetHeader("Content-Type")

    if err != nil {
        c.AbortWithError(500, err)
        return
    }

    rows, err := dbc.Query("SELECT ID, Date, Released, PID, Bond FROM arrests")

    if err != nil {
        c.AbortWithError(http.StatusInternalServerError, err)
        return 
    }

    defer rows.Close()

    for rows.Next() {
        arrest := Arrest{}
        var released sql.NullString
        var bpid []byte

        err  = rows.Scan(&arrest.ID, &arrest.Date, &released, &bpid, &arrest.Bond)

        if err != nil {
            c.AbortWithError(http.StatusInternalServerError, err)
            return
        }

        arrest.PID = hex.EncodeToString(bpid)
        arrest.Released = released.String

        arrests.Data = append(arrests.Data, arrest)
        arrests.Size++
    }

    arrests.QT =  time.Now().UnixMicro() - q_start.UnixMicro()

    arrests.Time = time.Now().String()

    switch ct {
    case "text/xml":
        c.XML(http.StatusOK, arrests)
        return
    default: 
        c.JSON(200, arrests)
    }
}
