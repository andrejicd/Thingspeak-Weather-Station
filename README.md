# ThingSpeak Weather Station with LCD Display

This project implements a simple weather station that reads data from a ThingSpeak channel and displays it on an LCD screen. It uses an ESP8266/ESP32 microcontroller, a ThingSpeak account, and an I2C LCD display.

## Features

*   Reads weather data (temperature, humidity, etc.) from a ThingSpeak channel.
*   Displays the data on an I2C LCD screen.
*   Controls LCD backlight based on time of day.
*   **LCD blinking based on PM2.5 value (field4).**
*   Handles network connection and ThingSpeak communication errors.

## Hardware Requirements

*   ESP8266 or ESP32 microcontroller
*   I2C LCD display (e.g., 16x2 or 20x4)
*   Connecting wires
*   (Optional) Breadboard

## Software Requirements

*   Arduino IDE
*   Required Libraries (install via Arduino Library Manager):
    *   WiFi
    *   ThingSpeak
    *   LCDI2C\_Multilingual
    *   WiFiUdp
    *   NTPClient
    *   TimeLib

## Setup

1.  **ThingSpeak Setup:**
    *   Create a ThingSpeak account at [thingspeak.com](https://thingspeak.com).
    *   Create a new channel and add fields for the weather data you want to read (e.g., Temperature, Humidity, Wind Speed, **PM2.5**).
    *   Note down the Channel ID.

2.  **Arduino IDE Setup:**
    *   Install the required libraries as listed in the Software Requirements.

3.  **Code Configuration:**
    *   Create a file named `secrets.h` in the same directory as your `.ino` file.
    *   Add your Wi-Fi credentials and ThingSpeak Channel ID to `secrets.h`:

    ```c++
    // Use this file to store all of the private credentials 
// and connection details

#define SECRET_SSID "MySSID"		// replace MySSID with your WiFi network name
#define SECRET_PASS "MyPassword"	// replace MyPassword with your WiFi password

#define SECRET_CH_ID_WEATHER_STATION 1234567	          	//MathWorks weather station

#define BACKLIGHT_ON_HOUR *        // Enter the hours to start
#define BACKLIGHT_ON_MINUTE *      // Enter minutes to start
#define BACKLIGHT_OFF_HOUR *       // Enter the hours for the end
#define BACKLIGHT_OFF_MINUTE *     // Enter the minutes to the end
    #endif
    ```

    *   Open the `.ino` file in the Arduino IDE.
    *   Adjust the LCD address (`0x27` in the code) if necessary. You can use an I2C scanner sketch to find the correct address.
    *   Upload the code to your ESP8266/ESP32 board.

4.  **Wiring:**
    *   Connect the LCD to the ESP8266/ESP32 using I2C (SDA, SCL, VCC, GND). Consult your LCD's datasheet for pinouts.

## Usage

After uploading the code, the weather data from your ThingSpeak channel will be displayed on the LCD screen. The backlight will be automatically turned off during specified hours (currently 22:30 to 6:15).

**The LCD will blink based on the PM2.5 value (field4) retrieved from ThingSpeak:**

*   **field4 > 150:** Very frequent blinking (severe air quality).
*   **field4 > 100:** Moderate blinking (unhealthy air quality).
*   **field4 > 50:** Less frequent blinking (moderate air quality).
*   **field4 <= 50:** No blinking (good air quality).

## Troubleshooting

*   **LCD not displaying anything:** Check the wiring and the I2C address.
*   **Not connecting to Wi-Fi:** Double-check your Wi-Fi credentials in `secrets.h`.
*   **Error reading ThingSpeak channel:** Verify the Channel ID in `secrets.h` and ensure the channel is public or you are using the correct Read API Key (if needed).

## Credits

This project is based on examples and libraries available online.

