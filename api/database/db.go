package database

import (
    "errors"
    "database/sql"
    _ "github.com/go-sql-driver/mysql"
)

var dbc *sql.DB = nil

func Connect() error {
    if dbc != nil {
        return nil
    }

    db, err := sql.Open("mysql", "sniffy@/inmates")

    if err != nil {
        return err
    }

    dbc = db

    return err
}

func Disconnect() {
    if dbc != nil {
        dbc.Close()
    }
}

func GetDBC() (*sql.DB, error) {
    if dbc == nil {
        return nil, errors.New("No dbc currently exists")
    }

    return dbc, nil
}
