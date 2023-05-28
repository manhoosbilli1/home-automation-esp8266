#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <ArduinoJson.h>
SoftwareSerial arduino(5, 4); // d1,d2
#define wifiPin 14            // d5
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

void setup()
{
  Serial.begin(9600);
  arduino.begin(9600);
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

  if (arduino.available())
  {
    message = arduino.readStringUntil('\n');
    // if (message[0] == '@')
    // {
    //   decode(message);
    //   Serial.println(message);
    // }
    // else
    // {
    //   message = pastMessage;
    // }
    StaticJsonDocument<500> doc;

    // Read the JSON document from the "link" serial port
    DeserializationError err = deserializeJson(doc, message);

    if (err == DeserializationError::Ok)
    {
      gasSensorValue = doc["gasSensorValue"].as<String>();
      waterLevelSensorValue = doc["waterLevelSensorValue"].as<String>();
      waterPumpState = doc["waterPumpState"].as<String>();
      lightState = doc["lightState"].as<String>();
      fanState = doc["fanState"].as<String>();
      lockState = doc["lockState"].as<String>();
      tempInside = doc["temperatureInside"].as<String>();
      tempOutside = doc["temperatureOutside"].as<String>();
      humInside = doc["humidityInside"].as<String>();
      humOutside = doc["humidityOutside"].as<String>();
      Serial.print("gas: ");
      Serial.print(gasSensorValue);
      Serial.print(" , water sensor: ");
      Serial.print(waterLevelSensorValue);
      Serial.print(" , waterpumpstate: ");
      Serial.print(waterPumpState);
      Serial.print(" , light state: ");
      Serial.print(lightState);
      Serial.print(" , fan state: ");
      Serial.print(fanState);
      Serial.print(" , lockState: ");
      Serial.print(lockState);
      Serial.print(" , tempinside: ");
      Serial.print(tempInside);
      Serial.print(" , tempoutside: ");
      Serial.print(tempOutside);
      Serial.print(" , hum Inside: ");
      Serial.print(humInside);
      Serial.print(" , hum outside: ");
      Serial.println(humOutside);

      if (Firebase.ready() && (millis() - prevMillis >= 1000))
      {
        dataReceived = false;
        // if (message != pastMessage)
        // {
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
        updateAppliances();
        // arduino.println(encode());
        Serial.println("sent to arduino");
        // pastMessage = message;

        dataReceived = true;
      }
      else
      {
        // Print error to the "debug" serial port
        Serial.print("deserializeJson() returned ");
        Serial.println(err.c_str());

        // Flush all bytes in the "link" serial port buffer
        while (arduino.available() > 0)
          arduino.read();
      }
      
    prevMillis = millis();
    }
    // only enter if message is new
  }
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
    waterPumpState = command.substring(command.indexOf("!") + 1, command.indexOf("#"));
    lightState = command.substring(command.indexOf('#') + 1, command.indexOf('&'));
    fanState = command.substring(command.indexOf('&') + 1, command.indexOf('$'));
    lockState = command.substring(command.indexOf('$') + 1, command.indexOf('+'));
    tempInside = command.substring(command.indexOf('+') + 1, command.indexOf('-'));
    tempOutside = command.substring(command.indexOf('-') + 1, command.indexOf(')'));
    humInside = command.substring(command.indexOf(')') + 1, command.indexOf('('));
    humOutside = command.substring(command.indexOf('(') + 1, command.indexOf('*'));
  }
