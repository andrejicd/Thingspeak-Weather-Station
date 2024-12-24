#include <WiFi.h>
#include "secrets.h"  // Include secrets header first (recommended)
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros
#include <LCDI2C_Multilingual.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h> // Include the Time library

// Configure LCD (adjust address and size as needed)
LCDI2C_Generic lcd(0x27, 16, 2);  // I2C address: 0x27; Display size: 20x4

char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient client;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600); // NTP server and time offset in seconds (3600 seconds = 1 hour)

// Weather station channel details
unsigned long weatherStationChannelNumber = SECRET_CH_ID_WEATHER_STATION;

int statusCode = 0;
int field[8] = {1,2,3,4,5,6,7,8};

bool backlightState = false;

void setup() {
  Serial.begin(115200);      // Initialize serial 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }
  
  lcd.init();
  lcd.clear();
  
  WiFi.mode(WIFI_STA);
 
  timeClient.begin();  // Initialize NTPClient
  ThingSpeak.begin(client);  // Initialize ThingSpeak

  // Connect or reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected");
  }

  // Set the initial time for demonstration purposes
  //setTime(5, 34, 0, 20, 12, 2024);  // hr, min, sec, day, month, year
}

void loop() {
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  setTime(epochTime);  // Set the system time using NTP

  // Update time every loop for simplicity
  time_t currentTime = now();
  int currentHour = hour(currentTime);
  int currentMinute = minute(currentTime);

  // Backlight control based on time
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

  // Read and store all the latest field values, location coordinates, status message, and created-at timestamp
  statusCode = ThingSpeak.readMultipleFields(weatherStationChannelNumber);
  
  if (statusCode == 200) {
    // Fetch the stored data
    float field1 = ThingSpeak.getFieldAsFloat(field[0]); // Field 1
    float field2 = ThingSpeak.getFieldAsFloat(field[1]); // Field 2
    float field3 = ThingSpeak.getFieldAsFloat(field[2]); // Field 3
    float field4 = ThingSpeak.getFieldAsFloat(field[3]); // Field 4
    float field5 = ThingSpeak.getFieldAsFloat(field[4]); // Field 5
    float field6 = ThingSpeak.getFieldAsFloat(field[5]); // Field 6
    float field7 = ThingSpeak.getFieldAsFloat(field[6]); // Field 7
    float field8 = ThingSpeak.getFieldAsFloat(field[7]); // Field 8
    String statusMessage = ThingSpeak.getStatus(); // Status message
    String latitude = ThingSpeak.getLatitude(); // Latitude
    String longitude = ThingSpeak.getLongitude(); // Longitude
    String elevation = ThingSpeak.getElevation(); // Elevation
    String createdAt = ThingSpeak.getCreatedAt(); // Created-at timestamp
    
    // Serial output
    Serial.println("Temperatura: " + String(field1) + " °C");
    Serial.println("Atmospheric Pressure: " + String(field2) + " mBar");
    Serial.println("Humidity: " + String(field3) + " %");
    Serial.println("PM 2.5: " + String(field4) + " ug/m3");
    Serial.println("dewPoint: " + String(field5) + " °C");
    Serial.println("humidityAbs: " + String(field6) + " g/m3");
    Serial.println("pressureSea: " + String(field7) + " mBar");
    Serial.println("altitude: " + String(field8) + " m");
   
    // Display data on LCD
    lcd.clear();
    lcd.blink();
    lcd.print("T:"); 
    lcd.print(field1);
    lcd.print("C ");
    lcd.print("H:"); 
    lcd.print(field3);
    lcd.print("% ");
    lcd.setCursor(0, 1);
    lcd.print("PM:"); 
    lcd.print(field4);
    lcd.print(" ");
    lcd.print("AP:"); 
    lcd.print(field2);
    lcd.print(" ");
    
    // Blink logic
    if (field4 > 150) {
      for (int i = 0; i < 10; i++) {
        lcd.noBacklight();
        delay(250);
        lcd.backlight();
        delay(250);
      }
    } else if (field4 > 100) {
      for (int i = 0; i < 5; i++) {
        lcd.noBacklight();
        delay(400);
        lcd.backlight();
        delay(400);
      }
    } else if (field4 > 50) {
      for (int i = 0; i < 2; i++) {
        lcd.noBacklight();
        delay(550);
        lcd.backlight();
        delay(550);
      }
    }

    delay(10000);  
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Alt:"); 
    lcd.print(field8);
    lcd.print("m ");
    lcd.print("pSea:"); 
    lcd.print(field7);
    lcd.print("mbar");
    
    delay(10000);  
    Serial.println();
    
    // Repeat first display
    lcd.clear();
    lcd.print("T:"); 
    lcd.print(field1);
    lcd.print("C ");
    lcd.print("H:"); 
    lcd.print(field3);
    lcd.print("% ");
    lcd.setCursor(0, 1);
    lcd.print("PM:"); 
    lcd.print(field4);
    lcd.print(" ");
    lcd.print("AP:"); 
    lcd.print(field2);
    lcd.print(" ");
  } else {
    Serial.println("Problem reading channel. HTTP error code " + String(statusCode)); 
  }

  Serial.println();
  delay(150000); // 2.5 min no need to fetch too often
}
