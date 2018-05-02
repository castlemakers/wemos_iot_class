# wemos_iot_class

This repository contains the files used in the April 2018 Internet of Things class.

Uses the WeMos D1 mini (esp8266), a 128x64 SSD1306 OLED display, and a DHT22 temperature/humidity sensor.
The OLED is connected via I2C (D2 is SDA and D1 is SCL), while the DHT data line is connected to pin D4 of the WeMos board.

Data from the sensor is both displayed on the OLED and published to MQTT (default is to use HiveMQ). Connect to MQTT
(e.g., at http://www.hivemq.com/demos/websocket-client/) and subscribe to the topic "castlemakers/demo/#". Publishing the
message "0" or "1" to the topic "castlemakers/demo/LEDSwitch" will turn OFF or ON the LED (at the moment, this only causes
the LED status to be shown on the display and published).

In addition to the sketch, the Castlemakers_WeMos_Temp_Demo directory also contains the contents of the
Adafruit_SSD1306 library, from https://github.com/adafruit/Adafruit_SSD1306/ -- this is because the
header file for the library needs to be edited to select the correct screen size, and we wanted to avoid
having the class do that.

Required libraries:

* ESP8266WiFi
* PubSubClient
* DHT sensor library for ESPx
* Adafruit GFX Library
