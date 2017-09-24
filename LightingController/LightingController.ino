#include <PubSubClient.h>
#include <ESP8266WiFi.h>


#define WIFI_AP "CATALINA_EXT"
#define WIFI_PASSWORD "th4m44kshallinh4ritth44arth"

#define USERNAME "bwharton"
#define TOKEN "e964b6ade2ab738f50deffdaf94d951f3baed3f0"
#define TOPIC "bwharton/f/seedling-lighting"


char thingsboardServer[] = "io.adafruit.com";

WiFiClient wifiClient;

PubSubClient client(wifiClient);

int status = WL_IDLE_STATUS;

void setup()
{
	pinMode(D0, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
	digitalWrite(D0, LOW);


	Serial.begin(115200);
	delay(10);
	InitWiFi();
	client.setServer(thingsboardServer, 1883);

	client.setCallback(callback);
}

void loop()
{
	if (!client.connected()) {
		reconnect();
	}

	client.loop();
}

void InitWiFi()
{
	Serial.println("Connecting to AP ...");
	// attempt to connect to WiFi network

	WiFi.begin(WIFI_AP, WIFI_PASSWORD);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("Connected to AP");
}


void reconnect() {
	// Loop until we're reconnected
	while (!client.connected()) {
		status = WiFi.status();
		if (status != WL_CONNECTED) {
			WiFi.begin(WIFI_AP, WIFI_PASSWORD);
			while (WiFi.status() != WL_CONNECTED) {
				delay(500);
				Serial.print(".");
			}
			Serial.println("Connected to AP");
		}
		Serial.print("Connecting to MQTT server ...");
		// Attempt to connect (clientId, username, password)
		if (client.connect("ESP8266 Device", USERNAME, TOKEN)) {
			Serial.println("[DONE]");

			client.subscribe(TOPIC);
		}
		else {
			Serial.print("[FAILED] [ rc = ");
			Serial.print(client.state());
			Serial.println(" : retrying in 5 seconds]");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}

void callback(char* topic, byte* payload, unsigned int length) {
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	for (int i = 0; i < length; i++) {
		Serial.print((char)payload[i]);
	}
	Serial.println();

	if ((char)payload[0] == '1') {
		Serial.println("Turn Switch On");
		digitalWrite(D0, HIGH);
	}
	else {
		Serial.println("Turn Switch Off");
		digitalWrite(D0, LOW);
	}

}

