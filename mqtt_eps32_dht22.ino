#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

const int DHT_PIN = 26;
const char* ssid = "YOUR_WIFI_SSID"; // wifi ssid
const char* password = "YOUR_PASS";
const char* mqtt_server = "YOUR_MOSQUITTO_SERVER_URL"; // mosquitto server url

DHT sensor_dht(DHT_PIN, DHT22);
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0; 

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Wifi terkoneksi ke : ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi berhasil terkoneksi");
  Serial.print("Alamat IP : ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Baru melakukan koneksi MQTT ...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  sensor_dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;

    float temp = sensor_dht.readTemperature();
    float hum = sensor_dht.readHumidity();

    StaticJsonDocument<200> doc;
    doc["temperature"] = temp;
    doc["humidity"] = hum;

    char buffer[512];
    serializeJson(doc, buffer);

    client.publish("/benzill_sensor/data", buffer);

    Serial.println("Data Terkirim:");
    Serial.println(buffer);
  }
  delay(15000);
}
