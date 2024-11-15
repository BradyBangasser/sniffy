package inmates

import (
    "time"
    "api/types"
    "net/http"
    "encoding/hex"
    "api/database"
    "github.com/gin-gonic/gin"
)

func GET(c *gin.Context) {
    spid := c.Param("pid")
    pid, err := hex.DecodeString(spid)

    if err != nil {
        c.AbortWithError(http.StatusBadRequest, err)
        return
    }

    q_start := time.Now()
    inmates := types.QueryResults{}
    inmates.TTL = time.Now().String()
    dbc, err := database.GetDBC()

    charges := map[int] []types.Charge {}

    if err != nil {
        c.AbortWithError(http.StatusInternalServerError, err)
        return
    }

    rows, err := dbc.Query("SELECT current_inmates.AID,current_inmates.PID, people.FirstName, people.LastName, arrests.Bond, arrests.Date FROM current_inmates JOIN people ON current_inmates.PID=? AND current_inmates.PID=people.ID JOIN arrests ON arrests.ID=current_inmates.AID ORDER BY arrests.Date DESC", pid)

    if err != nil {
        c.AbortWithError(http.StatusInternalServerError, err)
        return
    }

    defer rows.Close()

    charge_rows, err := dbc.Query("SELECT charges.AID, charges.ChargedAt, charges.Bond, charges.Notes as ChargeNotes, statutes.ID as SID, statutes.Name from charges join statutes ON charges.PID=? AND statutes.ID=charges.SID LEFT JOIN current_inmates ON charges.AID=current_inmates.AID ORDER BY charges.PID", pid)

    if err != nil {
        c.AbortWithError(http.StatusInternalServerError, err)
        return
    }

    defer charge_rows.Close()

    for charge_rows.Next() {
        charge := types.Charge{}
        var aid int
        err = charge_rows.Scan(&aid, &charge.ChargedAt, &charge.Bond, &charge.Notes, &charge.Charge, &charge.ChargeName)
        charges[aid] = append(charges[aid], charge)
    }

    for rows.Next() {
        inmate := types.Inmate{}
        var pid []byte
        err = rows.Scan(&inmate.AID, &pid, &inmate.FirstName, &inmate.LastName, &inmate.Bond, &inmate.Date)

        inmate.Charges = charges[inmate.AID]
        inmate.PID = hex.EncodeToString(pid)
        inmates.Data = append(inmates.Data, inmate)
        inmates.Size++
    }

    inmates.QT =  time.Now().UnixMicro() - q_start.UnixMicro()

    inmates.Time = time.Now().String()
    c.JSON(200, inmates)
}
