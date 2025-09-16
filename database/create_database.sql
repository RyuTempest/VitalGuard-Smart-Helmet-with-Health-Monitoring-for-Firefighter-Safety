-- VitalGuard MySQL Database Setup for XAMPP
-- Run this SQL script in phpMyAdmin to create the database and table

-- Create database
CREATE DATABASE IF NOT EXISTS vitalguard_db;
USE vitalguard_db;

-- Create sensor_data table with all VitalGuard fields
CREATE TABLE IF NOT EXISTS sensor_data (
    id INT AUTO_INCREMENT PRIMARY KEY,
    device_id VARCHAR(50) NOT NULL,
    timestamp BIGINT NOT NULL,
    date_time VARCHAR(50),
    
    -- Vital Signs Data (Objective 1)
    heart_rate DECIMAL(5,2),
    spo2 DECIMAL(5,2),
    body_temperature DECIMAL(5,2),
    ambient_temperature DECIMAL(5,2),
    
    -- Environmental Data (Objective 2)
    co_level DECIMAL(5,2),
    smoke_detected TINYINT(1) DEFAULT 0,
    
    -- Motion and Safety Data (Objective 3)
    accel_x DECIMAL(8,4),
    accel_y DECIMAL(8,4),
    accel_z DECIMAL(8,4),
    gyro_x DECIMAL(8,4),
    gyro_y DECIMAL(8,4),
    gyro_z DECIMAL(8,4),
    fall_detected TINYINT(1) DEFAULT 0,
    impact_detected TINYINT(1) DEFAULT 0,
    motionless_alert TINYINT(1) DEFAULT 0,
    
    -- Health Alert System (Objective 4)
    heat_stress TINYINT(1) DEFAULT 0,
    co_warning TINYINT(1) DEFAULT 0,
    critical_vitals TINYINT(1) DEFAULT 0,
    emergency_status TINYINT(1) DEFAULT 0,
    alert_active TINYINT(1) DEFAULT 0,
    
    -- System Health (Objective 5)
    system_error TINYINT(1) DEFAULT 0,
    sensor_error TINYINT(1) DEFAULT 0,
    
    -- Data Management (Objective 6)
    session_time BIGINT,
    data_packet_count INT,
    wifi_connected TINYINT(1) DEFAULT 0,
    free_heap INT,
    
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_device_id (device_id),
    INDEX idx_timestamp (timestamp),
    INDEX idx_created_at (created_at),
    INDEX idx_emergency (emergency_status),
    INDEX idx_alerts (fall_detected, heat_stress, co_warning, critical_vitals)
);

-- Create alerts table for storing alert history
CREATE TABLE IF NOT EXISTS alerts (
    id INT AUTO_INCREMENT PRIMARY KEY,
    device_id VARCHAR(50) NOT NULL,
    alert_type ENUM('fall_detected', 'impact_detected', 'motionless_alert', 'heat_stress', 'co_warning', 'critical_vitals', 'emergency', 'system_error', 'sensor_error') NOT NULL,
    alert_message TEXT,
    timestamp BIGINT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_device_id (device_id),
    INDEX idx_alert_type (alert_type),
    INDEX idx_timestamp (timestamp)
);

-- Create devices table for device management
CREATE TABLE IF NOT EXISTS devices (
    id INT AUTO_INCREMENT PRIMARY KEY,
    device_id VARCHAR(50) UNIQUE NOT NULL,
    device_name VARCHAR(100),
    firefighter_name VARCHAR(100),
    status ENUM('active', 'inactive', 'maintenance') DEFAULT 'active',
    last_seen TIMESTAMP NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_device_id (device_id),
    INDEX idx_status (status)
);

-- Insert sample device
INSERT INTO devices (device_id, device_name, firefighter_name, status) 
VALUES ('VitalGuard_001', 'Smart Helmet #1', 'Firefighter Alpha', 'active')
ON DUPLICATE KEY UPDATE 
device_name = VALUES(device_name),
firefighter_name = VALUES(firefighter_name),
status = VALUES(status);

-- Create view for latest sensor readings
CREATE OR REPLACE VIEW latest_readings AS
SELECT 
    s.*,
    d.device_name,
    d.firefighter_name,
    d.status as device_status
FROM sensor_data s
LEFT JOIN devices d ON s.device_id = d.device_id
WHERE s.created_at = (
    SELECT MAX(created_at) 
    FROM sensor_data s2 
    WHERE s2.device_id = s.device_id
);

-- Create view for alert summary
CREATE OR REPLACE VIEW alert_summary AS
SELECT 
    device_id,
    COUNT(CASE WHEN fall_detected = 1 THEN 1 END) as fall_count,
    COUNT(CASE WHEN impact_detected = 1 THEN 1 END) as impact_count,
    COUNT(CASE WHEN motionless_alert = 1 THEN 1 END) as motionless_count,
    COUNT(CASE WHEN heat_stress = 1 THEN 1 END) as heat_stress_count,
    COUNT(CASE WHEN co_warning = 1 THEN 1 END) as co_warning_count,
    COUNT(CASE WHEN critical_vitals = 1 THEN 1 END) as critical_vitals_count,
    COUNT(CASE WHEN emergency_status = 1 THEN 1 END) as emergency_count,
    COUNT(CASE WHEN system_error = 1 THEN 1 END) as system_error_count,
    COUNT(CASE WHEN sensor_error = 1 THEN 1 END) as sensor_error_count,
    COUNT(CASE WHEN alert_active = 1 THEN 1 END) as total_alerts,
    MAX(created_at) as last_alert_time
FROM sensor_data 
WHERE created_at >= DATE_SUB(NOW(), INTERVAL 24 HOUR)
GROUP BY device_id;
