#include <WiFi.h>
#include <WebServer.h>

// Network Configuration
IPAddress local_IP(192, 168, 4, 36);  // Static IP for this receiver
IPAddress gateway(192, 168, 4, 1);     // Your router's IP
IPAddress subnet(255, 255, 255, 0);

// WiFi credentials
const char* ssid = "Syndicate";
const char* password = "Syndicate1";

// Web Server
WebServer server(80);

// Hardware Configuration
const int buzzerPin = 27; // GPIO pin connected to the buzzer

// Periodic reboot configuration (6 hours)
const unsigned long rebootInterval = 6UL * 60UL * 60UL * 1000UL;
unsigned long lastRebootTime = 0;

// WiFi Reconnect Timer
unsigned long previousMillis = 0;
const long interval = 30000;  // Check WiFi every 30 seconds

// Handles the '/activate' endpoint
void handleActivate() {
  Serial.println("[Handler] '/activate' endpoint called.");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "Buzzer Activated!"); // Send response immediately
  
  Serial.println("--> Activating buzzer (pin LOW)");
  digitalWrite(buzzerPin, LOW);
  delay(1000); // Keep buzzer on for 1 second
  
  Serial.println("--> Deactivating buzzer (pin HIGH)");
  digitalWrite(buzzerPin, HIGH); // Turn buzzer off
  Serial.println("[Handler] 'handleActivate' complete.");
}

// Handles the '/status' endpoint for Dashboard
void handleStatus() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  String json = "{\"status\":\"ok\",\"uptime_ms\":" + String(millis()) + "}";
  server.send(200, "application/json", json);
}

// Handles not found pages
void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(404, "text/plain", message);
  Serial.print(message);
}

void setup() {
  Serial.begin(115200);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, HIGH); // Default state is OFF (assuming active-LOW)

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("Receiver IP Address: ");
  Serial.println(WiFi.localIP());

  // Define server routes
  server.on("/activate", HTTP_GET, handleActivate);
  server.on("/status", HTTP_GET, handleStatus);
  server.onNotFound(handleNotFound);

  // Start server
  server.begin();
  Serial.println("HTTP server started.");

  // Initialize reboot timer
  lastRebootTime = millis();
}

void loop() {
  // Check WiFi connection
  unsigned long currentMillis = millis();
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval)) {
    Serial.print(currentMillis);
    Serial.println(" Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }

  // Handle client requests
  server.handleClient();

  // Check for periodic reboot
  if (millis() - lastRebootTime >= rebootInterval) {
    Serial.println("Scheduled reboot...");
    ESP.restart();
  }
}
