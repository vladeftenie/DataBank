CREATE DATABASE IF NOT EXISTS databank;
USE databank;

CREATE TABLE IF NOT EXISTS clients (
    id INT AUTO_INCREMENT PRIMARY KEY,
    imsi VARCHAR(20) NOT NULL UNIQUE,
    msisdn VARCHAR(20) NOT NULL UNIQUE,
    nume_client VARCHAR(100) NOT NULL,
    date_consumate INT DEFAULT 0,        -- in MB
    date_available INT DEFAULT 15000,    -- 15GB
    bani DOUBLE DEFAULT 0.0
);

CREATE TABLE IF NOT EXISTS pending_requests (
    id INT AUTO_INCREMENT PRIMARY KEY,
    imsi VARCHAR(20),
    msisdn VARCHAR(20),
    request_type ENUM('ADD_DATE', 'ADD_MONEY', 'CHANGE', 'REMOVE') NOT NULL,
    payload TEXT,
    status ENUM('PENDING', 'PROCESSED') DEFAULT 'PENDING',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
