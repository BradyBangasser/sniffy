package cid

import (
    "fmt"
    "strconv"
    "net/http"
    "api/types"
    "time"
    "encoding/hex"
    "api/database"
    "github.com/gin-gonic/gin"
)

type Charge struct {
    CID int `json:"cid"`
    PID string `json:"pid"`
    AID int `json:"aid"`
    Bond int `json:"bond"`
    Statute string `json:"statute"`
    Charge string `json:"charge"`
    TimeStamp string `json:"ts"`
}

func GET(c *gin.Context) {
    cid, err := strconv.Atoi(c.Param("cid"))

    if err != nil {
        c.AbortWithError(http.StatusBadRequest, err)
        return
    }

    q_start := time.Now()
    charges := types.QueryResults{}
    charges.TTL = time.Now().String()
    dbc, err := database.GetDBC()
    ct := c.GetHeader("Content-Type")

    var pid []byte

    fmt.Println(cid)

    if err != nil {
        c.AbortWithError(500, err)
        return
    }

    rows, err := dbc.Query(`
        SELECT charges.ID, charges.PID, statutes.ID,
        statutes.Name, charges.AID, charges.ChargedAt, charges.Bond FROM charges
        JOIN statutes ON charges.ID=? AND statutes.ID=charges.SID
    `, cid)

    if err != nil {
        c.AbortWithError(500, err)
        return
    }

    defer rows.Close()

    for rows.Next() {
        charge := Charge{}

        rows.Scan(&charge.CID, &pid, &charge.Statute, &charge.Charge, &charge.AID, &charge.TimeStamp, &charge.Bond)
        charge.PID = hex.EncodeToString(pid)

        charges.Data = append(charges.Data, charge)
        charges.Size++
    }


    charges.QT = time.Now().UnixMicro() - q_start.UnixMicro()
    switch ct {
    case "text/xml":
        c.XML(http.StatusOK, charges)
        return
    default: 
        c.JSON(http.StatusOK, charges)
    }
}
