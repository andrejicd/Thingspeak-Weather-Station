#include <ESP8266WiFi.h>
#include "secrets.h" // Contains your WiFi SSID, password, and ThingSpeak channel ID
#include <ThingSpeak.h>
#include <LCDI2C_Multilingual.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>

// LCD configuration (adjust address and size if needed)
LCDI2C_Generic lcd(0x27, 16, 2);

// ThingSpeak channel details
unsigned long weatherStationChannelNumber = SECRET_CH_ID_WEATHER_STATION;

WiFiClient client;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600); // NTP server and offset (seconds)

bool backlightState = false;
float field[8] = {0}; // Initialize the array

void setup() {
  Serial.begin(115200);
  while (!Serial);

  lcd.init();
  lcd.clear();

  timeClient.begin();
  ThingSpeak.begin(client);

  // Connect to WiFi
  Serial.print("Connecting to ");
  Serial.println(SECRET_SSID);
  WiFi.begin(SECRET_SSID, SECRET_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  setTime(epochTime);

  time_t currentTime = now();
  int currentHour = hour(currentTime);
  int currentMinute = minute(currentTime);

  // Backlight control based on time (22:30 - 6:15)
  bool shouldBacklightBeOn = !((currentHour == 22 && currentMinute >= 30) || (currentHour > 22) || (currentHour < 6) || (currentHour == 6 && currentMinute < 15));

  if (shouldBacklightBeOn != backlightState) {
    backlightState = shouldBacklightBeOn;
    if (backlightState) {
      lcd.backlight();
      Serial.println("Backlight ON");
    } else {
      lcd.noBacklight();
      Serial.println("Backlight OFF");
    }
  }

  int statusCode = ThingSpeak.readMultipleFields(weatherStationChannelNumber);

  if (statusCode == 200) {
    // Read all 8 fields
    for (int i = 0; i < 8; i++) {
      field[i] = ThingSpeak.getFieldAsFloat(i + 1);
    }

    // Serial output
    Serial.println("--------------------");
    Serial.println("Temperature: " + String(field[0]) + " °C");
    Serial.println("Atmospheric Pressure: " + String(field[1]) + " mBar");
    Serial.println("Humidity: " + String(field[2]) + " %");
    Serial.println("PM 2.5: " + String(field[3]) + " ug/m3");
    Serial.println("Dew Point: " + String(field[4]) + " °C");
    Serial.println("Absolute Humidity: " + String(field[5]) + " g/m3");
    Serial.println("Sea Level Pressure: " + String(field[6]) + " mBar");
    Serial.println("Altitude: " + String(field[7]) + " m");
    Serial.println("--------------------");

    // Display on LCD
    lcd.clear();
    lcd.print("T:");
    lcd.print(field[0]);
    lcd.print("C H:");
    lcd.print(field[2]);
    lcd.print("%");
    lcd.setCursor(0, 1);
    lcd.print("PM:");
    lcd.print(field[3]);
    lcd.print(" AP:");
    lcd.print(field[1]);

    // Blink logic (based on PM2.5 - field[3])
    int blinkFrequency = 2; // Default blink frequency
    if (field[3] > 150) {
      blinkFrequency = 5; // Faster blinking for high PM2.5
    } else if (field[3] > 100) {
      blinkFrequency = 3; // Medium blinking
    } else if (field[3] > 50) {
      blinkFrequency = 2; // Slower blinking
    }

    for (int i = 0; i < 10; i++) {
      lcd.noBacklight();
      delay(500 / blinkFrequency);
      lcd.backlight();
      delay(500 / blinkFrequency);
    }
  } else {
    Serial.print("ThingSpeak error: ");
    Serial.println(statusCode);
  }
  delay(150000); // Update every 2.5 minutes
}