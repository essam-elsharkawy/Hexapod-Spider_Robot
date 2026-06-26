// ============================================================
// Hexapod Spider Robot - Main Firmware
// ESP32-S3 N16R8 UNO
// Graduation Project 2026 - KMA
// ============================================================

#include <WiFi.h>
#include <WebServer.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLECharacteristic.h>
#include <HardwareSerial.h>
#include "hexapod_controller.h"

// ============================================================
// Wi-Fi Configuration
// ============================================================
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// ============================================================
// Web Server
// ============================================================
WebServer server(80);

// ============================================================
// BLE Configuration
// ============================================================
#define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

BLECharacteristic* pCharacteristic;
bool deviceConnected = false;

// ============================================================
// Sensor Pins (HC-SR04 Ultrasonic)
// ============================================================
#define TRIG_PIN 26
#define ECHO_PIN 27

// ============================================================
// Hexapod Controller
// ============================================================
HexapodController hexapod;

// ============================================================
// BLE Callbacks
// ============================================================
class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("✅ BLE Device Connected");
    }
    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("❌ BLE Device Disconnected");
        pServer->getAdvertising()->start();
    }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0) {
            char cmd = toupper(value[0]);
            Serial.printf("📱 BLE Command: %c\n", cmd);
            
            switch(cmd) {
                case 'F': hexapod.forward(); break;
                case 'B': hexapod.backward(); break;
                case 'L': hexapod.turnLeft(); break;
                case 'R': hexapod.turnRight(); break;
                case 'S': hexapod.stop(); break;
                case 'H': hexapod.home(); break;
                default: Serial.println("⚠️ Unknown BLE command");
            }
        }
    }
};

// ============================================================
// Read Ultrasonic Distance
// ============================================================
float readDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    long duration = pulseIn(ECHO_PIN, HIGH, 26000);
    if (duration == 0) return -1.0f;
    return (float)duration * 0.034f / 2.0f;
}

// ============================================================
// HTTP Handlers
// ============================================================
void handleMove() {
    String response = "OK:";
    bool handled = false;
    
    // Debug: Print all received parameters
    Serial.print("🌐 Web Args: ");
    for (uint8_t i = 0; i < server.args(); i++) {
        Serial.printf("[%s=%s] ", server.argName(i).c_str(), server.arg(i).c_str());
    }
    Serial.println();
    
    // Handle direction commands
    if (server.hasArg("dir")) {
        String direction = server.arg("dir");
        Serial.printf("🎮 Direction: '%s'\n", direction.c_str());
        
        if (direction == "forward") hexapod.forward();
        else if (direction == "backward") hexapod.backward();
        else if (direction == "left") hexapod.turnLeft();
        else if (direction == "right") hexapod.turnRight();
        else if (direction == "stop") hexapod.stop();
        else if (direction == "home") hexapod.home();
        
        response += "M:" + direction + " ";
        handled = true;
    }
    
    // Handle height command
    if (server.hasArg("h")) {
        int level = server.arg("h").toInt();
        hexapod.setHeight(level);
        response += "H:" + String(level) + " ";
        handled = true;
    }
    
    // Handle yaw command
    if (server.hasArg("s")) {
        int level = server.arg("s").toInt();
        hexapod.setYaw(level);
        response += "S:" + String(level) + " ";
        handled = true;
    }
    
    if (!handled) {
        response = "ERROR: Unknown command";
    }
    
    server.send(200, "text/plain", response);
}

void handleRoot() {
    String html = "<html><head><title>Hexapod Robot</title>";
    html += "<style>body{font-family:Arial;text-align:center;background:#1a1a2e;color:white;padding:20px;}";
    html += ".btn{background:#e94560;border:none;color:white;padding:20px 40px;margin:10px;font-size:24px;border-radius:10px;cursor:pointer;min-width:120px;}";
    html += ".btn:active{background:#ff6b81;transform:scale(0.95);}";
    html += ".btn-green{background:#0f3460;}";
    html += ".slider{width:300px;margin:20px;}";
    html += ".status{color:#4ecca3;font-size:18px;}</style>";
    html += "</head><body>";
    html += "<h1>🕷️ Hexapod Spider Robot</h1>";
    html += "<p class='status'>✅ System Online</p>";
    html += "<div>";
    html += "<button class='btn' onmousedown='send(\"forward\")' onmouseup='send(\"stop\")' onmouseleave='send(\"stop\")'>⬆️ Forward</button><br>";
    html += "<button class='btn' onmousedown='send(\"left\")' onmouseup='send(\"stop\")' onmouseleave='send(\"stop\")'>⬅️ Left</button>";
    html += "<button class='btn' onmousedown='send(\"right\")' onmouseup='send(\"stop\")' onmouseleave='send(\"stop\")'>➡️ Right</button><br>";
    html += "<button class='btn' onmousedown='send(\"backward\")' onmouseup='send(\"stop\")' onmouseleave='send(\"stop\")'>⬇️ Backward</button><br>";
    html += "<button class='btn btn-green' onclick='send(\"home\")'>🏠 Home</button>";
    html += "<button class='btn' style='background:#ff4444;' onclick='send(\"stop\")'>⏹️ STOP</button>";
    html += "</div>";
    html += "<div style='margin-top:30px;'>";
    html += "<h3>📐 Body Control</h3>";
    html += "<label>Height: <span id='hVal'>0</span></label><br>";
    html += "<input type='range' class='slider' min='-5' max='5' value='0' id='heightSlider' oninput='updateHeight(this.value)'>";
    html += "<br><label>Yaw: <span id='sVal'>0</span></label><br>";
    html += "<input type='range' class='slider' min='-5' max='5' value='0' id='yawSlider' oninput='updateYaw(this.value)'>";
    html += "</div>";
    html += "<script>";
    html += "function send(cmd){fetch('/move?dir='+cmd).then(r=>r.text()).then(console.log);}";
    html += "function updateHeight(v){document.getElementById('hVal').textContent=v;fetch('/move?h='+v);}";
    html += "function updateYaw(v){document.getElementById('sVal').textContent=v;fetch('/move?s='+v);}";
    html += "</script>";
    html += "</body></html>";
    
    server.send(200, "text/html", html);
}

// ============================================================
// Setup
// ============================================================
void setup() {
    Serial.begin(115200);
    Serial.println("\n🕷️ Hexapod Spider Robot Booting...");
    
    // Initialize Ultrasonic Pins
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    
    // Initialize Hexapod
    hexapod.begin();
    Serial.println("✅ Hexapod Controller Initialized");
    
    // Connect to Wi-Fi
    Serial.printf("📶 Connecting to Wi-Fi: %s\n", ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n✅ Wi-Fi Connected!");
    Serial.printf("🌐 IP Address: http://%s\n", WiFi.localIP().toString().c_str());
    
    // Setup Web Server
    server.on("/", handleRoot);
    server.on("/move", handleMove);
    server.begin();
    Serial.println("✅ Web Server Started");
    
    // Setup BLE
    BLEDevice::init("HexapodRobot");
    BLEServer* pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    
    BLEService* pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
    pCharacteristic->addDescriptor(new BLE2902());
    
    pService->start();
    pServer->getAdvertising()->start();
    Serial.println("✅ BLE Server Started - Device: HexapodRobot");
    
    // Go to Home Position
    hexapod.home();
    Serial.println("✅ System Ready!\n");
}

// ============================================================
// Main Loop
// ============================================================
void loop() {
    static unsigned long lastSensorCheck = 0;
    static unsigned long lastDebugPrint = 0;
    
    // Handle Web Server
    server.handleClient();
    
    // Handle BLE Incoming Data
    // (Handled by callbacks)
    
    // Obstacle Detection (every 100ms only when walking)
    if (hexapod.isMoving() && (millis() - lastSensorCheck > 100)) {
        lastSensorCheck = millis();
        float dist = readDistance();
        
        if (dist > 0 && dist < 20.0f) {
            Serial.printf("⚠️ Obstacle at %.1f cm! Evading...\n", dist);
            
            // Stop and home
            hexapod.stop();
            delay(200);
            hexapod.home();
            delay(500);
            
            // Backward for 2 seconds
            hexapod.backward();
            unsigned long startBack = millis();
            while (millis() - startBack < 2000) {
                server.handleClient();
                hexapod.update();
                delay(5);
            }
            
            // Turn Left for 2 seconds
            hexapod.turnLeft();
            unsigned long startTurn = millis();
            while (millis() - startTurn < 2000) {
                server.handleClient();
                hexapod.update();
                delay(5);
            }
            
            // Resume forward
            hexapod.forward();
        }
    }
    
    // Update Hexapod Gaits
    hexapod.update();
    
    // Debug Print (every 5 seconds)
    if (millis() - lastDebugPrint > 5000) {
        lastDebugPrint = millis();
        Serial.printf("📊 Status: Moving=%d, BLE=%d, WiFi=%s\n",
            hexapod.isMoving(),
            deviceConnected,
            WiFi.isConnected() ? "Connected" : "Disconnected"
        );
    }
    
    delay(2); // Small delay to prevent watchdog timeout
}
