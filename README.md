# GoogleSay
ESP8266 making Google Home speak!

[![ESP8266 Google Home](https://img.youtube.com/vi/aWBr0WYP6Zs/0.jpg)](https://www.youtube.com/watch?v=aWBr0WYP6Zs)

### Prerequisites/Limitations
* The **GoogleHomeName** must match the name of Google Home / mini setup in Google Home App.
* The ESP8266 must be on the same network as Google Home.
* MDNS: Dont include ESP8266mDNS.h supplied by Arduino-ESP8266. Internal ESP8266mDNS.h should suffice.

## Libraries required
* [esp8266-google-home-notifier](https://github.com/horihiro/esp8266-google-home-notifier)
* [esp8266-google-tts](https://github.com/horihiro/esp8266-google-tts)
* [WiFiManager](https://github.com/tzapu/WiFiManager) (optional)
