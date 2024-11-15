package types

type Inmate struct {
    PID string `json:"pid" yaml:"pid"`
    AID int `json:"aid" yaml:"pid"`
    FirstName string `json:"firstName" yaml:"firstName"`
    LastName string `json:"lastName" yaml:"lastName"`
    Bond int `json:"bond" yaml:"bond"`
    Date string `json:"date" yaml:"date"`
    Charges []Charge `json:"charges" yaml:"charges"`
}

type Charge struct {
    ChargedAt string `json:"chargedAt" yaml:"chargedAt"`
    Bond int `json:"bond" yaml:"bond"`
    Notes string `json:"notes" yaml:"notes"`
    Charge string `json:"charge" yaml:"charge"`
    ChargeName string `json:"chargeName" yaml:"chargeName"`
}

// This is returned for more general queries
type People struct {
    PID string `json:"pid"`
    FirstName string `json:"firstName"`
    MiddleName string `json:"middleName"`
    LastName string `json:"lastName"`
    BirthYear int `json:"birthYear"`
    NumberOfArrests int `json:"noa"`
    Arrests []int `json:"arrests"`
    Weight int `json:"weight"`
    Gender bool `json:"sex"`
    Height int `json:"height"`
    Race string `json:"race"`
}

type QueryResults struct {
    Size int `json:"size" yaml:"size"`
    TTL string `json:"ttl" yaml:"ttl"`
    QT int64 `json:"qt" yaml:"qt"`
    Time string `json:"time" yaml:"time"`
    Data []any `json:"data" yaml:"data"`
}
