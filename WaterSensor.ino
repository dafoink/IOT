#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <ESP8266WiFi.h>

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"


#define DIGITAL_INPUT_SOIL_SENSOR 5   // Digital input did you attach your soil sensor.  
#define LED_DISPLAY D0
int lastSoilValue = -1;


/************************* Adafruit.io Setup *********************************/
char aio_server[40] = "io.adafruit.com";
char aio_serverport[6] = "1883";
char aio_username[34] = "YOUR_AIO_USERNAME";
char aio_key[60] = "YOUR_AIO_KEY";
char deviceID[34] = "device1";

/************ Global State (you don't need to change this!) ******************/
// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
//Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Client mqtt(&client, aio_server, 1883, aio_username, aio_key);

/****************************** Feeds ***************************************/
Adafruit_MQTT_Publish sensor1 = Adafruit_MQTT_Publish(&mqtt, "");
/****************************** Feeds ***************************************/

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(LED_DISPLAY, OUTPUT);

  Serial.println(F("CatalinaIoT Node"));
  
  //SPIFFS.format();  
 
  readSPFFS();
  
  WiFiManagerParameter custom_aio_server("server", "aio server", aio_server, 40);
  WiFiManagerParameter custom_aio_port("port", "aio port", aio_serverport, 6);
  WiFiManagerParameter custom_aio_username("username", "aio username", aio_username, 34);
  WiFiManagerParameter custom_aio_key("key", "aio key", aio_key, 60);
  WiFiManagerParameter custom_deviceID("deviceID", "device ID", deviceID, 34);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //wifiManager.resetSettings();
  
  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  //add all your parameters here
  wifiManager.addParameter(&custom_aio_server);
  wifiManager.addParameter(&custom_aio_port);
  wifiManager.addParameter(&custom_aio_username);
  wifiManager.addParameter(&custom_aio_key);
  wifiManager.addParameter(&custom_deviceID);

  //exit after config instead of connecting
  wifiManager.setBreakAfterConfig(true);

  //tries to connect to last known settings
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP" with password "password"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("CatalinaIoT", "password")) {
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
  }
  Serial.println("Connected...AllRight!");
  Serial.println();


  //read updated parameters
  strcpy(aio_server, custom_aio_server.getValue());
  strcpy(aio_serverport, custom_aio_port.getValue());
  strcpy(aio_username, custom_aio_username.getValue());
  strcpy(aio_key, custom_aio_key.getValue());
  strcpy(deviceID, custom_deviceID.getValue());

  saveSFFS();
  
  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  setupFeeds();

  // sets the soil sensor digital pin as input
  pinMode(DIGITAL_INPUT_SOIL_SENSOR, INPUT);    

}

uint32_t x=0;

void loop() {
  MQTT_connect();

  checkSensor();
  
  delay(1000);
}

void checkSensor()
{
  // Read digital soil value
  int soilValue = digitalRead(DIGITAL_INPUT_SOIL_SENSOR); // 1 = Not triggered, 0 = In soil with water 
  digitalWrite(LED_DISPLAY, soilValue);

  if (soilValue != lastSoilValue) {
    Serial.println(soilValue);
    if (! sensor1.publish(soilValue)) 
    {
      Serial.println(F("Sensor1 publish Failed"));
    }
    else 
    {
      Serial.println(F("Sensor1 publish OK!"));
    }
  }
  lastSoilValue = soilValue;
}


// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}


void readSPFFS()
{
    //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(aio_server, json["aio_server"]);
          strcpy(aio_serverport, json["aio_serverport"]);
          strcpy(aio_username, json["aio_username"]);
          strcpy(aio_key, json["aio_key"]);
          strcpy(deviceID, json["deviceID"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  Serial.println("Done retrieving setup");
  //end read
}

void saveSFFS()
{
  //save the custom parameters to FS
  if (SPIFFS.begin()) {
    if (shouldSaveConfig) {
      Serial.println("saving config");
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.createObject();
      json["aio_server"] = aio_server;
      json["aio_serverport"] = aio_serverport;
      json["aio_username"] = aio_username;
      json["aio_key"] = aio_key;
      json["deviceID"] = deviceID;
  
      File configFile = SPIFFS.open("/config.json", "w");
      if (!configFile) {
        Serial.println("failed to open config file for writing");
      }
  
      json.printTo(Serial);
      json.printTo(configFile);
      configFile.close();
      //end save
    }
  }
}

void setupFeeds()
{
  const char* feedLocation = "/feeds/";

  char* sensorFeed1;
  const char* sensorFeed1Location = "-moisture1";
  sensorFeed1 = (char*)malloc(strlen(aio_username) + strlen(feedLocation) + strlen(deviceID) + strlen(sensorFeed1Location) + 2);
  strcpy(sensorFeed1, aio_username);
  strcat(sensorFeed1, feedLocation);
  strcat(sensorFeed1, deviceID);
  strcat(sensorFeed1, sensorFeed1Location);

  Serial.print("Publish location ");

  Serial.println(sensorFeed1);
  sensor1 = Adafruit_MQTT_Publish(&mqtt, (char*)sensorFeed1);
}

void resetSystem()
{
  WiFiManager wifiManager;
  SPIFFS.format();  
  wifiManager.resetSettings();
}
