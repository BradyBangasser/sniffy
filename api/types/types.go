package types

type Inmates struct {
    PID string `json:"pid" yaml:"pid"`
    AID int `json:"aid" yaml:"pid"`
    FirstName string `json:"firstName" yaml:"firstName"`
    MiddleName string `json:"middleName" yaml:"middleName"`
    LastName string `json:"lastName" yaml:"lastName"`
    Age uint8 `json:"age" yaml:"age"`
    Bond int `json:"bond" yaml:"bond"`
    Date string `json:"date" yaml:"date"`
    Charges []Charges `json:"charges" yaml:"charges"`
}

type Charges struct {
    CID int `json:"cid" yaml:"cid"`
    ChargedAt string `json:"chargedAt" yaml:"chargedAt"`
    Bond int `json:"bond" yaml:"bond"`
    Charge string `json:"charge" yaml:"charge"`
    ChargeName string `json:"chargeName" yaml:"chargeName"`
}

type Inmate struct {
    PID string `json:"pid" yaml:"pid"`
    AID int `json:"aid" yaml:"pid"`
    MID string `json:"mid" yaml:"mid"`
    FirstName string `json:"firstName" yaml:"firstName"`
    MiddleName string `json:"middleName" yaml:"middleName"`
    LastName string `json:"lastName" yaml:"lastName"`
    Age uint8 `json:"age" yaml:"age"`
    Bond int `json:"bond" yaml:"bond"`
    Date string `json:"date" yaml:"date"`
    Charges []Charge `json:"charges" yaml:"charges"`
    NumberOfArrests int `json:"noa" yaml:"noa"`
    Notes string `json:"notes" yaml:"notes"`
}

type Charge struct {
    CID int `json:"cid" yaml:"cid"`
    ChargedAt string `json:"chargedAt" yaml:"chargedAt"`
    Bond int `json:"bond" yaml:"bond"`
    InitialBond int `json:"ibond" yaml:"ibond"`
    Notes string `json:"notes" yaml:"notes"`
    Charge string `json:"charge" yaml:"charge"`
    ChargeName string `json:"chargeName" yaml:"chargeName"`
    Agency string `json:"agency" yaml:"agency"`
    DocketNumber string `json:"docket" yaml:"docket"`
    ExpectedRelease string `json:"expRel" yaml:"expRel"`
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
