/**
 * @file main.cpp
 * @brief PIR Sensor with M5Stack, FastLED, and HTTPClient
 * 
 * This program reads data from a PIR sensor, updates an LED strip, and sends motion status to a server.
 * 
 * @author Sebastian Loft Larsen     
 * @date 02/09/2024
 */

#include <M5Stack.h>
#include <FastLED.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "env.h" 

#define PIR_PIN 36 ///< GPIO pin for the PIR sensor
#define LED_PIN 21 ///< GPIO pin for the LED strip
#define NUM_LEDS 3 ///< Number of LEDs in the strip
#define TIMEOUT 300000 ///< Timeout duration in milliseconds
#define COUNTDOWN_STEP 1000 ///< Countdown step in milliseconds

CRGB leds[NUM_LEDS]; ///< Array to hold LED data
unsigned long lastMotionTime = 0; ///< Timestamp of the last detected motion
unsigned long countdown = TIMEOUT / COUNTDOWN_STEP; ///< Countdown timer
bool lastMotionDetected = false; ///< Flag to indicate the last motion status

String apiUrl = "http://192.168.1.125:8000/motion-sensors"; ///< API URL for sending motion status

HTTPClient httpClient; ///< HTTPClient instance for making HTTP requests

/**
 * @brief Sends the motion status to the server.
 * 
 * @param motionDetected Boolean indicating whether motion was detected.
 */
void sendMotionStatus(bool motionDetected);

/**
 * @brief Setup function to initialize the M5Stack, WiFi, and peripherals.
 */
void setup() {
    M5.begin();
    Serial.begin(115200);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("PIR Sensor Test");

    pinMode(PIR_PIN, INPUT);
    FastLED.addLeds<SK6812, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(255);

    // Connect to WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // Initialize HTTPClient with API URL
    httpClient.begin(apiUrl);
}

/**
 * @brief Main loop function to read PIR sensor data, update LEDs, and send motion status.
 */
void loop() {
    int pirState = digitalRead(PIR_PIN);
    unsigned long currentTime = millis();

    M5.Lcd.fillRect(0, 20, 320, 40, TFT_BLACK);

    if (pirState == HIGH) {
        M5.Lcd.setCursor(0, 20);
        M5.Lcd.setTextColor(TFT_GREEN);
        M5.Lcd.println("Motion Detected!");

        // Update LED status
        fill_solid(leds, NUM_LEDS, CRGB::White);
        lastMotionTime = currentTime;
        countdown = TIMEOUT / COUNTDOWN_STEP;

        if (!lastMotionDetected) {
            sendMotionStatus(true);
            lastMotionDetected = true;
        }

    } else {
        unsigned long timeSinceLastMotion = currentTime - lastMotionTime;

        if (timeSinceLastMotion >= TIMEOUT) {
            countdown = 0;
            fill_solid(leds, NUM_LEDS, CRGB::Black);

            if (lastMotionDetected) {
                sendMotionStatus(false);
                lastMotionDetected = false;
            }

        } else {
            countdown = (TIMEOUT - timeSinceLastMotion) / COUNTDOWN_STEP;
        }

        M5.Lcd.setCursor(0, 20);
        M5.Lcd.setTextColor(TFT_RED);
        M5.Lcd.println("No Motion");

        M5.Lcd.setCursor(0, 40);
        M5.Lcd.setTextColor(TFT_YELLOW);
        M5.Lcd.printf("Turning off in: %lu s", countdown);
    }

    FastLED.show();
    delay(100); // Reduced delay for better responsiveness
}

/**
 * @brief Sends the motion status to the server.
 * 
 * @param motionDetected Boolean indicating whether motion was detected.
 */
void sendMotionStatus(bool motionDetected) {
    HTTPClient http;
    http.begin(apiUrl); // Specify the URL
    http.addHeader("Content-Type", "application/json");

    // Create the JSON payload
    String payload = "{\"roomName\": \"z29C\", \"motion\": \"" + String(motionDetected ? "1" : "0") + "\"}";

    Serial.println("Sending payload: " + payload); // Debugging line

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("HTTP Response code: " + String(httpResponseCode));
        Serial.println("Response: " + response);
    } else {
        Serial.println("Error on HTTP request. Code: " + String(httpResponseCode));
    }
    http.end(); // Free resources
}