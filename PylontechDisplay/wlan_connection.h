#include <ESP8266WiFi.h>
#include <time.h>
#include <TZ.h>

// Update these with values suitable for your network.

class WlanConnection{
  private:
  char* ssid;
  char* password;

  public:
  WlanConnection(char* ssid, char* password){
    this->ssid = ssid;
    this->password = password;
  }

  void setup_wifi() {
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print(F("Connecting to "));
    Serial.println(this->ssid);
  
    WiFi.mode(WIFI_STA);
    WiFi.begin(this->ssid, this->password);
  
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(F("."));
    }
  
    randomSeed(micros());
  
    Serial.println("");
    Serial.println(F("WiFi connected"));
    Serial.println(F("IP address: "));
    Serial.println(WiFi.localIP());
  }
  
  
  void setDateTime() {
    // You can use your own timezone, but the exact time is not used at all.
    // Only the date is needed for validating the certificates.
    configTime(TZ_Europe_Berlin, "pool.ntp.org", "time.nist.gov");
  
    Serial.print(F("Waiting for NTP time sync: "));
    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2) {
      delay(100);
      Serial.print(F("."));
      now = time(nullptr);
    }
    Serial.println();
  
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    Serial.printf("%s %s", tzname[0], asctime(&timeinfo));
  }
};
