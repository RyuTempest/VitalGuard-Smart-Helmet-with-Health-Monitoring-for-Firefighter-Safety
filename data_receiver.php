<?php
// VitalGuard Data Receiver for XAMPP MySQL Database
// This file receives sensor data from ESP32 and stores it in MySQL database

header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type');

// Database configuration
$servername = "localhost";
$username = "root";  // Default XAMPP MySQL username
$password = "";      // Default XAMPP MySQL password (empty)
$dbname = "vitalguard_db";

// Get JSON data from ESP32
$json = file_get_contents('php://input');
$data = json_decode($json, true);

// Check if JSON is valid
if (json_last_error() !== JSON_ERROR_NONE) {
    http_response_code(400);
    echo json_encode(['error' => 'Invalid JSON data']);
    exit;
}

// Validate required fields
if (!isset($data['device_id']) || !isset($data['api_key'])) {
    http_response_code(400);
    echo json_encode(['error' => 'Missing required fields: device_id or api_key']);
    exit;
}

// Validate API key
if ($data['api_key'] !== 'vitalguard_api_2024') {
    http_response_code(401);
    echo json_encode(['error' => 'Invalid API key']);
    exit;
}

try {
    // Create database connection
    $conn = new PDO("mysql:host=$servername;dbname=$dbname", $username, $password);
    $conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    
    // Prepare SQL statement for all VitalGuard sensor data
    $sql = "INSERT INTO sensor_data (
        device_id, 
        timestamp, 
        date_time,
        -- Vital Signs Data
        heart_rate, 
        spo2, 
        body_temperature, 
        ambient_temperature, 
        -- Environmental Data
        co_level,
        smoke_detected,
        -- Motion and Safety Data
        accel_x, 
        accel_y, 
        accel_z, 
        gyro_x, 
        gyro_y, 
        gyro_z,
        fall_detected,
        impact_detected,
        motionless_alert,
        -- Health Alert System
        heat_stress, 
        co_warning,
        critical_vitals,
        emergency_status,
        alert_active,
        -- System Health
        system_error,
        sensor_error,
        -- Data Management
        session_time,
        data_packet_count,
        wifi_connected, 
        free_heap
    ) VALUES (
        :device_id, 
        :timestamp, 
        :date_time,
        -- Vital Signs Data
        :heart_rate, 
        :spo2, 
        :body_temperature, 
        :ambient_temperature, 
        -- Environmental Data
        :co_level,
        :smoke_detected,
        -- Motion and Safety Data
        :accel_x, 
        :accel_y, 
        :accel_z, 
        :gyro_x, 
        :gyro_y, 
        :gyro_z,
        :fall_detected,
        :impact_detected,
        :motionless_alert,
        -- Health Alert System
        :heat_stress, 
        :co_warning,
        :critical_vitals,
        :emergency_status,
        :alert_active,
        -- System Health
        :system_error,
        :sensor_error,
        -- Data Management
        :session_time,
        :data_packet_count,
        :wifi_connected, 
        :free_heap
    )";
    
    $stmt = $conn->prepare($sql);
    
    // Bind parameters for all VitalGuard sensor data
    $stmt->bindParam(':device_id', $data['device_id']);
    $stmt->bindParam(':timestamp', $data['timestamp']);
    $stmt->bindParam(':date_time', $data['date_time']);
    
    // Vital Signs Data
    $stmt->bindParam(':heart_rate', $data['heart_rate']);
    $stmt->bindParam(':spo2', $data['spo2']);
    $stmt->bindParam(':body_temperature', $data['body_temperature']);
    $stmt->bindParam(':ambient_temperature', $data['ambient_temperature']);
    
    // Environmental Data
    $stmt->bindParam(':co_level', $data['co_level']);
    $stmt->bindParam(':smoke_detected', $data['smoke_detected']);
    
    // Motion and Safety Data
    $stmt->bindParam(':accel_x', $data['accel_x']);
    $stmt->bindParam(':accel_y', $data['accel_y']);
    $stmt->bindParam(':accel_z', $data['accel_z']);
    $stmt->bindParam(':gyro_x', $data['gyro_x']);
    $stmt->bindParam(':gyro_y', $data['gyro_y']);
    $stmt->bindParam(':gyro_z', $data['gyro_z']);
    $stmt->bindParam(':fall_detected', $data['fall_detected']);
    $stmt->bindParam(':impact_detected', $data['impact_detected']);
    $stmt->bindParam(':motionless_alert', $data['motionless_alert']);
    
    // Health Alert System
    $stmt->bindParam(':heat_stress', $data['heat_stress']);
    $stmt->bindParam(':co_warning', $data['co_warning']);
    $stmt->bindParam(':critical_vitals', $data['critical_vitals']);
    $stmt->bindParam(':emergency_status', $data['emergency_status']);
    $stmt->bindParam(':alert_active', $data['alert_active']);
    
    // System Health
    $stmt->bindParam(':system_error', $data['system_error']);
    $stmt->bindParam(':sensor_error', $data['sensor_error']);
    
    // Data Management
    $stmt->bindParam(':session_time', $data['session_time']);
    $stmt->bindParam(':data_packet_count', $data['data_packet_count']);
    $stmt->bindParam(':wifi_connected', $data['wifi_connected']);
    $stmt->bindParam(':free_heap', $data['free_heap']);
    
    // Execute the statement
    if ($stmt->execute()) {
        echo json_encode([
            'status' => 'success', 
            'message' => 'Data saved successfully',
            'device_id' => $data['device_id'],
            'timestamp' => $data['timestamp']
        ]);
    } else {
        echo json_encode(['status' => 'error', 'message' => 'Failed to save data']);
    }
    
} catch(PDOException $e) {
    http_response_code(500);
    echo json_encode([
        'status' => 'error', 
        'message' => 'Database error: ' . $e->getMessage()
    ]);
}

$conn = null;
?>
