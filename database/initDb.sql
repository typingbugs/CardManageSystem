DROP DATABASE IF EXISTS cardManageSystem;
CREATE DATABASE cardManageSystem;
ALTER DATABASE cardManageSystem CHARACTER SET utf8;

DROP USER IF EXISTS 'cardManageSystem'@'%';
CREATE USER 'cardManageSystem'@'%' IDENTIFIED BY 'RFID666';

USE cardManageSystem;

DROP PROCEDURE IF EXISTS sp_depositCard;
DROP PROCEDURE IF EXISTS sp_consumeCard;

CREATE TABLE device (
    id INT AUTO_INCREMENT PRIMARY KEY,
    `name` VARCHAR(255) UNIQUE,
    depositAllowed TINYINT NOT NULL
) AUTO_INCREMENT=1000;

CREATE TABLE `user` (
    id INT PRIMARY KEY,
    `name` VARCHAR(255) NOT NULL,
    `password` VARCHAR(255) NOT NULL
);

CREATE TABLE card (
    id VARCHAR(16) PRIMARY KEY,
    `status` TINYINT NOT NULL,
    balance DECIMAL(6, 2) NOT NULL,
    userId INT,
    FOREIGN KEY (userId) REFERENCES `user`(id),
    CONSTRAINT chk_card_balance CHECK (balance >= 0)
);

CREATE TABLE record (
    id VARCHAR(255) PRIMARY KEY,
    cardId VARCHAR(16),
    `time` DATETIME NOT NULL,
    `type` TINYINT NOT NULL,
    `value` DECIMAL(6, 2) NOT NULL,
    originalBalance DECIMAL(6, 2) NOT NULL,
    balance DECIMAL(6, 2) NOT NULL,
    deviceId INT,
    FOREIGN KEY (cardId) REFERENCES card(id),
    FOREIGN KEY (deviceId) REFERENCES device(id),
    CONSTRAINT chk_value CHECK (`value` >= -300),
    CONSTRAINT chk_record CHECK (originalBalance + `value` = balance),
    CONSTRAINT chk_record_originalBalance CHECK (originalBalance >= 0),
    CONSTRAINT chk_record_balance CHECK (balance >= 0)
);


DELIMITER //

CREATE PROCEDURE sp_depositCard(
    IN p_cardId VARCHAR(16),
    IN p_amount DECIMAL(6, 2),
    IN p_recordId VARCHAR(255),
    IN p_deviceId INT,
    IN p_time DATETIME,
    IN p_type TINYINT,
    OUT p_newBalance DECIMAL(6, 2)
)
BEGIN
    DECLARE v_originalBalance DECIMAL(6, 2);
    

    SELECT balance INTO v_originalBalance
    FROM card
    WHERE id = p_cardId;
    
    IF v_originalBalance IS NULL THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Card not found';
    END IF;


    SET p_newBalance = v_originalBalance + p_amount;
    
    
    UPDATE card
    SET balance = p_newBalance
    WHERE id = p_cardId;
    
    
    INSERT INTO record (id, cardId, `time`, `type`, `value`, originalBalance, balance, deviceId)
    VALUES (p_recordId, p_cardId, p_time, p_type, p_amount, v_originalBalance, p_newBalance, p_deviceId);
    
END //


CREATE PROCEDURE sp_consumeCard(
    IN p_cardId VARCHAR(18),
    IN p_amount DECIMAL(6, 2),
    IN p_recordId VARCHAR(255),
    IN p_deviceId INT,
    IN p_time DATETIME,
    IN p_type TINYINT,
    OUT p_newBalance DECIMAL(6, 2)
)
BEGIN
    DECLARE v_originalBalance DECIMAL(6, 2);

    IF p_amount > 300 THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Invalid Amount';
    END IF;
    
    
    SELECT balance INTO v_originalBalance
    FROM card
    WHERE id = p_cardId;
    
    IF v_originalBalance IS NULL THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Card not found';
    END IF;
    
    
    SET p_newBalance = v_originalBalance - p_amount;
    
    
    IF p_newBalance < 0 THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Insufficient balance';
    END IF;
    
    
    UPDATE card
    SET balance = p_newBalance
    WHERE id = p_cardId;

    
    INSERT INTO record (id, cardId, `time`, `type`, `value`, originalBalance, balance, deviceId)
    VALUES (p_recordId, p_cardId, p_time, p_type, -p_amount, v_originalBalance, p_newBalance, p_deviceId);
    
END //

DELIMITER ;


CREATE VIEW record_view AS
SELECT time, type, value, balance, `name` AS device, record.id AS id, cardId
FROM record
JOIN device
ON deviceId = device.id
ORDER BY time DESC, type DESC;


INSERT INTO device (`name`, depositAllowed)
VALUES ('食堂1号机', 0), ('管理员1号机', 1);


GRANT ALL PRIVILEGES ON cardManageSystem.* TO 'cardManageSystem'@'%';

FLUSH PRIVILEGES;