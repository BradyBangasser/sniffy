CREATE TABLE people (
    -- SHA2-224 hash of the following format:
    -- last:first:height:gender:middle
    -- Excluding weight due to meth
    ID binary(28) NOT NULL,
    LastName varchar(255) NOT NULL,
    FirstName varchar(255) NOT NULL,
    MiddleName varchar(255) NOT NULL,
    Birthyear int,
    Arrests int DEFAULT 1,
    Weight int NOT NULL,
    Height int NOT NULL,
    Gender bool NOT NULL,
    Race ENUM('WHITE', 'BLACK') NOT NULL,

    CHECK (Arrests >= 1),
    PRIMARY KEY (ID)
);

CREATE TABLE arrests (
    Notes mediumtext,
    ID int NOT NULL AUTO_INCREMENT,
    Date datetime NOT NULL,
    Released datetime,
    PID binary(28) NOT NULL,

    PRIMARY KEY (ID),
    FOREIGN KEY (PID) REFERENCES people(ID)
);

CREATE TABLE current_inmates (
    PID binary(28) NOT NULL,
    AID int NOT NULL,

    FOREIGN KEY (PID) REFERENCES people(ID),
    FOREIGN KEY (AID) REFERENCES arrests(ID)
);

CREATE TABLE statutes (
    -- Most iowa statutes are formatted XXX.XX, where X is a digit from 0-9
    ID varchar(32) NOT NULL, 
    Name varchar(255) NOT NULL,
    Description mediumtext NOT NULL,
    Notes mediumtext NOT NULL,
    PRIMARY KEY (ID)
);

CREATE TABLE mugshots (
    PID binary(28) NOT NULL,
    ID int NOT NULL,
    Taken datetime,
    -- Consider using blobs here

    PRIMARY KEY (ID),
    FOREIGN KEY (PID) REFERENCES people(ID)
);

CREATE TABLE charges (
    PID binary (28) NOT NULL,
    -- mugshot id
    MID int,
    -- arrest id
    AID int NOT NULL,
    -- charge id
    ID int NOT NULL AUTO_INCREMENT,
    -- Statute ID
    SID varchar(32) NOT NULL,
    ChargedAt datetime NOT NULL,
    Bond int,
    InitialBond int,
    CourtData datetime,
    ExpectedRelease datetime,
    Agency varchar(255),
    DocketNumber char(10),
    Notes mediumtext,

    PRIMARY KEY (ID),
    FOREIGN KEY (PID) REFERENCES people(ID),
    FOREIGN KEY (MID) REFERENCES mugshots(ID),
    FOREIGN KEY (SID) REFERENCES statutes(ID),
    FOREIGN KEY (AID) REFERENCES arrests(ID)
);
