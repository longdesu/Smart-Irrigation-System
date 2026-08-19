const int SOIL_PIN = 1;
const int PUMP_PIN = 4;

// Timers auxiliar variables
unsigned long now;
unsigned long lastMeasure;

// --- Updated Calibration Constants ---
const int VAL_DRY = 2350;
const int VAL_WET_SOIL = 1550; // 100% moisture now aligns with 1cm of standing water

// Shifted water depth values
const int VAL_1CM = 1550; // New starting  point
const int VAL_2CM = 1300; // Previously 1cm
const int VAL_3CM = 1113; // Previously 2cm
const int VAL_4CM = 1068; // Previously 3cm
const int VAL_5CM = 1012; // Previously 4cm
const int VAL_6CM = 981;  // Previously 5cm
//libraries
#include "Adafruit_SHTC3.h"
Adafruit_SHTC3 shtc3 = Adafruit_SHTC3();
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>


WiFiClient wifi_client;
PubSubClient mqtt_client;

#define WIFI_SSID "FPT Telecom-EA06"
#define WIFI_PASS "22805343"
#define MQTT_HOST "192.168.1.238" //DEPENDING ON SYSTEM CHANGE THIS NIGG
#define MQTT_PORT 1883
#define CLIENT_ID "my_first_publisher"

void setup() {
  Serial.begin(115200);
  //SHTC3 setup
  Serial.println("SHTC3 test");
  if (! shtc3.begin()) {
    Serial.println("Couldn't find SHTC3");
    while (1) delay(1);
  }
  Serial.println("Found SHTC3 sensor");
  //
  analogReadResolution(12);
  while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens
  pinMode(PUMP_PIN, OUTPUT);
  analogReadResolution(12);

  //MQTT
  WiFi.begin(WIFI_SSID, WIFI_PASS);
	while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(WiFi.status());
  }
	Serial.println("connected to wifi");

	mqtt_client.setClient(wifi_client);
	mqtt_client.setServer(MQTT_HOST, MQTT_PORT);

	mqtt_client.connect(CLIENT_ID);
	if (mqtt_client.state() == 0) Serial.println("connected to mqtt");
  
}



void loop() {
  mqtt_client.loop();

  now = millis();
  if (now - lastMeasure > 1000){
    lastMeasure = now;
    if (!mqtt_client.connected()) {
      mqtt_client.connect(CLIENT_ID);
    }
    sensors_event_t humidity, temp;
    shtc3.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
  
    Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
    Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");
    int rawValue = analogRead(SOIL_PIN);
    // 1. Calculate Soil Moisture Percentage (0% to 100%)
    int moisturePercent = map(rawValue, VAL_DRY, VAL_WET_SOIL, 0, 100);
    moisturePercent = constrain(moisturePercent, 0, 100);

    // 2. Calculate Standing Water Depth in cm
    float waterDepthCm = 0.0;
  
    if (rawValue <= VAL_1CM) {
      if (rawValue > VAL_2CM) {
        waterDepthCm = 1.0 + ((VAL_1CM - rawValue) / (float)(VAL_1CM - VAL_2CM));
      } else if (rawValue > VAL_3CM) {
        waterDepthCm = 2.0 + ((VAL_2CM - rawValue) / (float)(VAL_2CM - VAL_3CM));
      } else if (rawValue > VAL_4CM) {
        waterDepthCm = 3.0 + ((VAL_3CM - rawValue) / (float)(VAL_3CM - VAL_4CM));
      } else if (rawValue > VAL_5CM) {
        waterDepthCm = 4.0 + ((VAL_4CM - rawValue) / (float)(VAL_4CM - VAL_5CM));
      } else if (rawValue > VAL_6CM) {
        waterDepthCm = 5.0 + ((VAL_5CM - rawValue) / (float)(VAL_5CM - VAL_6CM));
      } else {
        waterDepthCm = 6.0; // Max calibrated depth
      }
    }

    // publish 
    char msg[20];

    sprintf(msg, "%d", rawValue);
    mqtt_client.publish("moisture", msg);

    dtostrf(temp.temperature, 4, 2, msg);
    mqtt_client.publish("temp", msg);

    dtostrf(humidity.relative_humidity, 4, 2, msg);
    mqtt_client.publish("humidity", msg);

    // 3. Print as a JSON string for easy web server parsing
    Serial.print("{\"raw\":");
    Serial.print(rawValue);
    Serial.print(", \"moisture_percent\":");
    Serial.print(moisturePercent);
    Serial.print(", \"water_depth_cm\":");
    Serial.print(waterDepthCm, 2); 
    Serial.println("}");

    /* 
    // EXAMPLE WATERING LOGIC:
    if (moisturePercent < 20) {
      // turnPumpOn();
    } else if (waterDepthCm >= 2.5) {
      // turnPumpOff(); // Stop when water level reaches 2.5cm
    }
    */
    // Dry threshold: Turn pump ON
  // Serial.print(rawValue);
    if(rawValue >= 2000){
      Serial.println(" Pump ON");
      digitalWrite(PUMP_PIN, HIGH);  // Active-LOW relay turns ON
    } 
    // Wet threshold: Turn pump OFF
    else {
      Serial.println(" Pump OFF");
      digitalWrite(PUMP_PIN, LOW); // Active-LOW relay turns OFF
    }
      Serial.println("\n\n\n");
  }
}