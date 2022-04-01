# Slack-Status-Board-01
This is an ESP8266 project on Arduino platform, were it retrieves a user profile status from Slack.

![](demo.gif)
## To get started:

### Prerequisite:
- Slack App token with profile read scope
### Hardware List:
- [Dotmatrix LED Display 8x32 max7219](https://www.amazon.com/s?k=dotmatrix+8x32+max7219&crid=OIBM16J3CVXL&sprefix=dotmatrix+8x32+max7219%2Caps%2C164&ref=nb_sb_noss)
- [ESP8266 Board](https://www.amazon.com/s?k=esp8266+wemose&crid=227ZO9DDYXSCO&sprefix=esp8266+wemos%2Caps%2C177&ref=nb_sb_noss)
### Arduino Libraries:
- [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson) 
- [AsyncTCP_SSL](https://github.com/khoih-prog/AsyncTCP_SSL)
- [AsyncWebConfig](https://github.com/GerLech/AsyncWebConfig)
- [ESP8266WiFi](https://github.com/esp8266/Arduino)
- [ESP8266HTTPClient](https://github.com/esp8266/Arduino)
- [WiFiClientSecure](https://github.com/esp8266/Arduino) 
- [TaskScheduler](https://github.com/arkhipenko/TaskScheduler)
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [Max72xxPanel](https://github.com/markruys/arduino-Max72xxPanel)

### Implementation:
- connect a Dotmatrix LED display 8x32 to any esp8266 device
- burn the code
- connect you to the WiFi AP named Slack-Status 
- browse to http://192.168.4.1 and set the proper configurations
