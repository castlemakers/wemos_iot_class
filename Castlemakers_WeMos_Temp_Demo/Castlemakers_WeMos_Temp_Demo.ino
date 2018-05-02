/*
 IoT / MQTT Test
 */

// Include needed libraries
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h" // local version customized to 128x64 screen

// Define Pins by name
// Expects SDA on D2 and SCL on D1
#define DHTPin D4

// Define WIFI Setup
const char* ssid = "<SSID>"; // Change this to the SSID of the WIFI network you'll connect to
const char* password = "<PASSWORD>"; // Change this to your WIFI password
const char* mqtt_server = "broker.hivemq.com"; // Change this to the MQTT Server you'll be using

#define MQTT_TOPIC_PREFIX "castlemakers/demo/"
#define MQTT_LED_SWITCH   MQTT_TOPIC_PREFIX "LEDSwitch"
#define MQTT_LED_STATUS   MQTT_TOPIC_PREFIX "LEDStatus"
#define MQTT_TEMPERATURE  MQTT_TOPIC_PREFIX "TempReading"
#define MQTT_HUMIDITY     MQTT_TOPIC_PREFIX "HumidityReading"
#define MQTT_HEATINDEX    MQTT_TOPIC_PREFIX "HeatIndex"

// Initialize Display
Adafruit_SSD1306 display(0);

// Initialize DHT Sensor
DHTesp dht;

// Initialize MQTT Client
WiFiClient espClient;
PubSubClient client(espClient);

// Define Variables
long lastMsg = 0;
char ledStatus = '0'; // Stored value of Builtin LED status
int value = 0;

// Main setup function
void setup() {
  // Set pin Modes
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(DHTPin, INPUT);

  // Initialize Serial Port
  Serial.begin(115200);

  // Initialize display
  display.begin();
  display.clearDisplay();
  display.setTextWrap(true);
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Call WIFI Setup Function
  setup_wifi();

  // Setup DHT
  dht.setup(DHTPin);

  // Set MQTT Server
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Set initial state of pins
  digitalWrite(BUILTIN_LED, HIGH);

  // Set a delay
  delay(1000);
}

// Function to setup WIFI
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Connecting to: ");
  display.print(ssid);
  display.display();

  delay(500);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    display.print(".");
    display.display();
  }

  // Debug information sent to serial monitor
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Debug information sent to display
  display.println("");
  display.println("WiFi connected");
  display.println("IP address: ");
  display.println(WiFi.localIP());
  display.display();
}

// This is the function that reads the MQTT message when triggered by the client.loop
void callback(char* topic, byte* payload, unsigned int length) {
  // This terminates the payload array with a null - otherwise you get leftover trash
  payload[length] = '\0';

  // Debug information that prints to the serial monitor
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println();

  // Switch on the LED if a 1 was received
  if (strcmp(topic, MQTT_LED_SWITCH) == 0) {
    if (strcmp((char*) payload, "1") == 0) {
      digitalWrite(BUILTIN_LED, LOW);
      ledStatus = '1';
    } else {
      digitalWrite(BUILTIN_LED, HIGH);
      ledStatus = '0';
    }
  }
}

//If MQTT client loses connection, reconnect
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    display.print("Attempting MQTT connection...");
    display.display();

    // Attempt to connect
    if (client.connect("ESPSensor")) {
      Serial.println("connected");
      display.println("connected");
      display.display();
      // Once connected, publish...
      client.publish(MQTT_LED_STATUS,
          String(ledStatus).c_str());
      // ... and resubscribe
      client.subscribe(MQTT_LED_SWITCH);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");

      display.print("failed, rc=");
      display.print(client.state());
      display.println(" try again in 5 seconds");
      display.display();
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  // Check if MQTT client is still connected
  if (!client.connected()) {
    reconnect();
  }

  // Run the client loop that checks for MQTT messages
  client.loop();

  // Clear the display to re-write values
  display.clearDisplay();

  long rightnow = millis();

  // Set Cursor and first line on display
  display.setCursor(0, 0);
  display.println("Title at the top"); // Change this to a title for the top of the display
  display.println("WIFI SSID: ");
  display.println(ssid);

  if (rightnow - lastMsg > 2000) {
    // Publish LED Status and put it on the second line of the display
    if (ledStatus == '1') {
      client.publish(MQTT_LED_STATUS, "1");
      display.setCursor(0, 54);
      display.print("LED: ON");
    } else {
      client.publish(MQTT_LED_STATUS, "0");
      display.setCursor(0, 54);
      display.print("LED: OFF");
    }

    // Publish and display Temp/Humidity/HeatIndex
    float humidity = dht.getHumidity();
    float temperatureC = dht.getTemperature();
    float temperatureF = dht.toFahrenheit(temperatureC);
    float heatIndexC = dht.computeHeatIndex(temperatureC, humidity, false);
    float heatIndexF = dht.computeHeatIndex(temperatureF, humidity, true);

    if (dht.getStatusString() == "OK") {
      Serial.print(dht.getStatusString());
      Serial.print("\t");
      Serial.print(humidity, 1);
      Serial.print("\t\t");
      Serial.print(temperatureC, 1);
      Serial.print("\t\t");
      Serial.print(temperatureF, 1);
      Serial.print("\t\t");
      Serial.print(heatIndexC, 1);
      Serial.print("\t\t");
      Serial.println(heatIndexF, 1);

      int pTemp = (int) temperatureF;
      int pHumidity = (int) humidity;

      client.publish(MQTT_TEMPERATURE, String(pTemp).c_str());

      display.setCursor(0, 27);
      display.print("Temperature: ");
      display.print(pTemp);
      display.print("F");

      client.publish(MQTT_HUMIDITY, String(pHumidity).c_str());

      display.setCursor(0, 36);
      display.print("Humidity: ");
      display.print(pHumidity);
      display.print("%");

      client.publish(MQTT_HEATINDEX, String(heatIndexF).c_str());

      display.setCursor(0,45);
      display.print("Heat Index: ");
      display.print(heatIndexF);
      display.print("F");

      display.display();
    }
    lastMsg = rightnow;
  }
}

