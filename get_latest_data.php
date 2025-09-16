<?php
// Get latest sensor data for dashboard display
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');

// Database configuration
$servername = "localhost";
$username = "root";
$password = "";
$dbname = "vitalguard_db";

try {
    $conn = new PDO("mysql:host=$servername;dbname=$dbname", $username, $password);
    $conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    
    // Get latest readings for each device
    $sql = "SELECT * FROM latest_readings ORDER BY created_at DESC";
    $stmt = $conn->prepare($sql);
    $stmt->execute();
    
    $data = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    echo json_encode($data);
    
} catch(PDOException $e) {
    http_response_code(500);
    echo json_encode(['error' => 'Database error: ' . $e->getMessage()]);
}

$conn = null;
?>
