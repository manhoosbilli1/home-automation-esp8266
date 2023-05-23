#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>

SoftwareSerial arduino(5, 4); // d1,d2
#define wifiPin 14            // d5
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
String gasSensorValue;
String waterLevelSensorValue;
String waterPumpState;
String lightState;
String fanState;
String lockState;
String tempInside, tempOutside, humInside, humOutside;

unsigned int long prevMillis;

void decode(String command);
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
    yield();
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
  if (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(wifiPin, LOW); // controls the status led on pcb
  }
  else
  {
    digitalWrite(wifiPin, HIGH);
  }
  if(Firebase.ready()){}
  else {return;}
  intervalMillis = millis();
  String prevLockState = lockState;
  String prevFanState = fanState;
  String prevLightState = lightState;
  String prevWaterPumpState = waterPumpState;

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
    fanState = fbdo.to<String>();

    Serial.print(fanState);
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }

  if (Firebase.RTDB.getString(&fbdo, "dashboard/lock"))
  {
    lockState = fbdo.to<String>();
    Serial.print(lockState);
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }

  if (Firebase.RTDB.getString(&fbdo, "dashboard/lights"))
  {

    lightState = fbdo.to<String>();
    Serial.print(lightState);
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }

  if (Firebase.RTDB.getString(&fbdo, "dashboard/waterpump"))
  {
    waterPumpState = fbdo.to<String>();

    Serial.print(waterPumpState);
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }

  if (prevFanState != fanState)
  {
    Serial.flush();
    arduino.flush();
    arduino.print("A");
  }
  if (prevLockState != lockState)
  {
    Serial.flush();
    arduino.flush();
    arduino.print("B");
  }
  if (prevLightState != lightState)
  {
    Serial.flush();
    arduino.flush();
    arduino.print("C");
  }
  if (prevWaterPumpState != waterPumpState)
  {
    Serial.flush();
    arduino.flush();
    arduino.print("D");
  }

  if (arduino.available() > 0)
  {
    String string = arduino.readString();
    Serial.println(string);
    decode(string);

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
}
// get node from and print to screen.
// will update database every 1 second.

void decode(String command)
{
  gasSensorValue = command.substring(command.indexOf('@') + 1, command.indexOf(','));
  waterLevelSensorValue = command.substring(command.indexOf(',') + 1, command.indexOf('!'));
  waterPumpState = command.substring(command.indexOf("!") + 1, command.indexOf("#"));
  lightState = command.substring(command.indexOf('#') + 1, command.indexOf('&'));
  fanState = command.substring(command.indexOf('&') + 1, command.indexOf('$'));
  lockState = command.substring(command.indexOf('$') + 1, command.indexOf('+'));
  tempInside = command.substring(command.indexOf('+') + 1, command.indexOf('-'));
  tempOutside = command.substring(command.indexOf('-') + 1, command.indexOf(')'));
  humInside = command.substring(command.indexOf(')') + 1, command.indexOf('('));
  humOutside = command.substring(command.indexOf('(') + 1, command.indexOf('*'));
}