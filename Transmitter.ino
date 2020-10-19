#include <NewRemoteTransmitter.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Wifi connection details
const char* ssid = "wifi ssid";
const char* password = "password";

// Details about KlikAanKlikUit remote unit
const long address = 25855686;
const byte pin = 4;
const byte unit = 0;
const int periodusec = 259;
const byte repeats = 4;

// MQTT broker IP
const char* mqtt_server = "ip address";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting up...");

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  delay(5000);

}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/shed-light") {
    Serial.print("Changing shed light to ");
    if(messageTemp == "ON"){
      Serial.println("ON");
      transmit(true);
      client.publish("esp32/shed-light-status", "ON");
    }
    else if(messageTemp == "OFF"){
      Serial.println("OFF");
      transmit(false);
      client.publish("esp32/shed-light-status", "OFF");
    }
  }
}


void transmit(bool value){
  NewRemoteTransmitter transmitter(address, pin, periodusec, repeats);
  transmitter.sendUnit(unit, value);
  Serial.println("Sent!");
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/shed-light");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    client.publish("esp32/status", "up");
  }
}
