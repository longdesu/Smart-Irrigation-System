#include "Adafruit_SHTC3.h"

const int SOIL_PIN = 1;
const int PUMP_PIN = 4;

// --- Updated Calibration Constants ---
const int VAL_DRY = 2350;
const int VAL_WET_SOIL = 1550; // 100% moisture now aligns with 1cm of standing water

// Shifted water depth values
const int VAL_1CM = 1550; // New starting point
const int VAL_2CM = 1300; // Previously 1cm
const int VAL_3CM = 1113; // Previously 2cm
const int VAL_4CM = 1068; // Previously 3cm
const int VAL_5CM = 1012; // Previously 4cm
const int VAL_6CM = 981;  // Previously 5cm

Adafruit_SHTC3 shtc3 = Adafruit_SHTC3();

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  
  while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("SHTC3 test");
  if (! shtc3.begin()) {
    Serial.println("Couldn't find SHTC3");
    while (1) delay(1);
  }
  
  Serial.println("Found SHTC3 sensor");
  pinMode(PUMP_PIN, OUTPUT);
}

void loop() {
  sensors_event_t humidity, temp;
  shtc3.getEvent(&humidity, &temp); // populate temp and humidity objects with fresh data
  
  Serial.print("Temperature: "); 
  Serial.print(temp.temperature); 
  Serial.println(" degrees C");
  
  Serial.print("Humidity: "); 
  Serial.print(humidity.relative_humidity); 
  Serial.println("% rH");
  
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

  // 3. Print as a JSON string for easy web server parsing
  Serial.print("{\"raw\":");
  Serial.print(rawValue);
  Serial.print(", \"moisture_percent\":");
  Serial.print(moisturePercent);
  Serial.print(", \"water_depth_cm\":");
  Serial.print(waterDepthCm, 2); 
  Serial.println("}");
  
  // 4. Watering Logic
  // Dry threshold: Turn pump ON
  if(rawValue >= 2000){
    Serial.println(" Pump ON");
    digitalWrite(PUMP_PIN, HIGH);  
  } 
  // Wet threshold: Turn pump OFF
  else {
    Serial.println(" Pump OFF");
    digitalWrite(PUMP_PIN, LOW); 
  }
  
  delay(1000);
}
