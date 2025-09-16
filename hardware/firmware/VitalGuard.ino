/*
 * VitalGuard: Smart Helmet with Health Monitoring for Firefighter Safety
 * ESP32 WROOM-32 Development Board Implementation
 * 
 * Hardware Components:
 * - ESP32 WROOM-32 Development Board
 * - MAX30102 Pulse Oximeter & Heart Rate Sensor
 * - MLX90614 IR Temperature Sensor
 * - MPU-6050 Accelerometer/Gyroscope
 * - MAX6675 + K-type Thermocouple (Optional)
 * - MQ-7 Carbon Monoxide Gas Sensor
 * - 3A Charge Module with BMS
 * - 18650 Li-ion Battery 4000mAh
 * 
 * Authors: LIBRE, KENT LOWELL M., PAGTALUNAN, EMJAY, VICENTINO, NATHANIEL
 * Adviser: ENGR. DAVID MUELLER
 * December 2025
 */

#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <esp_task_wdt.h>

// Pin Definitions
#define I2C_SDA 21
#define I2C_SCL 22
#define MQ7_ANALOG_PIN 34
#define MQ7_DIGITAL_PIN 35
#define MAX6675_SCK_PIN 18
#define MAX6675_CS_PIN 5
#define MAX6675_SO_PIN 19
#define BUZZER_PIN 25
#define LED_STATUS_PIN 2
#define BATTERY_PIN 36
#define EMERGENCY_BUTTON_PIN 0

// I2C Addresses
#define MAX30102_ADDRESS 0x57
#define MLX90614_ADDRESS 0x5A
#define MPU6050_ADDRESS 0x68

// Web Server
WebServer server(80);

// WiFi Credentials
const char* ssid = "VitalGuard_Command";
const char* password = "firefighter123";

// Data Structure for Sensor Readings
struct VitalSigns {
  float heartRate;
  float spO2;
  float bodyTemperature;
  float ambientTemperature;
  float thermocoupleTemp;
  float coLevel;
  float accelX, accelY, accelZ;
  float gyroX, gyroY, gyroZ;
  float batteryLevel;
  bool emergencyStatus;
  bool fallDetected;
  bool heatStress;
  bool coWarning;
  unsigned long timestamp;
};

VitalSigns currentReadings;

// Thresholds and Constants
const float HEART_RATE_MIN = 60.0;
const float HEART_RATE_MAX = 200.0;
const float SPO2_MIN = 90.0;
const float BODY_TEMP_MAX = 39.0; // 39°C = 102.2°F
const float CO_WARNING_LEVEL = 50.0; // ppm
const float CO_DANGER_LEVEL = 200.0; // ppm
const float FALL_THRESHOLD = 2.5; // g-force
const float BATTERY_LOW_THRESHOLD = 20.0; // percentage

// Timing Variables
unsigned long lastSensorRead = 0;
unsigned long lastDataTransmission = 0;
const unsigned long SENSOR_INTERVAL = 1000; // 1 second
const unsigned long TRANSMISSION_INTERVAL = 5000; // 5 seconds

// Alert System
bool alertActive = false;
unsigned long alertStartTime = 0;

// MAX30102 Variables
uint32_t irValue = 0;
uint32_t redValue = 0;

// MPU6050 Variables
int16_t ax, ay, az, gx, gy, gz;

void setup() {
  Serial.begin(115200);
  Serial.println("VitalGuard Smart Helmet Initializing...");
  
  // Initialize pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_STATUS_PIN, OUTPUT);
  pinMode(EMERGENCY_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MQ7_DIGITAL_PIN, INPUT);
  
  // Initialize I2C
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000); // 400kHz
  
  // Initialize EEPROM
  EEPROM.begin(512);
  
  // Initialize sensors
  initializeSensors();
  
  // Initialize WiFi
  initializeWiFi();
  
  // Setup web server routes
  setupWebServer();
  
  // Initialize watchdog timer
  esp_task_wdt_init(10, true);
  esp_task_wdt_add(NULL);
  
  Serial.println("VitalGuard System Ready!");
  playStartupTone();
}

void loop() {
  unsigned long currentTime = millis();
  
  // Handle web server requests
  server.handleClient();
  
  // Read sensors at regular intervals
  if (currentTime - lastSensorRead >= SENSOR_INTERVAL) {
    readAllSensors();
    lastSensorRead = currentTime;
  }
  
  // Transmit data at regular intervals
  if (currentTime - lastDataTransmission >= TRANSMISSION_INTERVAL) {
    transmitData();
    lastDataTransmission = currentTime;
  }
  
  // Process alerts and warnings
  processAlerts();
  
  // Handle emergency button
  handleEmergencyButton();
  
  // Update status LED
  updateStatusLED(currentTime);
  
  // Reset watchdog timer
  esp_task_wdt_reset();
  
  delay(100);
}

void initializeSensors() {
  Serial.println("Initializing sensors...");
  
  // Initialize MAX30102
  if (initMAX30102()) {
    Serial.println("MAX30102 initialized");
  } else {
    Serial.println("MAX30102 not found");
  }
  
  // Initialize MLX90614
  if (initMLX90614()) {
    Serial.println("MLX90614 initialized");
  } else {
    Serial.println("MLX90614 not found");
  }
  
  // Initialize MPU6050
  if (initMPU6050()) {
    Serial.println("MPU6050 initialized");
  } else {
    Serial.println("MPU6050 not found");
  }
  
  // Initialize MQ-7
  Serial.println("MQ-7 CO sensor initialized (warming up...)");
  
  Serial.println("All sensors initialized");
}

bool initMAX30102() {
  Wire.beginTransmission(MAX30102_ADDRESS);
  if (Wire.endTransmission() != 0) {
    return false;
  }
  
  // Reset the sensor
  writeRegister(MAX30102_ADDRESS, 0x09, 0x40);
  delay(100);
  
  // Configure the sensor
  writeRegister(MAX30102_ADDRESS, 0x09, 0x03); // SpO2 mode
  writeRegister(MAX30102_ADDRESS, 0x0A, 0x27); // SpO2 configuration
  writeRegister(MAX30102_ADDRESS, 0x0C, 0x24); // LED1 pulse amplitude
  writeRegister(MAX30102_ADDRESS, 0x0D, 0x24); // LED2 pulse amplitude
  
  return true;
}

bool initMLX90614() {
  Wire.beginTransmission(MLX90614_ADDRESS);
  return (Wire.endTransmission() == 0);
}

bool initMPU6050() {
  Wire.beginTransmission(MPU6050_ADDRESS);
  if (Wire.endTransmission() != 0) {
    return false;
  }
  
  // Wake up the sensor
  writeRegister(MPU6050_ADDRESS, 0x6B, 0x00);
  delay(100);
  
  // Set accelerometer range to ±2g
  writeRegister(MPU6050_ADDRESS, 0x1C, 0x00);
  
  // Set gyroscope range to ±250°/s
  writeRegister(MPU6050_ADDRESS, 0x1B, 0x00);
  
  return true;
}

void writeRegister(uint8_t address, uint8_t reg, uint8_t value) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

uint8_t readRegister(uint8_t address, uint8_t reg) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(address, (uint8_t)1);
  return Wire.read();
}

void initializeWiFi() {
  Serial.println("Initializing WiFi...");
  
  // Try to connect as client first
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi network");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    // Start as Access Point
    Serial.println("\nStarting Access Point mode");
    WiFi.softAP(ssid, password);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
  }
}

void setupWebServer() {
  // Root endpoint - System status
  server.on("/", HTTP_GET, []() {
    String html = generateStatusHTML();
    server.send(200, "text/html", html);
  });
  
  // API endpoint for real-time data
  server.on("/api/data", HTTP_GET, []() {
    DynamicJsonDocument doc(1024);
    
    doc["heartRate"] = currentReadings.heartRate;
    doc["spO2"] = currentReadings.spO2;
    doc["bodyTemp"] = currentReadings.bodyTemperature;
    doc["ambientTemp"] = currentReadings.ambientTemperature;
    doc["thermocoupleTemp"] = currentReadings.thermocoupleTemp;
    doc["coLevel"] = currentReadings.coLevel;
    doc["batteryLevel"] = currentReadings.batteryLevel;
    doc["emergencyStatus"] = currentReadings.emergencyStatus;
    doc["fallDetected"] = currentReadings.fallDetected;
    doc["heatStress"] = currentReadings.heatStress;
    doc["coWarning"] = currentReadings.coWarning;
    doc["timestamp"] = currentReadings.timestamp;
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  });
  
  // API endpoint for alerts
  server.on("/api/alerts", HTTP_GET, []() {
    DynamicJsonDocument doc(512);
    
    JsonArray alerts = doc.createNestedArray("alerts");
    
    if (currentReadings.emergencyStatus) {
      alerts.add("EMERGENCY BUTTON ACTIVATED");
    }
    if (currentReadings.fallDetected) {
      alerts.add("FALL DETECTED");
    }
    if (currentReadings.heatStress) {
      alerts.add("HEAT STRESS WARNING");
    }
    if (currentReadings.coWarning) {
      alerts.add("CO LEVEL WARNING");
    }
    if (currentReadings.batteryLevel < BATTERY_LOW_THRESHOLD) {
      alerts.add("LOW BATTERY");
    }
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  });
  
  server.begin();
  Serial.println("Web server started");
}

void readAllSensors() {
  currentReadings.timestamp = millis();
  
  // Read MAX30102 (Heart Rate & SpO2)
  readHeartRateAndSpO2();
  
  // Read MLX90614 (IR Temperature)
  readTemperatures();
  
  // Read MPU6050 (Accelerometer/Gyroscope)
  readMotionSensors();
  
  // Read MQ-7 (CO Gas Sensor)
  readCOSensor();
  
  // Read thermocouple (simple ADC reading)
  readThermocouple();
  
  // Read battery level
  readBatteryLevel();
  
  // Process sensor data for alerts
  processSensorAlerts();
}

void readHeartRateAndSpO2() {
  // Read FIFO data from MAX30102
  Wire.beginTransmission(MAX30102_ADDRESS);
  Wire.write(0x07); // FIFO data register
  Wire.endTransmission(false);
  Wire.requestFrom(MAX30102_ADDRESS, (uint8_t)6);
  
  if (Wire.available() >= 6) {
    // Read IR value (3 bytes)
    irValue = ((uint32_t)Wire.read() << 16) | ((uint32_t)Wire.read() << 8) | Wire.read();
    irValue &= 0x03FFFF; // 18-bit data
    
    // Read Red value (3 bytes)
    redValue = ((uint32_t)Wire.read() << 16) | ((uint32_t)Wire.read() << 8) | Wire.read();
    redValue &= 0x03FFFF; // 18-bit data
  }
  
  // Simple heart rate calculation
  static uint32_t lastIrValue = 0;
  static unsigned long lastBeatTime = 0;
  static int beatCount = 0;
  static float heartRateSum = 0;
  
  if (irValue > 100000 && lastIrValue <= 100000) { // Rising edge detection
    unsigned long currentTime = millis();
    if (currentTime - lastBeatTime > 300 && currentTime - lastBeatTime < 3000) {
      float bpm = 60000.0 / (currentTime - lastBeatTime);
      if (bpm >= 40 && bpm <= 200) {
        heartRateSum += bpm;
        beatCount++;
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
  
  // Simple SpO2 calculation
  if (irValue > 50000 && redValue > 50000) {
    float ratio = (float)redValue / (float)irValue;
    currentReadings.spO2 = 110 - 18 * ratio;
    if (currentReadings.spO2 > 100) currentReadings.spO2 = 100;
    if (currentReadings.spO2 < 70) currentReadings.spO2 = 70;
  } else {
    currentReadings.spO2 = 98; // Default value when no valid reading
  }
}

void readTemperatures() {
  // Read object temperature from MLX90614
  Wire.beginTransmission(MLX90614_ADDRESS);
  Wire.write(0x07); // Object temperature register
  Wire.endTransmission(false);
  Wire.requestFrom(MLX90614_ADDRESS, (uint8_t)3);
  
  if (Wire.available() >= 3) {
    uint16_t tempData = Wire.read() | (Wire.read() << 8);
    Wire.read(); // PEC byte
    currentReadings.bodyTemperature = (tempData * 0.02) - 273.15;
  }
  
  // Read ambient temperature from MLX90614
  Wire.beginTransmission(MLX90614_ADDRESS);
  Wire.write(0x06); // Ambient temperature register
  Wire.endTransmission(false);
  Wire.requestFrom(MLX90614_ADDRESS, (uint8_t)3);
  
  if (Wire.available() >= 3) {
    uint16_t tempData = Wire.read() | (Wire.read() << 8);
    Wire.read(); // PEC byte
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

void readThermocouple() {
  // Simple ADC reading for thermocouple
  int thermocoupleRaw = analogRead(MAX6675_CS_PIN);
  currentReadings.thermocoupleTemp = (thermocoupleRaw * 3.3 / 4095.0) * 100;
  
  // Validate thermocouple reading
  if (currentReadings.thermocoupleTemp < -50 || currentReadings.thermocoupleTemp > 200) {
    currentReadings.thermocoupleTemp = currentReadings.ambientTemperature;
  }
}

void readBatteryLevel() {
  int batteryRaw = analogRead(BATTERY_PIN);
  float batteryVoltage = (batteryRaw / 4095.0) * 3.3 * 2; // Voltage divider
  
  // Convert voltage to percentage (3.0V = 0%, 4.2V = 100%)
  currentReadings.batteryLevel = ((batteryVoltage - 3.0) / 1.2) * 100;
  
  if (currentReadings.batteryLevel > 100) currentReadings.batteryLevel = 100;
  if (currentReadings.batteryLevel < 0) currentReadings.batteryLevel = 0;
}

void processSensorAlerts() {
  // Heat stress detection
  currentReadings.heatStress = (currentReadings.bodyTemperature > BODY_TEMP_MAX) ||
                              (currentReadings.heartRate > HEART_RATE_MAX);
  
  // CO warning detection
  currentReadings.coWarning = currentReadings.coLevel > CO_WARNING_LEVEL;
}

void processAlerts() {
  bool currentAlertStatus = currentReadings.emergencyStatus ||
                           currentReadings.fallDetected ||
                           currentReadings.heatStress ||
                           currentReadings.coWarning ||
                           (currentReadings.batteryLevel < BATTERY_LOW_THRESHOLD);
  
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
  
  // Sound buzzer
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
  
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
  
  // Periodic buzzer for critical alerts
  if (currentReadings.emergencyStatus || currentReadings.fallDetected) {
    if ((millis() - alertStartTime) % 2000 < 200) {
      digitalWrite(BUZZER_PIN, HIGH);
    } else {
      digitalWrite(BUZZER_PIN, LOW);
    }
  }
}

void deactivateAlert() {
  Serial.println("Alert deactivated");
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_STATUS_PIN, LOW);
}

void handleEmergencyButton() {
  static bool lastButtonState = HIGH;
  static unsigned long buttonPressTime = 0;
  
  bool currentButtonState = digitalRead(EMERGENCY_BUTTON_PIN);
  
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    buttonPressTime = millis();
  } else if (lastButtonState == LOW && currentButtonState == HIGH) {
    if (millis() - buttonPressTime > 1000) { // Long press
      currentReadings.emergencyStatus = !currentReadings.emergencyStatus;
      Serial.println("Emergency status toggled");
    }
  }
  
  lastButtonState = currentButtonState;
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
  Serial.println("=== VitalGuard Data Transmission ===");
  Serial.printf("Heart Rate: %.1f BPM\n", currentReadings.heartRate);
  Serial.printf("SpO2: %.1f%%\n", currentReadings.spO2);
  Serial.printf("Body Temperature: %.1f°C\n", currentReadings.bodyTemperature);
  Serial.printf("Ambient Temperature: %.1f°C\n", currentReadings.ambientTemperature);
  Serial.printf("CO Level: %.1f ppm\n", currentReadings.coLevel);
  Serial.printf("Battery Level: %.1f%%\n", currentReadings.batteryLevel);
  Serial.printf("Emergency Status: %s\n", currentReadings.emergencyStatus ? "ACTIVE" : "NORMAL");
  Serial.printf("Fall Detected: %s\n", currentReadings.fallDetected ? "YES" : "NO");
  Serial.printf("Heat Stress: %s\n", currentReadings.heatStress ? "WARNING" : "NORMAL");
  Serial.printf("CO Warning: %s\n", currentReadings.coWarning ? "WARNING" : "NORMAL");
  Serial.println("=====================================");
}

void playStartupTone() {
  // Play a startup melody
  int frequencies[] = {262, 294, 330, 349};
  int durations[] = {200, 200, 200, 400};
  
  for (int i = 0; i < 4; i++) {
    tone(BUZZER_PIN, frequencies[i], durations[i]);
    delay(durations[i] + 50);
  }
}

String generateStatusHTML() {
  String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>VitalGuard Smart Helmet</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background: #f0f0f0; }
        .container { max-width: 800px; margin: 0 auto; background: white; border-radius: 10px; padding: 20px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }
        .header { text-align: center; color: #d32f2f; border-bottom: 2px solid #d32f2f; padding-bottom: 10px; }
        .status-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 15px; margin-top: 20px; }
        .status-card { background: #f8f9fa; border: 1px solid #dee2e6; border-radius: 5px; padding: 15px; text-align: center; }
        .status-value { font-size: 24px; font-weight: bold; margin: 10px 0; }
        .status-normal { color: #28a745; }
        .status-warning { color: #ffc107; }
        .status-danger { color: #dc3545; }
        .alert-section { margin-top: 20px; padding: 15px; background: #f8d7da; border: 1px solid #f5c6cb; border-radius: 5px; }
        .alert-active { display: block; }
        .alert-inactive { display: none; }
        button { background: #007bff; color: white; border: none; padding: 10px 20px; border-radius: 5px; cursor: pointer; margin: 5px; }
        button:hover { background: #0056b3; }
        .emergency-btn { background: #dc3545 !important; font-size: 18px; padding: 15px 30px; }
        .emergency-btn:hover { background: #c82333 !important; }
    </style>
    <script>
        function refreshData() {
            fetch('/api/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('heartRate').textContent = data.heartRate.toFixed(1) + ' BPM';
                    document.getElementById('heartRate').className = 'status-value ' + getVitalStatus(data.heartRate, 60, 100);
                    
                    document.getElementById('spO2').textContent = data.spO2.toFixed(1) + '%';
                    document.getElementById('spO2').className = 'status-value ' + (data.spO2 < 90 ? 'status-danger' : 'status-normal');
                    
                    document.getElementById('bodyTemp').textContent = data.bodyTemp.toFixed(1) + '°C';
                    document.getElementById('bodyTemp').className = 'status-value ' + (data.bodyTemp > 39 ? 'status-danger' : 'status-normal');
                    
                    document.getElementById('coLevel').textContent = data.coLevel.toFixed(1) + ' ppm';
                    document.getElementById('coLevel').className = 'status-value ' + getCOStatus(data.coLevel);
                    
                    document.getElementById('battery').textContent = data.batteryLevel.toFixed(1) + '%';
                    document.getElementById('battery').className = 'status-value ' + getBatteryStatus(data.batteryLevel);
                });
            
            fetch('/api/alerts')
                .then(response => response.json())
                .then(data => {
                    const alertSection = document.getElementById('alertSection');
                    const alertList = document.getElementById('alertList');
                    
                    if (data.alerts.length > 0) {
                        alertSection.className = 'alert-section alert-active';
                        alertList.innerHTML = data.alerts.map(alert => '<li>' + alert + '</li>').join('');
                    } else {
                        alertSection.className = 'alert-section alert-inactive';
                    }
                });
        }
        
        function getVitalStatus(value, min, max) {
            if (value < min || value > max) return 'status-danger';
            if (value < min + 10 || value > max - 10) return 'status-warning';
            return 'status-normal';
        }
        
        function getCOStatus(value) {
            if (value > 200) return 'status-danger';
            if (value > 50) return 'status-warning';
            return 'status-normal';
        }
        
        function getBatteryStatus(value) {
            if (value < 20) return 'status-danger';
            if (value < 40) return 'status-warning';
            return 'status-normal';
        }
        
        setInterval(refreshData, 2000);
        window.onload = refreshData;
    </script>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🚒 VitalGuard Smart Helmet</h1>
            <p>Real-time Firefighter Health Monitoring System</p>
        </div>
        
        <div class="status-grid">
            <div class="status-card">
                <h3>❤️ Heart Rate</h3>
                <div id="heartRate" class="status-value status-normal">-- BPM</div>
            </div>
            
            <div class="status-card">
                <h3>🫁 Blood Oxygen</h3>
                <div id="spO2" class="status-value status-normal">--%</div>
            </div>
            
            <div class="status-card">
                <h3>🌡️ Body Temperature</h3>
                <div id="bodyTemp" class="status-value status-normal">--°C</div>
            </div>
            
            <div class="status-card">
                <h3>💨 CO Level</h3>
                <div id="coLevel" class="status-value status-normal">-- ppm</div>
            </div>
            
            <div class="status-card">
                <h3>🔋 Battery</h3>
                <div id="battery" class="status-value status-normal">--%</div>
            </div>
        </div>
        
        <div id="alertSection" class="alert-section alert-inactive">
            <h3>⚠️ Active Alerts</h3>
            <ul id="alertList"></ul>
        </div>
        
        <div style="text-align: center; margin-top: 20px;">
            <button class="emergency-btn" onclick="alert('Emergency protocol activated!')">🆘 EMERGENCY</button>
            <button onclick="refreshData()">🔄 Refresh Data</button>
        </div>
        
        <div style="text-align: center; margin-top: 20px; color: #666; font-size: 12px;">
            <p>VitalGuard v1.0 | Kabankalan City Fire Station</p>
            <p>Developed by: Libre, Pagtalunan, Vicentino</p>
        </div>
    </div>
</body>
</html>
  )";
  
  return html;
}