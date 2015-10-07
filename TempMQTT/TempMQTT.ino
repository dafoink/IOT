#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>


// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2
#define DEVICE_ID "100"
#define MAX_ATTACHED_DS18B20 16

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 100);
IPAddress server(192, 168, 1, 11);

int numSensors=0;
float lastTemp;

// this is the callback for when a message is received from the server
// TODO:  put code here to act on the received topic/payload
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

EthernetClient ethClient;
PubSubClient client(ethClient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) 
    {
      Serial.println("connected");
      // Once connected, publish an announcement...

      // subscribe to server requests to this device
      client.subscribe("ct/s/100/#");
    } 
    else 
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(57600);

  // set the broker location
  client.setServer(server, 1883);

  // set the callback for when a message is received
  client.setCallback(callback);

  // start ethernet
  Ethernet.begin(mac, ip);

  // start the temperature sensor
  sensors.begin();
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");
  
  delay(1500);
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  processSensors();
  delay(5000);
}

void processSensors()
{
  // Fetch temperatures from Dallas sensors
  sensors.requestTemperatures(); 

  // round to 2 decimals
  float temperature = (sensors.getTempFByIndex(0) * 10.) / 10.;
  if(temperature != lastTemp)
  {
    Serial.println(temperature);

    //Convert the float to a char*
    char buf[100];
    dtostrf(temperature, 4, 2, buf);

    // publish the results to Broker
    client.publish("ct/d/100/TEMP1",buf);

    // set the lastTemp to current temp so we dont send the same thing over and over
    lastTemp = temperature;
  }
}






