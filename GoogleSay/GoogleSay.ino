#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <esp8266-google-home-notifier.h>  //https://github.com/horihiro/esp8266-google-home-notifier
                                           //https://github.com/horihiro/esp8266-google-tts
//#include "secret.h"
#include "version.h"

#define USE_WIFIMANAGER
#ifdef USE_WIFIMANAGER
  #include <WiFiManager.h>                 //https://github.com/tzapu/WiFiManager
#else
  #ifndef SECRET
    const char* ssid     = "<WiFiSSID>";
    const char* password = "<WiFiPassword>";
  #endif
#endif

const char GoogleHomeName[] = "Living Room speaker";  //Must match the name on Google Home APP

const char *host= "GoogleSay";
String ttstext = "Welcome to Esp8266 TTS";

ESP8266WebServer server(80);
GoogleHomeNotifier ghn;

void handleRoot() {
  String webpage;
  webpage =  "<html>";
  webpage += "<meta name='viewport' content='width=device-width,initial-scale=1' />";
   webpage += "<head><title>Make Google Home Speak</title>";
    webpage += "<style>";
     webpage += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
    webpage += "</style>";
   webpage += "</head>";
   webpage += "<body>";
    webpage += "<br>";  
    webpage += "<form action='/processspeak' method='POST'>";
     webpage += "<center><input type='text' name='search_input' value='" + ttstext + "' placeholder='Google Home Text' size='75' style='text-align:center;'></center><br>";
     webpage += "<center><input type='submit' value='Speak!'></center>";
    webpage += "</form>";
   webpage += "</body>";
  webpage += "</html>";
  server.send(200, "text/html", webpage); // Send a response to the client asking for input
}

void processSpeak(){
  if (server.args() > 0 and server.method() == HTTP_POST) { // Arguments were received
    for ( uint8_t i = 0; i < server.args(); i++ ) {
      Serial.print(server.argName(i)); // Display the argument
      if (server.argName(i) == "search_input") {
        Serial.print(" : ");
        Serial.println(server.arg(i));
        ttstext=server.arg(i);
        if (ghn.notify(ttstext.c_str()) != true) {
          Serial.println(ghn.getLastError());
          return;
        }
      }
    }
  }
  
  String t;
  t += "<html>";
  t += "<head>";
  t += "<meta name='viewport' content='width=device-width,initial-scale=1' />";
  t += "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' />";
  t += "<meta http-equiv='refresh' content='3;url=/'/>";
  t += "</head>";
  t += "<body>";
  t += "<center><p>Saying: " + ttstext + "</p></center>";
  t += "<br><center><a href='/'>Update again?</a></center>";
  t += "</form>";
  t += "</body>";
  t += "</html>";
  server.send(200, "text/html", t);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void connectToGH(){
  Serial.println("connecting to Google Home...");
  if (ghn.device(GoogleHomeName, "en") != true) {
    Serial.println(ghn.getLastError());
    return;
  }
  Serial.print("found Google Home(");
  Serial.print(ghn.getIPAddress());
  Serial.print(":");
  Serial.print(ghn.getPort());
  Serial.println(")");
}

#ifndef USE_WIFIMANAGER
void connectWiFiConfig(void) {
  Serial.println("\n"); Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  int wifi_loops=0, wifi_timeout = WIFI_TIMEOUT_DEF;
  while (WiFi.status() != WL_CONNECTED) {
    wifi_loops++; Serial.print("."); delay(500);
    if (wifi_loops>wifi_timeout) software_reset();
  }
  Serial.println("connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
#else
void connectWiFiManager() {
  WiFiManager wifiManager;
  wifiManager.setDebugOutput(true);
  wifiManager.setTimeout(180);
  if (!wifiManager.autoConnect(const_cast<char*>(host))) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }
  Serial.println("connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void resetWlan(){
  Serial.printf("/reset_wlan\n");
  server.send(200, "text/plain", "Resetting WLAN and restarting..." );
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  ESP.restart();
}
#endif

String ipToString(IPAddress ip){
  String s="http:// ";
  for (int i=0; i<4; i++)
    s += i  ? " dot " + String(ip[i]) : String(ip[i]);
  return s;
}

void setup(void) {

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  
  #ifndef USE_WIFIMANAGER
    connectWiFiConfig();  // Connect to WiFi
  #else
    connectWiFiManager(); // Connect to WiFi
  #endif

  server.on("/", handleRoot);
  server.on("/processspeak", processSpeak);
  #ifdef USE_WIFIMANAGER
    server.on("/reset_wlan", resetWlan);
  #endif
  server.on("/version", [](){
    String replyjson = "{\"version\":\"" + String(SKETCH_VERSION) + "\"}";
    server.send(200, "application/json", replyjson);
  });
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  
  if (MDNS.begin(host)) {
    Serial.println("MDNS responder started");
  }

  connectToGH();

  String init_text = "Goto " + ipToString(WiFi.localIP()) + " on your web browser to make me speak!";

  if (ghn.notify(init_text.c_str()) != true) {
    Serial.println(ghn.getLastError());
    return;
  }
  Serial.println("Done.");
}

void loop(void) {
  server.handleClient();
}
