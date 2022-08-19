#include "config.h"
#include "wlan_connection.h"
#include "PylontechMonitoring.h"
#include "OledWriter.h"


WlanConnection *wlanConnection;
PylontechMonitoring *p_monitoring;
OledWriter *oledWriter;


void setup() {
  pinMode(LED_BUILTIN, OUTPUT); 
  digitalWrite(LED_BUILTIN, HIGH);//high is off
  delay(500);
  Serial.begin(9600);
  delay(500);

  wlanConnection = new WlanConnection(WIFI_SSID, WIFI_PASS);
  wlanConnection->setup_wifi();
  wlanConnection->setDateTime();

  oledWriter = new OledWriter();
  oledWriter->setupDisplay();

  p_monitoring = new PylontechMonitoring();
  p_monitoring->setup();

  #ifdef ENABLE_MQTT
    mqttConnection = new MqttConnection(MQTT_SERVER, MQTT_PORT, MQTT_USER, MQTT_PASSWORD, MQTT_TOPIC_ROOT, MAX_PUSH_FREQ_SEC);
    mqttConnection->setup();
    mqttConnection->publish("pylonTechTestTopic", "startup d");
  #endif


}

int lastTime = 0;

void loop(){
  bool tts = (millis()/1000 - lastTime) > MAX_PUSH_FREQ_SEC;
  // TODO: delete, testing only
  oledWriter->drawText(String((millis()/1000 - lastTime)), OLED_TEXT_SIZE);
  //mqttConnection->publish("pylonTechTestTopic2", "lalalala");
  if(tts){
    //mqttConnection->publish("pylonTechTest17", "huhu");
    mqttConnection->publish("pylonTechTest16", String((millis()/1000 - lastTime)).c_str());
    oledWriter->drawText("tts", OLED_TEXT_SIZE);
    //pushLoop();
    lastTime = millis()/1000;
  }
  
  p_monitoring->loop();

}

void pushLoop(){
      // Read from Pylontech
      if(p_monitoring->sendCommandAndReadSerialResponse("pwr") == true){
        p_monitoring->parsePwrResponse(g_szRecvBuff);

        // Write To OLED
        oledWriter->drawText(String(p_monitoring->g_stack.soc) + "%", OLED_TEXT_SIZE);      

        // Do other stuff
        #ifdef ENABLE_MQTT
        mqttLoop();
        #endif

      
      }
}

#ifdef ENABLE_MQTT
void mqttLoop(){
      static PylontechMonitoring::batteryStack lastSentData; //this is the last state we sent to MQTT, used to prevent sending the same data over and over again
      static unsigned int callCnt = 0;
      bool forceUpdate = (callCnt % 20 == 0); //push all the data every 20th call
      p_monitoring->pushBatteryDataToMqtt(lastSentData, forceUpdate);
      
      callCnt++;
      memcpy(&lastSentData, &p_monitoring->g_stack, sizeof(PylontechMonitoring::batteryStack));
}
#endif //ENABLE_MQTT
