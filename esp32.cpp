#include <WiFi.h>
#include <PubSubClient.h>

const char *ssid = "MB2"; 
const char *password = "271809271809"; 
const char *mqtt_broker = "192.168.1.45";
const char *topic = "parking/availability";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

#define RXD2 16
#define TXD2 17

String serialData = "";

void setup() {
  Serial.begin(9600);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

  client.setServer(mqtt_broker, mqtt_port);
  client.setKeepAlive(60); // Set keep-alive interval to 60 seconds

  reconnect();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  while (Serial2.available()) {
    char c = Serial2.read();
    serialData += c;
  }

  if (serialData.length() > 0) {
    Serial.println(serialData);
    if (!client.publish(topic, serialData.c_str())) {
      Serial.println("Publish failed, attempting to reconnect...");
      reconnect();
    }
    serialData = ""; // Clear buffer
  }
}

void reconnect() {
  while (!client.connected()) {
    String client_id = "esp8266-client-";
    client_id += String(WiFi.macAddress());
    if (client.connect(client_id.c_str())) {
      Serial.println("Reconnected to MQTT broker");
    } else {
      delay(2000);
    }
  }
}
