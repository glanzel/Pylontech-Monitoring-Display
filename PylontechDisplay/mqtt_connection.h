#include <string>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// A single, global CertStore which can be used by all connections.
// Needs to stay live the entire time any of the WiFiClientBearSSLs
// are present.
//BearSSL::CertStore certStore;

//WiFiClientSecure espClient;
PubSubClient * client;
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (500)
char msg[MSG_BUFFER_SIZE];
int value = 0;

class MqttConnection {

  private:
    char* mqtt_server;
    int mqtt_port;
    char* mqtt_user;
    char* mqtt_pw;
    std::string mqtt_topic_root;

  public:
    MqttConnection(char* mqtt_server, int mqtt_port, char* mqtt_user, char* mqtt_pw, std::string mqtt_topic_root, int mqtt_push_freq_sec) {
      this->mqtt_server = mqtt_server;
      this->mqtt_port = mqtt_port;
      this->mqtt_user = mqtt_user;
      this->mqtt_pw = mqtt_pw;
      this->mqtt_topic_root = mqtt_topic_root;
    }

    static void callback(char* topic, byte* payload, unsigned int length) {
      Serial.print("Message arrived [");
      Serial.print(topic);
      Serial.print("] ");
      for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
      }
      Serial.println();

      // Switch on the LED if the first character is present
      if ((char)payload[0] != NULL) {
        digitalWrite(LED_BUILTIN, LOW); // Turn the LED on (Note that LOW is the voltage level
        // but actually the LED is on; this is because
        // it is active low on the ESP-01)
        delay(500);
        digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
      } else {
        digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
      }
    }


    void reconnect() {
      // Loop until we’re reconnected
      while (!client->connected()) {
        Serial.print("Attempting MQTT connection…");
        String clientId = "ESP8266Client - MyClient";
        // Attempt to connect
        // Insert your password
        if (client->connect(clientId.c_str(), this->mqtt_user, this->mqtt_pw)) {
          Serial.println("connected");
          // Once connected, publish an announcement…
          client->publish("testTopic", "hello world");
          // … and resubscribe
          client->subscribe("testTopic");
        } else {
          Serial.print("failed, rc = ");
          Serial.print(client->state());
          Serial.println(" try again in 5 seconds");
          // Wait 5 seconds before retrying
          delay(5000);
        }
      }
    }

    void setup() {
      BearSSL::WiFiClientSecure *bear = new BearSSL::WiFiClientSecure();
      // Integrate the cert store with this connection
      // you can use the insecure mode, when you want to avPubSubClientificates
      bear->setInsecure();
      //bear->setCertStore(&certStore);

      client = new PubSubClient(*bear);

      client->setServer(this->mqtt_server, this->mqtt_port);
      client->setCallback(&MqttConnection::callback);
    }

    void publish(const char* topic, const char* msg) {
      if (!client->connected()) {
        Serial.print("reconnect");
        this->reconnect();
      }else Serial.print(client->connected());
      Serial.print("publish: "); Serial.print(topic); Serial.print(" : ");Serial.print(msg);
      
      if(client->publish(topic, msg)){
         Serial.println("         -> published: ");
      }else{
        Serial.println(" reconnect  needed");
        client->disconnect();
        this->reconnect();
        client->publish(topic, msg);
      }
    }

    void testloop() {
      if (!client->connected()) {
        this->reconnect();
      }
      client->loop();

      unsigned long now = millis();
      if (now - lastMsg > 2000) {
        lastMsg = now;
        ++value;
        snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
        Serial.print("Publish message: ");
        Serial.println(msg);
        client->publish("testTopic", msg);
      }
    }
#define ABS_DIFF(a, b) (a > b ? a-b : b-a)
    void mqtt_publish_f(std::string topic, float newValue, float oldValue, float minDiff, bool force)
    {
      char szTmp[16] = "";
      snprintf(szTmp, 15, "%.2f", newValue);
      if (force || ABS_DIFF(newValue, oldValue) > minDiff)
      {
        client->publish(( this->mqtt_topic_root + topic).c_str(), szTmp, false);
      }
    }

    void mqtt_publish_i(std::string topic, int newValue, int oldValue, int minDiff, bool force)
    {
      char szTmp[16] = "";
      snprintf(szTmp, 15, "%d", newValue);
      if (force || ABS_DIFF(newValue, oldValue) > minDiff)
      {
        client->publish(( this->mqtt_topic_root + topic).c_str(), szTmp, false);
      }
    }

    void mqtt_publish_s(std::string topic, const char* newValue, const char* oldValue, bool force)
    {
      if (force || strcmp(newValue, oldValue) != 0)
      {
        client->publish(( this->mqtt_topic_root + topic).c_str(), newValue, false);
      }
    }


};
