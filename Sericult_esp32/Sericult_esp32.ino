#include <Arduino.h>
#include <WiFi.h>               // ESP32
#include <Firebase_ESP_Client.h>
#include <DHT.h>                // Install DHT library by Adafruit 1.3.8

#define DHT_SENSOR_PIN 4
#define DHT_SENSOR_TYPE DHT11
#define LDR_SENSOR_PIN 34       // Analog pin for LDR (change based on your circuit)
#define RELAY_PIN 26            // GPIO pin for the relay (change as needed)

// Initialize the DHT sensor
DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

// Provide the token generation process info
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "ROCKSTAR"
#define WIFI_PASSWORD "1234567890"

// Insert Firebase project API Key
#define API_KEY "AIzaSyAZiwEWb7rEHtml1KqOV6iapEQ8Yorc2c8"

// Insert RTDB URL
#define DATABASE_URL "https://iotpro-ea2f4-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

void setup() {
  dht_sensor.begin();
  Serial.begin(115200);

  // Configure relay pin
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Start with relay off

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Firebase configuration
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Anonymous sign-in
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Sign up successful!");
    signupOK = true;
  } else {
    Serial.printf("Sign up error: %s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  // Read temperature and humidity from DHT sensor
  float temperature = dht_sensor.readTemperature();
  float humidity = dht_sensor.readHumidity();

  // Read light intensity from LDR
  int ldrValue = analogRead(LDR_SENSOR_PIN);

  // Control the relay based on conditions
  bool relayState = false;
  if (temperature > 30 && humidity < 70) {
    relayState = true;
    digitalWrite(RELAY_PIN, HIGH); // Turn relay on
  } else {
    relayState = false;
    digitalWrite(RELAY_PIN, LOW); // Turn relay off
  }

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    // Send temperature data to Firebase
    if (Firebase.RTDB.setInt(&fbdo, "DHT_11/Temperature", temperature)) {
      Serial.print("Temperature: ");
      Serial.println(temperature);
    } else {
      Serial.println("Failed to send temperature data");
      Serial.println("Reason: " + fbdo.errorReason());
    }

    // Send humidity data to Firebase
    if (Firebase.RTDB.setFloat(&fbdo, "DHT_11/Humidity", humidity)) {
      Serial.print("Humidity: ");
      Serial.println(humidity);
    } else {
      Serial.println("Failed to send humidity data");
      Serial.println("Reason: " + fbdo.errorReason());
    }

    // Send LDR data to Firebase
    if (Firebase.RTDB.setInt(&fbdo, "LDR/LightIntensity", ldrValue)) {
      Serial.print("Light Intensity: ");
      Serial.println(ldrValue);
    } else {
      Serial.println("Failed to send LDR data");
      Serial.println("Reason: " + fbdo.errorReason());
    }

    // Send relay state to Firebase
    if (Firebase.RTDB.setBool(&fbdo, "Relay/State", relayState)) {
      Serial.print("Relay State: ");
      Serial.println(relayState ? "ON" : "OFF");
    } else {
      Serial.println("Failed to send relay state");
      Serial.println("Reason: " + fbdo.errorReason());
    }
  }
}