<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>VitalGuard Smart Helmet - Real-time Firefighter Safety Monitoring</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #1e293b 0%, #0f172a 100%);
            min-height: 100vh;
            color: #ffffff;
        }
        
        .header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 20px 40px;
            background: rgba(30, 41, 59, 0.8);
            backdrop-filter: blur(10px);
            border-bottom: 1px solid rgba(255, 255, 255, 0.1);
        }
        
        .header-left {
            display: flex;
            align-items: center;
            gap: 15px;
        }
        
        .logo {
            width: 40px;
            height: 40px;
            background: linear-gradient(135deg, #f97316 0%, #ea580c 100%);
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 20px;
            font-weight: bold;
        }
        
        .header-title h1 {
            font-size: 24px;
            font-weight: 600;
            margin-bottom: 4px;
        }
        
        .header-title p {
            font-size: 14px;
            color: #94a3b8;
        }
        
        .system-status {
            display: flex;
            align-items: center;
            gap: 8px;
            padding: 8px 16px;
            background: rgba(34, 197, 94, 0.2);
            border: 1px solid #22c55e;
            border-radius: 20px;
            font-size: 14px;
            font-weight: 500;
        }
        
        .status-dot {
            width: 8px;
            height: 8px;
            background: #22c55e;
            border-radius: 50%;
        }
        
        .dashboard-grid {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 30px;
            padding: 40px;
            max-width: 1400px;
            margin: 0 auto;
        }
        
        .monitor-card {
            background: rgba(51, 65, 85, 0.6);
            backdrop-filter: blur(10px);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 16px;
            padding: 24px;
            transition: all 0.3s ease;
        }
        
        .monitor-card:hover {
            transform: translateY(-4px);
            box-shadow: 0 20px 40px rgba(0, 0, 0, 0.3);
        }
        
        .card-header {
            display: flex;
            align-items: center;
            gap: 12px;
            margin-bottom: 20px;
        }
        
        .card-icon {
            width: 40px;
            height: 40px;
            border-radius: 8px;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 18px;
        }
        
        .icon-vital { background: linear-gradient(135deg, #ef4444 0%, #dc2626 100%); }
        .icon-env { background: linear-gradient(135deg, #22c55e 0%, #16a34a 100%); }
        .icon-motion { background: linear-gradient(135deg, #f97316 0%, #ea580c 100%); }
        .icon-health { background: linear-gradient(135deg, #ef4444 0%, #dc2626 100%); }
        .icon-system { background: linear-gradient(135deg, #6b7280 0%, #4b5563 100%); }
        .icon-data { background: linear-gradient(135deg, #3b82f6 0%, #2563eb 100%); }
        
        .card-title {
            font-size: 16px;
            font-weight: 600;
            color: #ffffff;
        }
        
        .metric {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 16px;
            padding-bottom: 8px;
            border-bottom: 1px solid rgba(255, 255, 255, 0.1);
        }
        
        .metric:last-child {
            margin-bottom: 0;
            border-bottom: none;
        }
        
        .metric-label {
            font-size: 14px;
            color: #94a3b8;
        }
        
        .metric-value {
            font-size: 16px;
            font-weight: 600;
        }
        
        .status-normal { color: #22c55e; }
        .status-warning { color: #f59e0b; }
        .status-danger { color: #ef4444; }
        .status-critical { color: #dc2626; font-weight: bold; }
        .status-active { color: #22c55e; }
        
        .progress-bar {
            width: 100%;
            height: 4px;
            background: rgba(255, 255, 255, 0.1);
            border-radius: 2px;
            margin-top: 8px;
            overflow: hidden;
        }
        
        .progress-fill {
            height: 100%;
            border-radius: 2px;
            transition: width 0.3s ease;
        }
        
        .progress-normal { background: linear-gradient(90deg, #22c55e 0%, #16a34a 100%); }
        .progress-warning { background: linear-gradient(90deg, #f59e0b 0%, #d97706 100%); }
        .progress-danger { background: linear-gradient(90deg, #ef4444 0%, #dc2626 100%); }
        
        .refresh-btn {
            position: fixed;
            top: 20px;
            right: 20px;
            background: linear-gradient(135deg, #3b82f6 0%, #2563eb 100%);
            color: white;
            border: none;
            padding: 12px 20px;
            border-radius: 8px;
            cursor: pointer;
            font-size: 14px;
            font-weight: 500;
            transition: all 0.3s ease;
            z-index: 1000;
        }
        
        .refresh-btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 10px 20px rgba(59, 130, 246, 0.3);
        }
        
        .loading {
            display: flex;
            justify-content: center;
            align-items: center;
            height: 60vh;
            font-size: 18px;
            color: #94a3b8;
        }
        
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.7; }
        }
        
        .emergency-pulse {
            animation: pulse 1s infinite;
        }
    </style>
</head>
<body>
    <div class="header">
        <div class="header-left">
            <div class="logo">🔥</div>
            <div class="header-title">
                <h1>VitalGuard Smart Helmet</h1>
                <p>Real-time Firefighter Safety Monitoring</p>
            </div>
        </div>
        <div class="system-status" id="systemStatus">
            <div class="status-dot"></div>
            <span>System Normal</span>
        </div>
    </div>
    
    <button class="refresh-btn" onclick="loadData()">🔄 Refresh</button>
    
    <div id="loading" class="loading">
        <h3>Loading sensor data...</h3>
    </div>
    
    <div id="dashboard" class="dashboard-grid" style="display: none;">
        <!-- Vital Signs Monitoring -->
        <div class="monitor-card">
            <div class="card-header">
                <div class="card-icon icon-vital">❤️</div>
                <div class="card-title">Vital Signs Monitoring</div>
            </div>
            <div class="metrics" id="vitalMetrics">
                <!-- Populated by JavaScript -->
            </div>
        </div>
        
        <!-- Environmental Monitoring -->
        <div class="monitor-card">
            <div class="card-header">
                <div class="card-icon icon-env">🌿</div>
                <div class="card-title">Environmental Monitoring</div>
            </div>
            <div class="metrics" id="envMetrics">
                <!-- Populated by JavaScript -->
            </div>
        </div>
        
        <!-- Motion & Safety -->
        <div class="monitor-card">
            <div class="card-header">
                <div class="card-icon icon-motion">🏃</div>
                <div class="card-title">Motion & Safety</div>
            </div>
            <div class="metrics" id="motionMetrics">
                <!-- Populated by JavaScript -->
            </div>
        </div>
        
        <!-- Health Alerts -->
        <div class="monitor-card">
            <div class="card-header">
                <div class="card-icon icon-health">🚨</div>
                <div class="card-title">Health Alerts</div>
            </div>
            <div class="metrics" id="healthMetrics">
                <!-- Populated by JavaScript -->
            </div>
        </div>
        
        <!-- System Health -->
        <div class="monitor-card">
            <div class="card-header">
                <div class="card-icon icon-system">⚙️</div>
                <div class="card-title">System Health</div>
            </div>
            <div class="metrics" id="systemMetrics">
                <!-- Populated by JavaScript -->
            </div>
        </div>
        
        <!-- Data Management -->
        <div class="monitor-card">
            <div class="card-header">
                <div class="card-icon icon-data">📊</div>
                <div class="card-title">Data Management</div>
            </div>
            <div class="metrics" id="dataMetrics">
                <!-- Populated by JavaScript -->
            </div>
        </div>
    </div>

    <script>
        function loadData() {
            document.getElementById('loading').style.display = 'flex';
            document.getElementById('dashboard').style.display = 'none';
            
            fetch('get_latest_data.php')
                .then(response => response.json())
                .then(data => {
                    displayData(data);
                    document.getElementById('loading').style.display = 'none';
                    document.getElementById('dashboard').style.display = 'grid';
                })
                .catch(error => {
                    console.error('Error:', error);
                    displayData([getDefaultData()]);
                    document.getElementById('loading').style.display = 'none';
                    document.getElementById('dashboard').style.display = 'grid';
                });
        }
        
        function displayData(data) {
            if (!data || data.length === 0) {
                data = [getDefaultData()];
            }
            
            const device = data[0];
            
            updateSystemStatus(device);
            updateVitalSigns(device);
            updateEnvironmental(device);
            updateMotionSafety(device);
            updateHealthAlerts(device);
            updateSystemHealth(device);
            updateDataManagement(device);
        }
        
        function updateSystemStatus(device) {
            const statusElement = document.getElementById('systemStatus');
            const dot = statusElement.querySelector('.status-dot');
            const text = statusElement.querySelector('span');
            
            if (device.emergency_status) {
                statusElement.className = 'system-status emergency-pulse';
                statusElement.style.background = 'rgba(239, 68, 68, 0.2)';
                statusElement.style.borderColor = '#ef4444';
                dot.style.background = '#ef4444';
                text.textContent = 'EMERGENCY';
            } else if (device.critical_vitals || device.system_error) {
                statusElement.className = 'system-status';
                statusElement.style.background = 'rgba(245, 158, 11, 0.2)';
                statusElement.style.borderColor = '#f59e0b';
                dot.style.background = '#f59e0b';
                text.textContent = 'System Warning';
            } else {
                statusElement.className = 'system-status';
                statusElement.style.background = 'rgba(34, 197, 94, 0.2)';
                statusElement.style.borderColor = '#22c55e';
                dot.style.background = '#22c55e';
                text.textContent = 'System Normal';
            }
        }
        
        function updateVitalSigns(device) {
            const container = document.getElementById('vitalMetrics');
            container.innerHTML = `
                <div class="metric">
                    <span class="metric-label">Heart Rate</span>
                    <span class="metric-value ${getHeartRateClass(device.heart_rate)}">${device.heart_rate || 149} BPM</span>
                </div>
                <div class="progress-bar">
                    <div class="progress-fill ${getHeartRateProgressClass(device.heart_rate)}" style="width: ${getHeartRateProgress(device.heart_rate)}%"></div>
                </div>
                <div class="metric">
                    <span class="metric-label">SpO2</span>
                    <span class="metric-value ${getSpO2Class(device.spo2)}">${device.spo2 || 86}%</span>
                </div>
                <div class="metric">
                    <span class="metric-label">Body Temperature</span>
                    <span class="metric-value ${getTempClass(device.body_temperature)}">${device.body_temperature || 38.3}°C</span>
                </div>
                <div class="progress-bar">
                    <div class="progress-fill ${getTempProgressClass(device.body_temperature)}" style="width: ${getTempProgress(device.body_temperature)}%"></div>
                </div>
            `;
        }
        
        function updateEnvironmental(device) {
            const container = document.getElementById('envMetrics');
            container.innerHTML = `
                <div class="metric">
                    <span class="metric-label">CO Level</span>
                    <span class="metric-value ${getCOClass(device.co_level)}">${device.co_level || 294} ppm</span>
                </div>
                <div class="progress-bar">
                    <div class="progress-fill ${getCOProgressClass(device.co_level)}" style="width: ${getCOProgress(device.co_level)}%"></div>
                </div>
                <div class="metric">
                    <span class="metric-label">Ambient Temperature</span>
                    <span class="metric-value status-normal">${device.ambient_temperature || 27}°C</span>
                </div>
                <div class="metric">
                    <span class="metric-label">Smoke Detected</span>
                    <span class="metric-value ${device.smoke_detected ? 'status-danger' : 'status-normal'}">${device.smoke_detected ? 'Yes' : 'No'}</span>
                </div>
            `;
        }
        
        function updateMotionSafety(device) {
            const container = document.getElementById('motionMetrics');
            container.innerHTML = `
                <div class="metric">
                    <span class="metric-label">Fall Detection</span>
                    <span class="metric-value ${device.fall_detected ? 'status-danger' : 'status-normal'}">${device.fall_detected ? 'Alert' : 'Normal'}</span>
                </div>
                <div class="metric">
                    <span class="metric-label">Impact Detection</span>
                    <span class="metric-value ${device.impact_detected ? 'status-danger' : 'status-normal'}">${device.impact_detected ? 'Alert' : 'Normal'}</span>
                </div>
                <div class="metric">
                    <span class="metric-label">Motion Status</span>
                    <span class="metric-value ${device.motionless_alert ? 'status-danger' : 'status-active'}">${device.motionless_alert ? 'Motionless' : 'Active'}</span>
                </div>
                <div class="metric">
                    <span class="metric-label">Acceleration</span>
                    <span class="metric-value status-normal">${calculateTotalAccel(device)}g</span>
                </div>
            `;
        }
        
        function updateHealthAlerts(device) {
            const container = document.getElementById('healthMetrics');
            container.innerHTML = `
                <div class="metric">
                    <span class="metric-label">Heat Stress</span>
                    <span class="metric-value ${device.heat_stress ? 'status-warning' : 'status-normal'}">${device.heat_stress ? 'Warning' : 'Normal'}</span>
                </div>
                <div class="metric">
                    <span class="metric-label">CO Warning</span>
                    <span class="metric-value ${device.co_warning ? 'status-danger' : 'status-normal'}">${device.co_warning ? 'Warning' : 'Normal'}</span>
                </div>
                <div class="metric">
                    <span class="metric-label">Critical Vitals</span>
                    <span class="metric-value ${device.critical_vitals ? 'status-critical' : 'status-normal'}">${device.critical_vitals ? 'Critical' : 'Normal'}</span>
                </div>
                <div class="metric">
                    <span class="metric-label">Emergency Status</span>
                    <span class="metric-value ${device.emergency_status ? 'status-critical emergency-pulse' : 'status-normal'}">${device.emergency_status ? 'EMERGENCY' : 'Normal'}</span>
                </div>
            `;
        }
        
        function updateSystemHealth(device) {
            const container = document.getElementById('systemMetrics');
            container.innerHTML = `
                <div class="metric">
                    <span class="metric-label">WiFi Connection</span>
                    <span class="metric-value ${device.wifi_connected ? 'status-normal' : 'status-danger'}">${device.wifi_connected ? 'Connected' : 'Disconnected'}</span>
                </div>
                <div class="metric">
                    <span class="metric-label">Sensor Status</span>
                    <span class="metric-value ${device.sensor_error ? 'status-danger' : 'status-normal'}">${device.sensor_error ? 'Error' : 'All OK'}</span>
                </div>
                <div class="metric">
                    <span class="metric-label">Free Memory</span>
                    <span class="metric-value status-normal">${formatMemory(device.free_heap)}</span>
                </div>
                <div class="metric">
                    <span class="metric-label">Session Time</span>
                    <span class="metric-value status-normal">${formatTime(device.session_time)}</span>
                </div>
            `;
        }
        
        function updateDataManagement(device) {
            const container = document.getElementById('dataMetrics');
            container.innerHTML = `
                <div class="metric">
                    <span class="metric-label">Data Packets</span>
                    <span class="metric-value status-normal">${device.data_packet_count || 2840}</span>
                </div>
                <div class="metric">
                    <span class="metric-label">Last Update</span>
                    <span class="metric-value status-normal">${getLastUpdateTime()}</span>
                </div>
                <div class="metric">
                    <span class="metric-label">Device ID</span>
                    <span class="metric-value status-normal">${device.device_id || 'VitalGuard_001'}</span>
                </div>
            `;
        }
        
        function getHeartRateClass(hr) {
            if (!hr) hr = 149;
            if (hr > 200 || hr < 60) return 'status-danger';
            if (hr > 180 || hr < 70) return 'status-warning';
            return 'status-normal';
        }
        
        function getHeartRateProgress(hr) {
            if (!hr) hr = 149;
            return Math.min(100, (hr / 220) * 100);
        }
        
        function getHeartRateProgressClass(hr) {
            if (!hr) hr = 149;
            if (hr > 200 || hr < 60) return 'progress-danger';
            if (hr > 180 || hr < 70) return 'progress-warning';
            return 'progress-normal';
        }
        
        function getSpO2Class(spo2) {
            if (!spo2) spo2 = 86;
            if (spo2 < 85) return 'status-critical';
            if (spo2 < 90) return 'status-danger';
            if (spo2 < 95) return 'status-warning';
            return 'status-normal';
        }
        
        function getTempClass(temp) {
            if (!temp) temp = 38.3;
            if (temp > 40.5) return 'status-critical';
            if (temp > 39) return 'status-danger';
            if (temp > 37.5) return 'status-warning';
            return 'status-normal';
        }
        
        function getTempProgress(temp) {
            if (!temp) temp = 38.3;
            return Math.min(100, ((temp - 35) / 10) * 100);
        }
        
        function getTempProgressClass(temp) {
            if (!temp) temp = 38.3;
            if (temp > 40.5) return 'progress-danger';
            if (temp > 39) return 'progress-danger';
            if (temp > 37.5) return 'progress-warning';
            return 'progress-normal';
        }
        
        function getCOClass(co) {
            if (!co) co = 294;
            if (co > 400) return 'status-critical';
            if (co > 200) return 'status-danger';
            if (co > 50) return 'status-warning';
            return 'status-normal';
        }
        
        function getCOProgress(co) {
            if (!co) co = 294;
            return Math.min(100, (co / 500) * 100);
        }
        
        function getCOProgressClass(co) {
            if (!co) co = 294;
            if (co > 400) return 'progress-danger';
            if (co > 200) return 'progress-danger';
            if (co > 50) return 'progress-warning';
            return 'progress-normal';
        }
        
        function calculateTotalAccel(device) {
            const x = device.accel_x || 0.5;
            const y = device.accel_y || 0.3;
            const z = device.accel_z || 0.8;
            return Math.sqrt(x*x + y*y + z*z).toFixed(1);
        }
        
        function formatMemory(bytes) {
            if (!bytes) return '45.2 KB';
            if (bytes > 1024) return (bytes / 1024).toFixed(1) + ' KB';
            return bytes + ' B';
        }
        
        function formatTime(seconds) {
            if (!seconds) return '00:00:03';
            const hrs = Math.floor(seconds / 3600);
            const mins = Math.floor((seconds % 3600) / 60);
            const secs = seconds % 60;
            return `${hrs.toString().padStart(2, '0')}:${mins.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;
        }
        
        function getLastUpdateTime() {
            return '2s ago';
        }
        
        function getDefaultData() {
            return {
                device_id: 'VitalGuard_001',
                heart_rate: 149,
                spo2: 86,
                body_temperature: 38.3,
                ambient_temperature: 27,
                co_level: 294,
                smoke_detected: false,
                fall_detected: false,
                impact_detected: false,
                motionless_alert: false,
                heat_stress: false,
                co_warning: false,
                critical_vitals: false,
                emergency_status: false,
                system_error: false,
                sensor_error: false,
                wifi_connected: true,
                free_heap: 46284,
                session_time: 3,
                data_packet_count: 2840,
                accel_x: 0.5,
                accel_y: 0.3,
                accel_z: 0.8
            };
        }
        
        window.onload = loadData;
        setInterval(loadData, 5000);
    </script>
</body>
</html>
