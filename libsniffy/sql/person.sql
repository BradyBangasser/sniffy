DROP TABLE IF EXISTS people;

CREATE TABLE people (
    id BINARY(32) NOT NULL,
    first_name VARCHAR(256) NOT NULL,
    middle_name VARCHAR(256),
    last_name VARCHAR(256) NOT NULL,
    suffix VARCHAR(32),
    sex TINYINT UNSIGNED,
    race TINYINT UNSIGNED,
    birth_year TINYINT UNSIGNED NOT NULL,
    height TINYINT UNSIGNED,
    weight SMALLINT UNSIGNED,
    address VARCHAR(128),
    phone_number INT UNSIGNED,
    notes VARCHAR(1024),
    PRIMARY KEY(id)
);
