package people

import (
    "net/http"
    "api/types"
    "time"
    "encoding/hex"
    "api/database"
    "github.com/gin-gonic/gin"
)

func GET(c *gin.Context) {
    q_start := time.Now()
    people := types.QueryResults{}
    people.TTL = time.Now().String()
    dbc, err := database.GetDBC()
    ct := c.GetHeader("Content-Type")

    arrests := map[string][]int {}

    if err != nil {
        c.AbortWithError(http.StatusInternalServerError, err)
        return
    }

    rows, err := dbc.Query("SELECT ID, LastName, FirstName, MiddleName, Birthyear, Arrests, Weight, Height, Gender, Race FROM people")

    if err != nil {
        c.AbortWithError(http.StatusInternalServerError, err)
        return
    }

    defer rows.Close()

    arrest_rows, err := dbc.Query("SELECT ID, PID FROM arrests")

    if err != nil {
        c.AbortWithError(http.StatusInternalServerError, err)
        return
    }

    defer arrest_rows.Close()

    for arrest_rows.Next() {
        var (
            aid int
            pid []byte
            spid string
        )

        err = arrest_rows.Scan(&aid, &pid)

        if err != nil {
            c.AbortWithError(http.StatusInternalServerError, err)
            return
        }

        spid = hex.EncodeToString(pid)
        
        arrests[spid] = append(arrests[spid], aid)
    }

    for rows.Next() {
        person := types.People{}
        var pid []byte
        err = rows.Scan(&pid, &person.LastName, &person.FirstName, &person.MiddleName, &person.BirthYear, &person.NumberOfArrests, &person.Weight, &person.Height, &person.Gender, &person.Race)

        if err != nil {
            c.AbortWithError(http.StatusInternalServerError, err)
            return
        }

        person.PID = hex.EncodeToString(pid)
        person.Arrests = arrests[person.PID]
        people.Data = append(people.Data, person)
        people.Size++
    }

    people.QT =  time.Now().UnixMicro() - q_start.UnixMicro()

    people.Time = time.Now().String()

    switch ct {
    case "text/xml":
        c.XML(http.StatusOK, people)
        return
    default: 
        c.JSON(200, people)
    }
}
