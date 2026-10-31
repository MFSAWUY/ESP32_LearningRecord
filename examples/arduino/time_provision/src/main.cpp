#include "app_network.h"
#include <WebServer.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <time.h>
WebServer portal(80);Preferences prefs;
bool accessPoint=false,clockConfigured=false;String pendingSsid,pendingPassword;bool apply=false;
void setup(){
  Serial.begin(115200);pinMode(27,INPUT_PULLUP);prefs.begin("network",false);
  if(digitalRead(27)==LOW)prefs.clear();
  String ssid=prefs.getString("ssid",""),pass=prefs.getString("pass","");
  if(ssid.length()){WiFi.mode(WIFI_STA);WiFi.setAutoReconnect(true);WiFi.begin(ssid.c_str(),pass.c_str());}
  else{
    accessPoint=true;WiFi.mode(WIFI_AP);WiFi.softAP("ESP32-Setup","configure123");
    Serial.println("setup Wi-Fi password=configure123; browse http://192.168.4.1");
    portal.on("/",HTTP_GET,[]{portal.send(200,"text/html","<form method='POST' action='/save'>SSID <input name='ssid' maxlength='32'><br>Password <input type='password' name='password' maxlength='63'><button>Save</button></form>");});
    portal.on("/save",HTTP_POST,[]{
      String s=portal.arg("ssid"),p=portal.arg("password");
      if(s.length()==0||s.length()>32||p.length()>63||(p.length()>0&&p.length()<8)){portal.send(400,"text/plain","invalid SSID/password length");return;}
      pendingSsid=s;pendingPassword=p;apply=true;portal.send(200,"text/plain","Saved; rebooting. Hold GPIO27 button at reset to clear.");
    });portal.begin();
  }
}
void loop(){
  if(accessPoint){portal.handleClient();if(apply){prefs.putString("ssid",pendingSsid);prefs.putString("pass",pendingPassword);delay(500);ESP.restart();}delay(1);return;}
  static uint32_t last=0,retry=0;
  if(WiFi.status()!=WL_CONNECTED){if(millis()-retry>15000){retry=millis();WiFi.reconnect();Serial.println("retry stored Wi-Fi; hold GPIO27 on reset to clear");}delay(10);return;}
  if(!clockConfigured){configTzTime("CST-8","pool.ntp.org","time.nist.gov");MDNS.begin("esp32-lab");Serial.println(WiFi.localIP());clockConfigured=true;}
  if(millis()-last>1000){last=millis();struct tm now;if(getLocalTime(&now,10)){char out[32];strftime(out,sizeof(out),"%Y-%m-%d %H:%M:%S",&now);Serial.println(out);}else Serial.println("clock=unsynchronized");}
  delay(1);
}
