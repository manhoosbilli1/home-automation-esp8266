#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <ArduinoJson.h>
SoftwareSerial arduino(5, 4); // d1,d2
#define wifiPin 14            // d5
#define SERIALREADY 12
unsigned int prevTime;
unsigned int intervalMillis;

// Provide the token generation process info.
#include <addons/TokenHelper.h>
// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>
/* 1. Define the WiFi credentials */
#define WIFI_SSID "robeel047"
#define WIFI_PASSWORD "33445566"
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

String message;
String pastMessage;
// Define variables to store sensor values and appliance states
String gasSensorValue;
String waterLevelSensorValue;
String waterPumpState;
String lightState;
String fanState;
String lockState;
String waterPumpStateF;
String lightStateF;
String fanStateF;
String lockStateF;
String tempInside, tempOutside, humInside, humOutside;
String prevLockState, prevLightState, prevFanState, prevWaterPumpState;

unsigned long sendDataPrevMillis = 0;
bool dataReceived = false;
int count = 0;

unsigned int long prevMillis;

void decode(String command);
String encode();
void updateAppliances()
{
  if (Firebase.RTDB.getString(&fbdo, "/dashboard/lock"))
  {
    String data = fbdo.to<String>();

    int tempData = data.toInt();
    if (tempData != lockState.toInt())
    {
      // means data is different than what came in. so first send the new data to arduino and then update database
      lockState = data; // updated current value
      arduino.println("#B");
      // update arduino at the end.
      // no need to set in firebase since thats where we got it from.
    }
    // if that's not true then by default our value will be that of the arduino.
    Serial.print("OK: lockState: ");
    Serial.println(lockState);
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }

  if (Firebase.RTDB.getString(&fbdo, "/dashboard/fans"))
  {
    String data = fbdo.to<String>();

    int tempData = data.toInt();
    if (tempData != fanState.toInt())
    {
      // means data is different than what came in. so first send the new data to arduino and then update database
      fanState = data; // updated current value
      arduino.println("#A");
      // update arduino at the end.
      // no need to set in firebase since thats where we got it from.
    }
    // if that's not true then by default our value will be that of the arduino.
    Serial.print("OK: fanState: ");
    Serial.println(fanState);
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }

  if (Firebase.RTDB.getString(&fbdo, "/dashboard/lights"))
  {
    String data = fbdo.to<String>();

    int tempData = data.toInt();
    if (tempData != lightState.toInt())
    {
      // means data is different than what came in. so first send the new data to arduino and then update database
      lightState = data; // updated current value
      arduino.println("#C");
      // update arduino at the end.
      // no need to set in firebase since thats where we got it from.
    }
    // if that's not true then by default our value will be that of the arduino.
    Serial.print("OK: lightstate: ");
    Serial.println(lightState);
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }

  if (Firebase.RTDB.getString(&fbdo, "/dashboard/waterpump"))
  {
    String data = fbdo.to<String>();

    int tempData = data.toInt();
    if (tempData != waterPumpState.toInt())
    {
      // means data is different than what came in. so first send the new data to arduino and then update database
      waterPumpState = data; // updated current value
      arduino.println("#D");
      // update arduino at the end.
      // no need to set in firebase since thats where we got it from.
    }
    // if that's not true then by default our value will be that of the arduino.
    Serial.print("OK: water pump state: ");
    Serial.println(waterPumpState);
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}

void getAppliancesState()
{
  if (Firebase.RTDB.getString(&fbdo, "/dashboard/lock"))
  {
    lockStateF = fbdo.to<String>();
    // if that's not true then by default our value will be that of the arduino.
    Serial.print("OK: lockState: ");
    Serial.println(lockStateF);
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }

  if (Firebase.RTDB.getString(&fbdo, "/dashboard/fans"))
  {
    fanStateF = fbdo.to<String>();
    // if that's not true then by default our value will be that of the arduino.
    Serial.print("OK: fanState: ");
    Serial.println(fanStateF);
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }

  if (Firebase.RTDB.getString(&fbdo, "/dashboard/lights"))
  {
    lightStateF = fbdo.to<String>();
    Serial.print("OK: lightstate: ");
    Serial.println(lightStateF);
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }

  if (Firebase.RTDB.getString(&fbdo, "/dashboard/waterpump"))
  {
    waterPumpStateF = fbdo.to<String>();
    Serial.print("OK: water pump state: ");
    Serial.println(waterPumpStateF);
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}
void setup()
{
  Serial.begin(9600);
  arduino.begin(9600);
  pinMode(SERIALREADY, OUTPUT);
  StaticJsonDocument<200> doc;
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
  while (!Firebase.ready())
  {
  }
  getAppliancesState(); // local copy of database set.
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

  // while (arduino.available() < 0)
  // {
  //   Serial.println("Waiting for arduino to send something");
  // }
  if (arduino.available())
  {
    Serial.readString();
  }
  digitalWrite(SERIALREADY, HIGH); // means ready
  delay(500);
  if (arduino.available())
  {
    message = arduino.readStringUntil('\n');
    digitalWrite(SERIALREADY, LOW);
    Serial.println("received data. now uploading...");
    Serial.println(message);
    decode(message);
    while (!Firebase.ready())
    {
      Serial.println("Waiting for firebase");
    }
    if (millis() - prevMillis >= 1000)
    {
      Serial.println("In firebase ready");
      if (Firebase.RTDB.setString(&fbdo, "/dashboard/gas", gasSensorValue))
      {
        Serial.println("Updated gas. ");
      }
      else
      {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setString(&fbdo, "/dashboard/humidity_outside", humOutside))
      {
        Serial.println("Updated hum outside. ");
      }
      else
      {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setString(&fbdo, "/dashboard/humidity_inside", humInside))
      {
        Serial.println("Updated hum inside. ");
      }
      else
      {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setString(&fbdo, "/dashboard/temperature_inside", tempInside))
      {
        Serial.println("Updated temp inside. ");
      }
      else
      {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setString(&fbdo, "/dashboard/temperature_outside", tempOutside))
      {
        Serial.println("Updated temp outside. ");
      }
      else
      {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setString(&fbdo, "/dashboard/water", waterLevelSensorValue))
      {
        Serial.println("Updated water level. ");
      }
      else
      {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setString(&fbdo, "/dashboard/alive", "1"))
      {
        Serial.println("Updated gas. ");
      }
      else
      {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
      // updateAppliances();
      // arduino.println(encode());
      Serial.println("sent to arduino");
      // pastMessage = message;
    }
    else
      // {
      //   // Print error to the "debug" serial port
      //   Serial.print("deserializeJson() returned ");
      //   Serial.println(err.c_str());

      // Flush all bytes in the "link" serial port buffer
      if (arduino.available() > 0)
      {
        arduino.readString();
        Serial.flush();
        arduino.flush();
      }
    prevMillis = millis();
  }
  // only enter if message is new
  getAppliancesState();
  digitalWrite(SERIALREADY, LOW);
  if (arduino.available())
  {
    Serial.readString(); // empty the buffer
    arduino.flush();
  }
  if (waterPumpState != waterPumpStateF && waterPumpStateF != NULL)
  {
    waterPumpState = waterPumpStateF;
    if (waterPumpState == "true" || waterPumpState == "1")
    {
      arduino.print("#D");
    }
    else if (waterPumpState == "false" || waterPumpState == "0")
    {
      arduino.print("#E");
    }
  }
  if (lightState != lightStateF && lightStateF != NULL)
  {
    lightState = lightStateF;
    if (lightState == "true" || lightState == "1")
    {
      arduino.print("#C");
    }
    else if (lightState == "false" || lightState == "0")
    {
      arduino.print("#F");
    }
  }
  if (fanState != fanStateF && fanStateF != NULL)
  {
    fanState = fanStateF;

    if (fanState == "true" || fanState == "1")
    {
      arduino.print("#A");
    }
    else if (fanState == "false" || fanState == "0")
    {
      arduino.print("#H");
    }
  }
  if (lockState != lockStateF && lockStateF != NULL)
  {
    lockState = lockStateF;

    if (lockState == "true" || lockState == "1")
    {
      arduino.print("#B");
      delay(500);
      if (Firebase.RTDB.setString(&fbdo, "/dashboard/lock", "0"))
      {
        Serial.println("Updated hum outside. ");
      }
      else
      {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
    }
    else if (lockState == "false" || lockState == "0")
    {
      arduino.print("#I");
    }
  }
  arduino.flush();
  Serial.flush();
}

// get node from and print to screen.
// will update database every 1 second.

String encode()
{
  String ptr;
  ptr += '@';
  ptr += String(fanState);
  ptr += ',';
  ptr += String(lightState);
  ptr += '!';
  ptr += String(waterPumpState);
  ptr += '#';
  ptr += String(lockState);
  ptr += '&';
  ptr += '\n';
  return ptr;
}

void decode(String command)
{
  gasSensorValue = command.substring(command.indexOf('@') + 1, command.indexOf(','));
  waterLevelSensorValue = command.substring(command.indexOf(',') + 1, command.indexOf('!'));
  tempInside = command.substring(command.indexOf('!') + 1, command.indexOf('-'));
  tempOutside = command.substring(command.indexOf('-') + 1, command.indexOf(')'));
  humInside = command.substring(command.indexOf(')') + 1, command.indexOf('('));
  humOutside = command.substring(command.indexOf('(') + 1, command.indexOf('*'));
}
