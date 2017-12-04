#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "YOURSSIDHERE"
#define WLAN_PASS       "YOURWIFIPWDHERE"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "YourServerHere"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "MQTTUsername"
#define AIO_KEY         "MQTTPassword"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;


// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'treewater' for publishing.
Adafruit_MQTT_Publish treewater = Adafruit_MQTT_Publish(&mqtt, "bwharton/f/treewater");


/*************************** Sketch Code ************************************/

#define DIGITAL_INPUT_SOIL_SENSOR 5   // Digital input did you attach your soil sensor.  
#define LED_DISPLAY D0
int lastSoilValue = -1;

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
	Serial.begin(115200);
	delay(10);
	
	pinMode(LED_DISPLAY, OUTPUT);
	
	// Connect to WiFi access point.
	Serial.println(); Serial.println();
	Serial.print("Connecting to ");
	Serial.println(WLAN_SSID);

	WiFi.begin(WLAN_SSID, WLAN_PASS);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println();

	Serial.println("WiFi connected");
	Serial.println("IP address: "); Serial.println(WiFi.localIP());
}

uint32_t x = 0;

void loop() {
	// Ensure the connection to the MQTT server is alive (this will make the first
	// connection and automatically reconnect when disconnected).  See the MQTT_connect
	// function definition further below.
	MQTT_connect();

	checkSensor();


	// ping the server to keep the mqtt connection alive
	// NOT required if you are publishing once every KEEPALIVE seconds
	/*
	if(! mqtt.ping()) {
	mqtt.disconnect();
	}
	*/
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

void checkSensor()
{
	char msg[50];
	
	// Read digital soil value
	int soilValue = digitalRead(DIGITAL_INPUT_SOIL_SENSOR); // 1 = Not triggered, 0 = In soil with water 
	snprintf(msg, 75, "#%ld", soilValue);

	digitalWrite(LED_DISPLAY, soilValue);
	if (soilValue != lastSoilValue) {
		Serial.println(soilValue);
		if (!treewater.publish(soilValue)) {
			Serial.println("publish failed");
		}
		else {
			Serial.println("publish succeeded");
		}

		lastSoilValue = soilValue;
	}
}