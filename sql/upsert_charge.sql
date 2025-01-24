USE sniffy;

DROP PROCEDURE IF EXISTS upsert_charge;
DELIMITER //
CREATE procedure upsert_charge(
    id BIGINT UNSIGNED,
    aid BIGINT UNSIGNED,
    sid VARCHAR(32),
    docket VARCHAR(32),
    bond INT UNSIGNED,
    chargedAt DATETIME,
    notes VARCHAR(4096),
    sid_name VARCHAR(256)
)
BEGIN
    IF sid_name IS NOT NULL then
        INSERT INTO statutes (ID, Name) VALUES(sid, sid_name) ON DUPLICATE KEY UPDATE `ID` = VALUES(`id`);
    END IF;
    INSERT INTO charges (ID, AID, SID, DocketNumber, Bond, ChargedAt, Notes) VALUES(id, aid, sid, docket, bond, chargedAt, notes) ON DUPLICATE KEY UPDATE `Bond` = VALUES(`Bond`);
END //
DELIMITER ;
