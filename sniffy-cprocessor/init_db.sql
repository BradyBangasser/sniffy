DROP DATABASE IF EXISTS sniffy;

CREATE DATABASE sniffy;
USE sniffy;

CREATE TABLE people (
    ID BINARY(32) NOT NULL,
    FirstName VARCHAR(255) NOT NULL,
    MiddleName VARCHAR(255),
    LastName VARCHAR(255) NOT NULL,
    Address VARCHAR(255),
    Suffix VARCHAR(3),
    Race SMALLINT UNSIGNED NOT NULL,
    Height TINYINT UNSIGNED NOT NULL,
    Weight SMALLINT UNSIGNED NOT NULL,
    BirthYear SMALLINT UNSIGNED NOT NULL,
    PhoneNumber CHAR(10),
    Sex BOOL NOT NULL,
    InsertedAt DATETIME DEFAULT CURRENT_TIMESTAMP,
    UpdatedAt DATETIME ON UPDATE CURRENT_TIMESTAMP,
    -- Notes VARCHAR(16383),
    PRIMARY KEY (ID)
);

CREATE TABLE arrests (
    ID BIGINT UNSIGNED NOT NULL,
    PID BINARY(32) NOT NULL,
    BookingAgencyID VARCHAR(64) NOT NULL,
    Bond INT UNSIGNED DEFAULT 0,
    InitialBond INT UNSIGNED DEFAULT 0,
    ArrestDate DATETIME NOT NULL,
    InsertedAt DATETIME DEFAULT CURRENT_TIMESTAMP,
    UpdatedAt DATETIME ON UPDATE CURRENT_TIMESTAMP,
    -- Notes VARCHAR(16383),

    PRIMARY KEY (ID),
    FOREIGN KEY (PID) REFERENCES people(ID) ON DELETE CASCADE
);

CREATE TABLE charges (
    ID BIGINT UNSIGNED NOT NULL,
    AID BIGINT UNSIGNED NOT NULL,
    SID VARCHAR(32) NOT NULL,
    DocketNumber VARCHAR(32),
    InsertedAt DATETIME DEFAULT CURRENT_TIMESTAMP,
    UpdatedAt DATETIME ON UPDATE CURRENT_TIMESTAMP,

    -- Notes VARCHAR(16383),

    PRIMARY KEY (ID),
    FOREIGN KEY (AID) REFERENCES arrests(ID) ON DELETE CASCADE
    -- Add foreign key for statute
);

CREATE TABLE statutes (
    ID varchar(32) NOT NULL, 
    Name varchar(255) NOT NULL,
    Description mediumtext NOT NULL,
    InsertedAt DATETIME DEFAULT CURRENT_TIMESTAMP,
    UpdatedAt DATETIME ON UPDATE CURRENT_TIMESTAMP,
    -- Notes VARCHAR(16383),
    PRIMARY KEY (ID)
);

CREATE TABLE agencies (
    ID VARCHAR(64) NOT NULL,
    InsertedAt DATETIME DEFAULT CURRENT_TIMESTAMP,
    UpdatedAt DATETIME ON UPDATE CURRENT_TIMESTAMP
    -- Notes VARCHAR(16383)
);

CREATE TABLE roster (
    PID BINARY(32) NOT NULL,
    AID BIGINT UNSIGNED NOT NULL,

    FOREIGN KEY (PID) REFERENCES people(ID),
    FOREIGN KEY (AID) REFERENCES arrests(ID)
);
