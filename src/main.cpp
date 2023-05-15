#include <Arduino.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>

SoftwareSerial arduino(2, 3);
#define wifiPin 5
unsigned int prevTime;
unsigned int intervalMillis;

// Provide the token generation process info.
#include <addons/TokenHelper.h>
// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>
/* 1. Define the WiFi credentials */
#define WIFI_SSID "Shoaib"
#define WIFI_PASSWORD "A0123456789"
/* 2. Define the API Key */
#define API_KEY "AIzaSyC7Qu5XfM-_jXhLYT7gmbgdctG6U7UexL0"
/* 3. Define the RTDB URL */
#define DATABASE_URL "home-automation-3fbc1-default-rtdb.firebaseio.com" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "iotDevice@gmail.com"
#define USER_PASSWORD "device123"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Define variables to store sensor values and appliance states
uint8_t gasSensorValue;
uint8_t waterLevelSensorValue;
bool waterPumpState;
bool lightState;
bool fanState;
bool lockState;
bool updated; // keeps track of things that changed.
uint8_t tempInside, tempOutside, humInside, humOutside;

void setup()
{
  Serial.begin(9600);
  arduino.begin(9600);
  Serial.println("hello world");
  pinMode(wifiPin, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting");
  int cnt;

  while (WiFi.status() != WL_CONNECTED)
  {
    cnt++;
    Serial.print(".");
    delay(300);
    if (cnt >= 30)
      ESP.restart();
  }
  digitalWrite(wifiPin, HIGH); // to signal that its on.
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop()
{

  unsigned int now = millis();
  if (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(wifiPin, LOW); // controls the status led on pcb
  }
  else
  {
    digitalWrite(wifiPin, HIGH);
  }

  if (arduino.available())
  {
    DynamicJsonDocument doc(96);
    String message = arduino.readStringUntil('\n');
    Serial.print(message);
    DeserializationError error = deserializeJson(doc, message);

    if (error)
    {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.f_str());
      return;
    }
    gasSensorValue = doc["gasSensorValue"];
    waterLevelSensorValue = doc["waterLevelSensorValue"];
    waterPumpState = doc["waterPumpState"];
    lightState = doc["lightState"];
    fanState = doc["fanState"];
    lockState = doc["lockState"];
    tempInside = doc["temperatureInside"];
    tempOutside = doc["temperatureOutside"];
    humInside = doc["humidityInside"];
    humOutside = doc["humidityOutside"];

    if (Firebase.RTDB.setString(&fbdo, "dashboard/gas", gasSensorValue))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setString(&fbdo, "dashboard/lock", lockState))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setString(&fbdo, "dashboard/lights", lightState))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    } 

    if (Firebase.RTDB.setString(&fbdo, "dashboard/fans", fanState))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setString(&fbdo, "dashboard/waterpump", waterPumpState))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setString(&fbdo, "dashboard/temperature_inside", tempInside))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setString(&fbdo, "dashboard/temperature_outside", tempOutside))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setString(&fbdo, "dashboard/humidity_outside", humOutside))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setString(&fbdo, "dashboard/humidity_inside", humInside))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setString(&fbdo, "dashboard/water", waterLevelSensorValue))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setString(&fbdo, "dashboard/humidity_inside", humInside))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }

  // get node from and print to screen.
  // will update database every 1 second. 
  if (millis() - intervalMillis >= 1000)
  {
    bool prevLockState = lockState;
    bool prevFanState = fanState;
    bool prevLightState = lightState;
    bool prevWaterPumpState = waterPumpState;


// keep sending alive signal as long as device is online. 
    if (Firebase.RTDB.setString(&fbdo, "dashboard/alive", "1"))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.getString(&fbdo, "dashboard/fans"))
    {
      String data = fbdo.to<String>();
      fanState = data.toInt();
      Serial.print(fanState);
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.getString(&fbdo, "dashboard/lock"))
    {
      String data = fbdo.to<String>();
      lockState = data.toInt();
      Serial.print(lockState);
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.getString(&fbdo, "dashboard/lights"))
    {
      String data = fbdo.to<String>();
      lightState = data.toInt();
      Serial.print(lightState);
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.getString(&fbdo, "dashboard/waterpump"))
    {
      String data = fbdo.to<String>();
      waterPumpState = data.toInt();
      Serial.print(waterPumpState);
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (prevFanState != fanState)
    {
      arduino.print("A");
      delay(5);
    }
    if (prevLockState != lockState)
    {
      arduino.print("B");
    }
    if (prevLightState != lightState)
    {
      arduino.print("C");
    }
    if (prevWaterPumpState != waterPumpState)
    {
      arduino.write("D");
    }
    intervalMillis = millis();
  }
}
