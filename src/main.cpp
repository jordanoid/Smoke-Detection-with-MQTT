#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <TroykaMQ.h>

// Update these with values suitable for your network.

const char* ssid = "Redmi Note 10 Pro"; // Ganti dengan SSID
const char* password = "123456789"; // Ganti dengan password
const char* mqtt_server = "broker.mqtt-dashboard.com";

#define sensor A0
#define buzzer D5
bool buzzerToggle = false;
int sensorValue;

#define sub_QoS 0
byte willQoS = 2;
// const char* willTopic = "SisTer2022/Kel15/WillTopic";
// const char* willMessage = "Publisher Offline";
// bool willRetain = true;

MQ2 mq2(sensor);
WiFiClient espClient;
PubSubClient mqtt(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if ((char)payload[0] == '1'){
    buzzerToggle = true; // mengaktifkan
  }else{
    buzzerToggle = false;  // Mematikan buzzer
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqtt.connect(clientId.c_str())) {
      Serial.println("connected");
      mqtt.subscribe("SisTer2022/Kel15/buzzerToggle", sub_QoS);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(sensor, INPUT);
  pinMode(buzzer, OUTPUT);
  Serial.begin(115200);
  mq2.calibrate();
  setup_wifi();
  mqtt.setServer(mqtt_server, 1883);
  mqtt.setCallback(callback);
  mqtt.setKeepAlive(30); //keep alive 30 detik
}

void loop() {

  if (!mqtt.connected()) {
    reconnect();
  }
  mqtt.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    mqtt.subscribe("SisTer2022/Kel15/buzzerToggle", sub_QoS);
    if (buzzerToggle == true && sensorValue > 70) { // membunyikan buzzer apabila toggle on dan bacaan diatas threshold
      digitalWrite(buzzer, HIGH);
      delay(250);
      digitalWrite(buzzer, LOW);
      delay(250);
    }
    sensorValue = mq2.readSmoke();
    Serial.print("Bacaan sensor: ");
    Serial.println(sensorValue);
    mqtt.publish("SisTer2022/Kel15/MQ2", String(sensorValue).c_str());
  }
}