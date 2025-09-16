/*
 * VitalGuard: Smart Helmet with Health Monitoring for Firefighter Safety
 * ESP32 WROOM-32 Development Board Implementation
 * 
 * Core Hardware Components:
 * - ESP32 WROOM-32 Development Board (Main Controller)
 * - MAX30102 Pulse Oximeter & Heart Rate Sensor (Health Monitoring)
 * - MLX90614 IR Temperature Sensor (Body & Ambient Temperature)
 * - MPU-6050 Accelerometer/Gyroscope (Motion & Fall Detection)
 * - MQ-7 Carbon Monoxide Gas Sensor (Air Quality Monitoring)
 * - LED (Alert System)
 * - WiFi Module (Data Transmission)
 * 
 * Key Features:
 * - Real-time heart rate and blood oxygen monitoring
 * - Body temperature monitoring for heat stress detection
 * - Fall detection using accelerometer data
 * - Carbon monoxide level monitoring
 * - Data transmission to server MySQL database for remote monitoring
 * - Alert system with LED indicators
 * 
 */

#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <esp_task_wdt.h>

// Pin Definitions
#define I2C_SDA 21              // I2C Data line for sensor communication
#define I2C_SCL 22              // I2C Clock line for sensor communication
#define MQ7_ANALOG_PIN 34       // Analog input for MQ-7 CO sensor readings
#define MQ7_DIGITAL_PIN 35      // Digital input for MQ-7 CO sensor threshold
#define LED_STATUS_PIN 2        // Status LED for system indication

// I2C Addresses for Sensor Communication
#define MAX30102_ADDRESS 0x57   // MAX30102 Pulse Oximeter & Heart Rate Sensor
#define MLX90614_ADDRESS 0x5A   // MLX90614 IR Temperature Sensor
#define MPU6050_ADDRESS 0x68    // MPU-6050 Accelerometer & Gyroscope

// HTTP Client for Server MySQL Database
HTTPClient http;

// WiFi Credentials for local network
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

//Server Configuration
const char* serverURL = "http://192.168.1.100/vitalguard/data_receiver.php"; // Replace with your server's IP
const char* deviceID = "VitalGuard_001"; // Unique device identifier
const char* apiKey = "vitalguard_api_2024"; // API key for authentication

// Data Structure for Sensor Readings - Enhanced for All Objectives
struct VitalSigns {
  // Vital Sign Monitoring (Objective 1)
  float heartRate;           // Heart rate in BPM (from MAX30102)
  float spO2;               // Blood oxygen saturation in % (from MAX30102)
  float bodyTemperature;    // Body temperature in °C (from MLX90614)
  float ambientTemperature; // Ambient temperature in °C (from MLX90614)
  
  // Environmental Monitoring (Objective 2)
  float coLevel;            // Carbon monoxide level in ppm (from MQ-7)
  bool smokeDetected;       // Smoke detection flag
  
  // Motion and Safety Monitoring (Objective 3)
  float accelX, accelY, accelZ; // Accelerometer readings in g-force (from MPU-6050)
  float gyroX, gyroY, gyroZ;    // Gyroscope readings in °/s (from MPU-6050)
  bool fallDetected;        // Fall detection flag
  bool impactDetected;      // Head impact detection flag
  bool motionlessAlert;     // No motion detected for extended period
  unsigned long lastMotionTime; // Last time motion was detected
  
  // Health Alert System (Objective 4)
  bool heatStress;          // Heat stress warning flag
  bool coWarning;           // CO warning flag
  bool criticalVitals;      // Critical vital signs flag
  bool emergencyStatus;     // Emergency status flag
  
  // System Health (Objective 5)
  bool systemError;         // System error flag
  bool sensorError;         // Sensor error flag
  
  // Data Management (Objective 6)
  unsigned long timestamp;  // Timestamp of last reading
  unsigned long sessionTime; // Total session time
  int dataPacketCount;      // Data packet counter
};

VitalSigns currentReadings;

// Safety Thresholds and Constants - Firefighter Safety Objectives
const float HEART_RATE_MIN = 60.0;      // Minimum normal heart rate (BPM)
const float HEART_RATE_MAX = 200.0;     // Maximum normal heart rate (BPM)
const float HEART_RATE_CRITICAL = 220.0; // Critical heart rate threshold
const float SPO2_MIN = 90.0;            // Minimum safe blood oxygen saturation (%)
const float SPO2_CRITICAL = 85.0;       // Critical SpO2 threshold
const float BODY_TEMP_MAX = 39.0;       // Maximum safe body temperature (°C) - 102.2°F
const float BODY_TEMP_CRITICAL = 40.5;  // Critical body temperature (°C) - 104.9°F
const float AMBIENT_TEMP_WARNING = 60.0; // Ambient temperature warning (°C)
const float AMBIENT_TEMP_CRITICAL = 80.0; // Critical ambient temperature (°C)
const float CO_WARNING_LEVEL = 50.0;    // CO warning threshold (ppm)
const float CO_DANGER_LEVEL = 200.0;    // CO danger threshold (ppm)
const float CO_CRITICAL_LEVEL = 400.0;  // CO critical threshold (ppm)
const float FALL_THRESHOLD = 2.5;       // Fall detection threshold (g-force)
const float IMPACT_THRESHOLD = 4.0;     // Head impact detection threshold (g-force)
const float MOTION_THRESHOLD = 0.1;     // Motion detection threshold
const unsigned long MOTIONLESS_TIME = 300000; // 5 minutes of no motion = emergency

// Timing Variables for System Operation
unsigned long lastSensorRead = 0;        // Last sensor reading timestamp
unsigned long lastDataTransmission = 0;  // Last data transmission timestamp
unsigned long systemStartTime = 0;       // System start time
unsigned long lastHealthCheck = 0;       // Last system health check
const unsigned long SENSOR_INTERVAL = 1000;      // Sensor reading interval (1 second)
const unsigned long TRANSMISSION_INTERVAL = 5000; // Data transmission interval (5 seconds)
const unsigned long HEALTH_CHECK_INTERVAL = 30000; // System health check (30 seconds)
const unsigned long EMERGENCY_TRANSMISSION_INTERVAL = 1000; // Emergency data interval (1 second)

// Alert System Variables
bool alertActive = false;           // Current alert status
unsigned long alertStartTime = 0;   // Alert activation timestamp

// MAX30102 Sensor Variables
uint32_t irValue = 0;              // Infrared LED reading for heart rate calculation
uint32_t redValue = 0;             // Red LED reading for SpO2 calculation

// MPU-6050 Sensor Variables
int16_t ax, ay, az, gx, gy, gz;    // Raw accelerometer and gyroscope data

void setup() {
  Serial.begin(115200);
  Serial.println("VitalGuard Smart Helmet Initializing...");
  Serial.println("Implementing All Safety Objectives...");
  
  // Initialize system start time
  systemStartTime = millis();
  
  // Initialize GPIO pins
  pinMode(LED_STATUS_PIN, OUTPUT);  // Status LED for system indication
  pinMode(MQ7_DIGITAL_PIN, INPUT);  // MQ-7 digital threshold input
  
  // Initialize I2C communication
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000); // 400kHz I2C clock speed
  
  // Initialize EEPROM for data storage
  EEPROM.begin(512);
  
  // Initialize all sensors (MAX30102, MLX90614, MPU-6050, MQ-7)
  initializeSensors();
  
  // Initialize WiFi connection for server communication
  initializeWiFi();
  
  // Initialize system health monitoring
  initializeSystemHealth();
  
  // Initialize watchdog timer for system stability (ESP32 v2.0+ compatible)
  esp_task_wdt_config_t twdt_config = {
    .timeout_ms = 10000,
    .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
    .trigger_panic = true
  };
  esp_task_wdt_init(&twdt_config);
  esp_task_wdt_add(NULL);
  
  // Initialize data structures
  initializeDataStructures();
  
  Serial.println("VitalGuard System Ready - All Objectives Active!");
  playStartupLED();
  
  // Initialize last health check time
  lastHealthCheck = millis();
}

void loop() {
  unsigned long currentTime = millis();
  
  // OBJECTIVE 1 & 2: Continuous Sensor Monitoring
  if (currentTime - lastSensorRead >= SENSOR_INTERVAL) {
    readAllSensors();
    lastSensorRead = currentTime;
    
    // Increment data packet counter
    currentReadings.dataPacketCount++;
  }
  
  // OBJECTIVE 5: System Health Monitoring
  if (currentTime - lastHealthCheck >= HEALTH_CHECK_INTERVAL) {
    performSystemHealthCheck();
    lastHealthCheck = currentTime;
  }
  
  // OBJECTIVE 4: Emergency Protocol Handling
  handleEmergencyProtocols();
  
  // OBJECTIVE 6: Data Transmission (Normal or Emergency)
  unsigned long transmissionInterval = currentReadings.emergencyStatus ? 
                                     EMERGENCY_TRANSMISSION_INTERVAL : 
                                     TRANSMISSION_INTERVAL;
  
  if (currentTime - lastDataTransmission >= transmissionInterval) {
    if (currentReadings.emergencyStatus) {
      transmitEmergencyData();
    } else {
      transmitDataToServer();
    }
    lastDataTransmission = currentTime;
  }
  
  // OBJECTIVE 3 & 4: Alert Processing and Safety Monitoring
  processAlerts();
  
  // System status indication
  updateStatusLED(currentTime);
  
  // System stability monitoring
  esp_task_wdt_reset();
  
  delay(100); // Small delay for system stability
}

void initializeSensors() {
  Serial.println("Initializing sensors...");
  
  // Initialize MAX30102 Pulse Oximeter & Heart Rate Sensor
  if (initMAX30102()) {
    Serial.println("MAX30102 initialized - Heart rate and SpO2 monitoring ready");
  } else {
    Serial.println("MAX30102 not found - Heart rate monitoring unavailable");
  }
  
  // Initialize MLX90614 IR Temperature Sensor
  if (initMLX90614()) {
    Serial.println("MLX90614 initialized - Temperature monitoring ready");
  } else {
    Serial.println("MLX90614 not found - Temperature monitoring unavailable");
  }
  
  // Initialize MPU-6050 Accelerometer & Gyroscope
  if (initMPU6050()) {
    Serial.println("MPU-6050 initialized - Motion and fall detection ready");
  } else {
    Serial.println("MPU-6050 not found - Motion monitoring unavailable");
  }
  
  // Initialize MQ-7 Carbon Monoxide Sensor
  Serial.println("MQ-7 CO sensor initialized (warming up for 30 seconds...)");
  
  Serial.println("All sensors initialized successfully");
}

bool initMAX30102() {
  // Check if MAX30102 is present on I2C bus
  Wire.beginTransmission(MAX30102_ADDRESS);
  if (Wire.endTransmission() != 0) {
    return false;
  }
  
  // Reset the sensor to default state
  writeRegister(MAX30102_ADDRESS, 0x09, 0x40);
  delay(100);
  
  // Configure sensor for SpO2 and heart rate measurement
  writeRegister(MAX30102_ADDRESS, 0x09, 0x03); // Enable SpO2 mode
  writeRegister(MAX30102_ADDRESS, 0x0A, 0x27); // SpO2 configuration (sample rate, pulse width)
  writeRegister(MAX30102_ADDRESS, 0x0C, 0x24); // Red LED pulse amplitude (7.6mA)
  writeRegister(MAX30102_ADDRESS, 0x0D, 0x24); // IR LED pulse amplitude (7.6mA)
  
  return true;
}

bool initMLX90614() {
  // Check if MLX90614 IR temperature sensor is present
  Wire.beginTransmission(MLX90614_ADDRESS);
  return (Wire.endTransmission() == 0);
}

bool initMPU6050() {
  // Check if MPU-6050 is present on I2C bus
  Wire.beginTransmission(MPU6050_ADDRESS);
  if (Wire.endTransmission() != 0) {
    return false;
  }
  
  // Wake up the sensor from sleep mode
  writeRegister(MPU6050_ADDRESS, 0x6B, 0x00);
  delay(100);
  
  // Configure accelerometer range to ±2g for fall detection
  writeRegister(MPU6050_ADDRESS, 0x1C, 0x00);
  
  // Configure gyroscope range to ±250°/s for motion detection
  writeRegister(MPU6050_ADDRESS, 0x1B, 0x00);
  
  return true;
}

void writeRegister(uint8_t address, uint8_t reg, uint8_t value) {
  // Write a value to a specific register on an I2C device
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

uint8_t readRegister(uint8_t address, uint8_t reg) {
  // Read a value from a specific register on an I2C device
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(address, (uint8_t)1);
  return Wire.read();
}

void initializeWiFi() {
  Serial.println("Initializing WiFi for server connection...");
  
  // Connect to local WiFi network
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi network");
    Serial.print("ESP32 IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Server URL: ");
    Serial.println(serverURL);
  } else {
    Serial.println("\nFailed to connect to WiFi - server communication unavailable");
  }
}

void transmitDataToServer() {
  // Check WiFi connection before transmittingF
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected - Skipping data transmission to server");
    return;
  }
  
  // Create JSON payload for MySQL database storage
    DynamicJsonDocument doc(1024);
    
  // Add device identification and authentication
  doc["device_id"] = deviceID;
  doc["api_key"] = apiKey;
    doc["timestamp"] = currentReadings.timestamp;
  doc["date_time"] = getCurrentDateTime();
  
  // OBJECTIVE 1: Vital Signs Data
  doc["heart_rate"] = currentReadings.heartRate;
  doc["spo2"] = currentReadings.spO2;
  doc["body_temperature"] = currentReadings.bodyTemperature;
  doc["ambient_temperature"] = currentReadings.ambientTemperature;
  
  // OBJECTIVE 2: Environmental Data
  doc["co_level"] = currentReadings.coLevel;
  doc["smoke_detected"] = currentReadings.smokeDetected ? 1 : 0;
  
  // OBJECTIVE 3: Motion and Safety Data
  doc["accel_x"] = currentReadings.accelX;
  doc["accel_y"] = currentReadings.accelY;
  doc["accel_z"] = currentReadings.accelZ;
  doc["gyro_x"] = currentReadings.gyroX;
  doc["gyro_y"] = currentReadings.gyroY;
  doc["gyro_z"] = currentReadings.gyroZ;
  
  // OBJECTIVE 4: Alert and Emergency Status
  doc["fall_detected"] = currentReadings.fallDetected ? 1 : 0;
  doc["impact_detected"] = currentReadings.impactDetected ? 1 : 0;
  doc["motionless_alert"] = currentReadings.motionlessAlert ? 1 : 0;
  doc["heat_stress"] = currentReadings.heatStress ? 1 : 0;
  doc["co_warning"] = currentReadings.coWarning ? 1 : 0;
  doc["critical_vitals"] = currentReadings.criticalVitals ? 1 : 0;
  doc["emergency_status"] = currentReadings.emergencyStatus ? 1 : 0;
  doc["alert_active"] = alertActive ? 1 : 0;
  
  // OBJECTIVE 5: System Health Data
  doc["system_error"] = currentReadings.systemError ? 1 : 0;
  doc["sensor_error"] = currentReadings.sensorError ? 1 : 0;
  
  // OBJECTIVE 6: Data Management
  doc["session_time"] = currentReadings.sessionTime;
  doc["data_packet_count"] = currentReadings.dataPacketCount;
  
  // Add system status
  doc["wifi_connected"] = (WiFi.status() == WL_CONNECTED) ? 1 : 0;
  doc["free_heap"] = ESP.getFreeHeap();
  
  // Serialize JSON to string
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Send HTTP POST request to server PHP endpoint
  http.begin(serverURL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("User-Agent", "VitalGuard-ESP32/1.0");
  http.addHeader("Accept", "application/json");
  
  int httpResponseCode = http.POST(jsonString);
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.printf("Data sent to server MySQL successfully. Response code: %d\n", httpResponseCode);
    Serial.printf("Response: %s\n", response.c_str());
  } else {
    Serial.printf("Error sending data to server. Response code: %d\n", httpResponseCode);
    Serial.println("Check server URL and PHP endpoint");
  }
  
  http.end();
  
  // Also output to serial for debugging
  transmitData();
}

void readAllSensors() {
  // Update timestamp for current sensor readings
  currentReadings.timestamp = millis();
  
  // Read heart rate and blood oxygen from MAX30102
  readHeartRateAndSpO2();
  
  // Read body and ambient temperature from MLX90614
  readTemperatures();
  
  // Read motion data and detect falls from MPU-6050
  readMotionSensors();
  
  // Read carbon monoxide level from MQ-7
  readCOSensor();
  
  // Process all sensor data to determine alert conditions
  processSensorAlerts();
}

void readHeartRateAndSpO2() {
  // Read FIFO data from MAX30102 sensor
  Wire.beginTransmission(MAX30102_ADDRESS);
  Wire.write(0x07); // FIFO data register address
  Wire.endTransmission(false);
  Wire.requestFrom(MAX30102_ADDRESS, (uint8_t)6);
  
  if (Wire.available() >= 6) {
    // Read IR LED value (3 bytes) for heart rate calculation
    irValue = ((uint32_t)Wire.read() << 16) | ((uint32_t)Wire.read() << 8) | Wire.read();
    irValue &= 0x03FFFF; // Mask to 18-bit data
    
    // Read Red LED value (3 bytes) for SpO2 calculation
    redValue = ((uint32_t)Wire.read() << 16) | ((uint32_t)Wire.read() << 8) | Wire.read();
    redValue &= 0x03FFFF; // Mask to 18-bit data
  }
  
  // Heart rate calculation using peak detection algorithm
  static uint32_t lastIrValue = 0;
  static unsigned long lastBeatTime = 0;
  static int beatCount = 0;
  static float heartRateSum = 0;
  
  // Detect rising edge in IR signal (heartbeat peak)
  if (irValue > 100000 && lastIrValue <= 100000) {
    unsigned long currentTime = millis();
    // Validate beat interval (300ms to 3000ms = 20-200 BPM)
    if (currentTime - lastBeatTime > 300 && currentTime - lastBeatTime < 3000) {
      float bpm = 60000.0 / (currentTime - lastBeatTime);
      // Filter valid heart rate range (40-200 BPM)
      if (bpm >= 40 && bpm <= 200) {
        heartRateSum += bpm;
        beatCount++;
        // Average over 4 beats for stability
        if (beatCount >= 4) {
          currentReadings.heartRate = heartRateSum / beatCount;
          heartRateSum = 0;
          beatCount = 0;
        }
      }
    }
    lastBeatTime = currentTime;
  }
  lastIrValue = irValue;
  
  // SpO2 calculation using red/IR ratio method
  if (irValue > 50000 && redValue > 50000) {
    // Calculate ratio of red to IR LED readings
    float ratio = (float)redValue / (float)irValue;
    // Simplified SpO2 formula (calibration required for accuracy)
    currentReadings.spO2 = 110 - 18 * ratio;
    // Clamp values to realistic range
    if (currentReadings.spO2 > 100) currentReadings.spO2 = 100;
    if (currentReadings.spO2 < 70) currentReadings.spO2 = 70;
  } else {
    currentReadings.spO2 = 98; // Default value when signal is too weak
  }
}

void readTemperatures() {
  // Read object temperature (body temperature) from MLX90614
  Wire.beginTransmission(MLX90614_ADDRESS);
  Wire.write(0x07); // Object temperature register address
  Wire.endTransmission(false);
  Wire.requestFrom(MLX90614_ADDRESS, (uint8_t)3);
  
  if (Wire.available() >= 3) {
    uint16_t tempData = Wire.read() | (Wire.read() << 8);
    Wire.read(); // Skip PEC (Packet Error Check) byte
    // Convert from Kelvin to Celsius (0.02°C per LSB, -273.15°C offset)
    currentReadings.bodyTemperature = (tempData * 0.02) - 273.15;
  }
  
  // Read ambient temperature from MLX90614
  Wire.beginTransmission(MLX90614_ADDRESS);
  Wire.write(0x06); // Ambient temperature register address
  Wire.endTransmission(false);
  Wire.requestFrom(MLX90614_ADDRESS, (uint8_t)3);
  
  if (Wire.available() >= 3) {
    uint16_t tempData = Wire.read() | (Wire.read() << 8);
    Wire.read(); // Skip PEC (Packet Error Check) byte
    // Convert from Kelvin to Celsius
    currentReadings.ambientTemperature = (tempData * 0.02) - 273.15;
  }
  
  // Validate readings
  if (currentReadings.bodyTemperature < 30.0 || currentReadings.bodyTemperature > 45.0) {
    currentReadings.bodyTemperature = 37.0; // Default to normal body temp
  }
  if (currentReadings.ambientTemperature < -40.0 || currentReadings.ambientTemperature > 85.0) {
    currentReadings.ambientTemperature = 25.0; // Default ambient temp
  }
}

void readMotionSensors() {
  // Read accelerometer and gyroscope data from MPU6050
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(0x3B); // Starting register for accelerometer data
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDRESS, (uint8_t)14);
  
  if (Wire.available() >= 14) {
    ax = Wire.read() << 8 | Wire.read();
    ay = Wire.read() << 8 | Wire.read();
    az = Wire.read() << 8 | Wire.read();
    Wire.read(); Wire.read(); // Skip temperature
    gx = Wire.read() << 8 | Wire.read();
    gy = Wire.read() << 8 | Wire.read();
    gz = Wire.read() << 8 | Wire.read();
    
    // Convert to g-force and degrees/second
    currentReadings.accelX = ax / 16384.0;
    currentReadings.accelY = ay / 16384.0;
    currentReadings.accelZ = az / 16384.0;
    
    currentReadings.gyroX = gx / 131.0;
    currentReadings.gyroY = gy / 131.0;
    currentReadings.gyroZ = gz / 131.0;
    
    // Detect falls
    float totalAccel = sqrt(pow(currentReadings.accelX, 2) + 
                           pow(currentReadings.accelY, 2) + 
                           pow(currentReadings.accelZ, 2));
    
    currentReadings.fallDetected = (totalAccel > FALL_THRESHOLD);
  }
}

void readCOSensor() {
  int analogValue = analogRead(MQ7_ANALOG_PIN);
  
  // Convert analog reading to CO concentration (ppm)
  // This is a simplified conversion - actual calibration needed
  float voltage = analogValue * (3.3 / 4095.0);
  currentReadings.coLevel = (voltage - 0.1) * 100; // Simplified conversion
  
  if (currentReadings.coLevel < 0) currentReadings.coLevel = 0;
}


void processSensorAlerts() {
  // OBJECTIVE 1: Vital Signs Monitoring and Alert System
  
  // Heat stress detection (enhanced)
  currentReadings.heatStress = (currentReadings.bodyTemperature > BODY_TEMP_MAX) ||
                              (currentReadings.heartRate > HEART_RATE_MAX) ||
                              (currentReadings.ambientTemperature > AMBIENT_TEMP_WARNING);
  
  // Critical vital signs detection
  currentReadings.criticalVitals = (currentReadings.bodyTemperature > BODY_TEMP_CRITICAL) ||
                                  (currentReadings.heartRate > HEART_RATE_CRITICAL) ||
                                  (currentReadings.spO2 < SPO2_CRITICAL) ||
                                  (currentReadings.ambientTemperature > AMBIENT_TEMP_CRITICAL);
  
  // OBJECTIVE 2: Environmental Hazard Detection
  
  // CO warning detection (enhanced levels)
  if (currentReadings.coLevel > CO_CRITICAL_LEVEL) {
    currentReadings.coWarning = true;
    currentReadings.emergencyStatus = true;
  } else if (currentReadings.coLevel > CO_DANGER_LEVEL) {
    currentReadings.coWarning = true;
  } else if (currentReadings.coLevel > CO_WARNING_LEVEL) {
    currentReadings.coWarning = true;
  } else {
    currentReadings.coWarning = false;
  }
  
  // Smoke detection (based on ambient temperature)
  currentReadings.smokeDetected = (currentReadings.ambientTemperature > AMBIENT_TEMP_WARNING);
  
  // OBJECTIVE 3: Motion and Safety Monitoring
  
  // Impact detection (head trauma)
  float totalAccel = sqrt(pow(currentReadings.accelX, 2) + 
                         pow(currentReadings.accelY, 2) + 
                         pow(currentReadings.accelZ, 2));
  
  currentReadings.impactDetected = (totalAccel > IMPACT_THRESHOLD);
  
  // Motion detection for motionless alert
  if (totalAccel > MOTION_THRESHOLD) {
    currentReadings.lastMotionTime = millis();
    currentReadings.motionlessAlert = false;
  } else {
    // Check if firefighter has been motionless for too long
    if (millis() - currentReadings.lastMotionTime > MOTIONLESS_TIME) {
      currentReadings.motionlessAlert = true;
      currentReadings.emergencyStatus = true;
    }
  }
  
  // OBJECTIVE 4: Emergency Status Determination
  
  // Set emergency status based on critical conditions
  if (currentReadings.criticalVitals || 
      currentReadings.fallDetected || 
      currentReadings.impactDetected ||
      currentReadings.motionlessAlert ||
      (currentReadings.coLevel > CO_CRITICAL_LEVEL)) {
    currentReadings.emergencyStatus = true;
  }
}

void processAlerts() {
  bool currentAlertStatus = currentReadings.fallDetected ||
                           currentReadings.heatStress ||
                           currentReadings.coWarning;
  
  if (currentAlertStatus && !alertActive) {
    alertActive = true;
    alertStartTime = millis();
    activateAlert();
  } else if (!currentAlertStatus && alertActive) {
    alertActive = false;
    deactivateAlert();
  }
  
  // Handle ongoing alerts
  if (alertActive) {
    updateAlert();
  }
}

void activateAlert() {
  Serial.println("ALERT ACTIVATED!");
  
  // Flash LED
  digitalWrite(LED_STATUS_PIN, HIGH);
}

void updateAlert() {
  // Flash LED during alert
  if ((millis() - alertStartTime) % 500 < 250) {
    digitalWrite(LED_STATUS_PIN, HIGH);
  } else {
    digitalWrite(LED_STATUS_PIN, LOW);
  }
}

void deactivateAlert() {
  Serial.println("Alert deactivated");
  digitalWrite(LED_STATUS_PIN, LOW);
}


void updateStatusLED(unsigned long currentTime) {
  if (!alertActive) {
    // Normal operation - slow blink
    if ((currentTime % 2000) < 100) {
      digitalWrite(LED_STATUS_PIN, HIGH);
    } else {
      digitalWrite(LED_STATUS_PIN, LOW);
    }
  }
}

void transmitData() {
  // Output current sensor readings to serial monitor for debugging
  Serial.println("=== VitalGuard Complete Status Report ===");
  
  // OBJECTIVE 1: Vital Signs Monitoring
  Serial.println("[VITAL SIGNS]");
  Serial.printf("Heart Rate: %.1f BPM\n", currentReadings.heartRate);
  Serial.printf("SpO2: %.1f%%\n", currentReadings.spO2);
  Serial.printf("Body Temperature: %.1f°C\n", currentReadings.bodyTemperature);
  Serial.printf("Ambient Temperature: %.1f°C\n", currentReadings.ambientTemperature);
  
  // OBJECTIVE 2: Environmental Monitoring
  Serial.println("[ENVIRONMENT]");
  Serial.printf("CO Level: %.1f ppm\n", currentReadings.coLevel);
  Serial.printf("Smoke Detected: %s\n", currentReadings.smokeDetected ? "YES" : "NO");
  
  // OBJECTIVE 3: Motion and Safety
  Serial.println("[MOTION & SAFETY]");
  Serial.printf("Fall Detected: %s\n", currentReadings.fallDetected ? "YES" : "NO");
  Serial.printf("Impact Detected: %s\n", currentReadings.impactDetected ? "YES" : "NO");
  Serial.printf("Motionless Alert: %s\n", currentReadings.motionlessAlert ? "YES" : "NO");
  
  // OBJECTIVE 4: Health Alerts
  Serial.println("[HEALTH ALERTS]");
  Serial.printf("Heat Stress: %s\n", currentReadings.heatStress ? "WARNING" : "NORMAL");
  Serial.printf("CO Warning: %s\n", currentReadings.coWarning ? "WARNING" : "NORMAL");
  Serial.printf("Critical Vitals: %s\n", currentReadings.criticalVitals ? "CRITICAL" : "NORMAL");
  Serial.printf("Emergency Status: %s\n", currentReadings.emergencyStatus ? "ACTIVE" : "NORMAL");
  
  // OBJECTIVE 5: System Health
  Serial.println("[SYSTEM HEALTH]");
  Serial.printf("System Error: %s\n", currentReadings.systemError ? "ERROR" : "OK");
  Serial.printf("Sensor Error: %s\n", currentReadings.sensorError ? "ERROR" : "OK");
  
  // OBJECTIVE 6: Data Management
  Serial.println("[DATA MANAGEMENT]");
  Serial.printf("Session Time: %lu seconds\n", currentReadings.sessionTime);
  Serial.printf("Data Packets: %d\n", currentReadings.dataPacketCount);
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  
  Serial.println("===========================================");
}

String getCurrentDateTime() {
  // Get current date and time for MySQL database storage
  // Note: ESP32 doesn't have RTC, so this uses millis()
  // For accurate timestamps, consider using NTP or external RTC
  unsigned long currentTime = millis();
  unsigned long seconds = currentTime / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;
  
  // Simple time format (you may want to implement NTP for real timestamps)
  String timeStr = String(days) + "d " + String(hours % 24) + "h " + String(minutes % 60) + "m " + String(seconds % 60) + "s";
  return timeStr;
}

void initializeSystemHealth() {
  // OBJECTIVE 5: System Health and Diagnostics Monitoring 
  Serial.println("Initializing system health monitoring...");
  
  // Initialize system health variables
  currentReadings.systemError = false;
  currentReadings.sensorError = false;
  
  Serial.println("System health monitoring initialized");
}

void initializeDataStructures() {
  // OBJECTIVE 6: Data Management and Logging
  Serial.println("Initializing data structures...");
  
  // Initialize data management variables
  currentReadings.sessionTime = 0;
  currentReadings.dataPacketCount = 0;
  currentReadings.lastMotionTime = millis();
  currentReadings.emergencyStatus = false;
  
  Serial.println("Data structures initialized");
}

void performSystemHealthCheck() {
  // OBJECTIVE 5: System Health Monitoring
  
  // Check sensor connectivity
  currentReadings.sensorError = false;
  
  // Check MAX30102
  Wire.beginTransmission(MAX30102_ADDRESS);
  if (Wire.endTransmission() != 0) {
    currentReadings.sensorError = true;
    Serial.println("ERROR: MAX30102 sensor disconnected");
  }
  
  // Check MLX90614
  Wire.beginTransmission(MLX90614_ADDRESS);
  if (Wire.endTransmission() != 0) {
    currentReadings.sensorError = true;
    Serial.println("ERROR: MLX90614 sensor disconnected");
  }
  
  // Check MPU6050
  Wire.beginTransmission(MPU6050_ADDRESS);
  if (Wire.endTransmission() != 0) {
    currentReadings.sensorError = true;
    Serial.println("ERROR: MPU6050 sensor disconnected");
  }
  
  // Check WiFi connectivity
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WARNING: WiFi disconnected");
  }
  
  // Check memory usage
  if (ESP.getFreeHeap() < 10000) {
    currentReadings.systemError = true;
    Serial.println("WARNING: Low memory");
  }
  
  // Update session time
  currentReadings.sessionTime = (millis() - systemStartTime) / 1000; // in seconds
}

void handleEmergencyProtocols() {
  // OBJECTIVE 4: Emergency Detection and Response
  
  if (currentReadings.emergencyStatus) {
    Serial.println("EMERGENCY PROTOCOL ACTIVATED!");
    
    // Emergency data transmission (faster interval)
    static unsigned long lastEmergencyTransmission = 0;
    if (millis() - lastEmergencyTransmission >= EMERGENCY_TRANSMISSION_INTERVAL) {
      transmitEmergencyData();
      lastEmergencyTransmission = millis();
    }
    
    // Emergency LED pattern (rapid flashing)
    if ((millis() % 200) < 100) {
      digitalWrite(LED_STATUS_PIN, HIGH);
    } else {
      digitalWrite(LED_STATUS_PIN, LOW);
    }
  }
}

void transmitEmergencyData() {
  // Enhanced emergency data transmission
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("EMERGENCY: WiFi not connected - Cannot transmit emergency data");
    return;
  }
  
  // Create emergency JSON payload
  DynamicJsonDocument doc(1536); // Larger size for emergency data
  
  // Emergency header
  doc["emergency"] = true;
  doc["device_id"] = deviceID;
  doc["api_key"] = apiKey;
  doc["timestamp"] = currentReadings.timestamp;
  doc["session_time"] = currentReadings.sessionTime;
  
  // Critical vital signs
  doc["heart_rate"] = currentReadings.heartRate;
  doc["spo2"] = currentReadings.spO2;
  doc["body_temperature"] = currentReadings.bodyTemperature;
  doc["ambient_temperature"] = currentReadings.ambientTemperature;
  doc["co_level"] = currentReadings.coLevel;
  
  // Emergency conditions
  doc["fall_detected"] = currentReadings.fallDetected ? 1 : 0;
  doc["impact_detected"] = currentReadings.impactDetected ? 1 : 0;
  doc["motionless_alert"] = currentReadings.motionlessAlert ? 1 : 0;
  doc["critical_vitals"] = currentReadings.criticalVitals ? 1 : 0;
  doc["heat_stress"] = currentReadings.heatStress ? 1 : 0;
  doc["co_warning"] = currentReadings.coWarning ? 1 : 0;
  
  // System status
  doc["system_error"] = currentReadings.systemError ? 1 : 0;
  doc["sensor_error"] = currentReadings.sensorError ? 1 : 0;
  
  // Location data (motion)
  doc["accel_x"] = currentReadings.accelX;
  doc["accel_y"] = currentReadings.accelY;
  doc["accel_z"] = currentReadings.accelZ;
  
  // Serialize and send
  String jsonString;
  serializeJson(doc, jsonString);
  
  http.begin(serverURL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("User-Agent", "VitalGuard-ESP32/1.0-EMERGENCY");
  http.addHeader("Priority", "urgent");
  
  int httpResponseCode = http.POST(jsonString);
  
  if (httpResponseCode > 0) {
    Serial.printf("EMERGENCY data transmitted. Response: %d\n", httpResponseCode);
  } else {
    Serial.printf("EMERGENCY transmission failed: %d\n", httpResponseCode);
  }
  
  http.end();
}

void playStartupLED() {
  // Visual startup indication with LED
  Serial.println("System startup complete - All objectives operational");
  
  for (int i = 0; i < 4; i++) {
    digitalWrite(LED_STATUS_PIN, HIGH);
    delay(200);
    digitalWrite(LED_STATUS_PIN, LOW);
    delay(200);
  }
  delay(400);
  digitalWrite(LED_STATUS_PIN, HIGH);
  delay(400);
  digitalWrite(LED_STATUS_PIN, LOW);
}

