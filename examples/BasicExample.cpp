#include <Arduino.h>
#include "ConnectionManager.h"




void cm_setup() {

    // Add WiFiManager custom parameter with name, placeholder
    ConnectionManager::parameter("mqtt_server", "mqtt_server");

    // Set a callback to run on WiFi connect
    ConnectionManager::onConnect([]() {
        Serial.println("Connected!");
    });

    // Initiate autoconnection with a given AP SSID for fallback
    ConnectionManager::autoConnect("APNAME");

    // After connection, retrieve a parameter by ID
    std::string test = ConnectionManager::parameter("mqtt_server");
    Serial.print("Value: ");
    Serial.println(test.c_str());
}

void setup() {
    Serial.begin(115200); Serial.println("\r\n\r\n");
    delay(200);

    cm_setup();

    // Use this to clear saved networks
    // ConnectionManager::erase();

}

void loop() {
  
}
