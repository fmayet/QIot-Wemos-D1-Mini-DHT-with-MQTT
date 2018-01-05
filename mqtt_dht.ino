#include <ESP8266WiFi.h>
#include <PubSubClient.h>      // MQTT library 
#include <DHT.h>


#define DHTTYPE DHT22   // DHT Shield uses DHT 22
#define DHTPIN D4       // DHT Shield uses pin D4

// Existing WiFi network
const char* ssid     = "ssid"; // please fill
const char* password = "**********"; // please fill

// Data must be taken from the JSON file
const char* mqtt_server = "your_ip_address";
const int mqtt_port = 1338; // replace with real port
const char* mqtt_topic_temp = "your_topic_1"; //e.g. "qiot/things/admin/RaspberryPi/roomtemperature1"
const char* mqtt_topic_hum = "your_topic_2"; //e.g. "qiot/things/admin/RaspberryPi/roomhumidity1"
const char* mqtt_id = "your_id";
const char* mqtt_user = "your_user";
const char* mqtt_pass = "your_password";

DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);
int value = 0;
unsigned long previousMillis = 0;
// update interval
unsigned long interval = 60000;

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

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_id, mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      //resubscribe
      client.subscribe(mqtt_topic_temp);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature(false);
    if (isnan(humidity) || isnan(temperature)) {
      return;
    }
    Serial.print(humidity, 1);
    Serial.print("\t\t");
    Serial.print(temperature, 1);
    
    String msg_payload = "{\"value\":"+String(temperature)+"}";
    client.publish(mqtt_topic_temp, msg_payload.c_str());
    
    msg_payload = "{\"value\":"+String(humidity)+"}";
    client.publish(mqtt_topic_hum, msg_payload.c_str());
    
    previousMillis = currentMillis;
  }
}
