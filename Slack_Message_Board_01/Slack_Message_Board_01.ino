/* ESP8266 plus MAX7219 LED Matrix that displays messages revecioved via a WiFi connection using a Web Server
  #######################################################################################################################################
  This software, the ideas, and concepts is Copyright (c) David Bird 2018. All rights to this software are reserved.

  Any redistribution or reproduction of any part or all of the contents in any form is prohibited other than the following:
  1. You may print or download to a local hard disk extracts for your personal and non-commercial use only.
  2. You may copy the content to individual third parties for their personal use, but only if you acknowledge the author David Bird as the source of the material.
  3. You may not, except with my express written permission, distribute or commercially exploit the content.
  4. You may not transmit it or store it in any other website or other form of electronic retrieval system for commercial purposes.

  The above copyright ('as annotated') notice and this permission notice shall be included in all copies or substantial portions of the Software and where the
  software use is visible to an end-user.

  THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT. FOR PERSONAL USE IT IS SUPPLIED WITHOUT WARRANTY
  OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  See more at http://www.dsbird.org.uk


  ## App url for Slack: https://slack.com/oauth/v2/authorize?client_id=1717155021442.3243511379959&scope=users.profile:read&user_scope=users.profile:read
*/

//################# LIBRARIES ##########################
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <ArduinoJson.h>
#include <TaskScheduler.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include <AsyncWebConfig.h>

String params = "["
                "{"
                "'name':'ssid',"
                "'label':'WLAN SSID',"
                "'type':" + String(INPUTTEXT) + ","
                "'default':''"
                "},"
                "{"
                "'name':'pwd',"
                "'label':'WLAN Password',"
                "'type':" + String(INPUTPASSWORD) + ","
                "'default':''"
                "},"

                "{"
                "'name':'slackToken',"
                "'label':'Slack Token',"
                "'type':" + String(INPUTTEXT) + ","
                "'default':''"
                "},"

                "{"
                "'name':'slackUser',"
                "'label':'Slack User',"
                "'type':" + String(INPUTTEXT) + ","
                "'default':''"
                "}"

                "]";


AsyncWebServer server(80);
AsyncWebConfig conf;

// We declare the function that we are going to use
void httpRequest();

// We create the Scheduler that will be in charge of managing the tasks
Scheduler runner;

// We create the task indicating that it runs every 500 milliseconds, forever, and call the led_blink function
Task getSlackStatus(60000, TASK_FOREVER, &httpRequest);

int pinCS = D4; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays   = 1;
char time_value[20];
String message, myStatus, serverName , slackToken;
DynamicJsonDocument doc(512);
//https://arduinojson.org/v6/assistant/
//################# DISPLAY CONNECTIONS ################
// LED Matrix Pin -> ESP8266 Pin
// Vcc            -> 3v  (3V on NodeMCU 3V3 on WEMOS)
// Gnd            -> Gnd (G on NodeMCU)
// DIN            -> D7  (Same Pin for WEMOS)
// CS             -> D4  (Same Pin for WEMOS)
// CLK            -> D5  (Same Pin for WEMOS)

//################ PROGRAM SETTINGS ####################

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);
int wait   = 120; // In milliseconds between scroll movements
int spacer = 1;
int width  = 5 + spacer; // The font width is 5 pixels
String SITE_WIDTH =  "1000";

void setup() {
  Serial.begin(115200); // initialize serial communications
  Serial.println("Please autherize the App to your Slack workspace by visiting this url:\n App url for Slack: https://slack.com/oauth/v2/authorize?client_id=1717155021442.3243511379959&scope=users.profile:read&user_scope=users.profile:read");
  matrix.setIntensity(2);    // Use a value between 0 and 15 for brightness
  matrix.setRotation(0, 1);  // The first display is position upside down
  matrix.setRotation(1, 1);  // The first display is position upside down
  matrix.setRotation(2, 1);  // The first display is position upside down
  matrix.setRotation(3, 1);  // The first display is position upside down
  //  wait    = 50;
  //  message = "Hello, I'm Mujahed.. Here is my status";
  //  display_message(message); // Display the message
  //  wait    = 80;
  //  message = "Welcome...";

  //  message = "Connecting to " + String(ssid);
  //  display_message(message);
  //  message = "Connecting...";

  //  Serial.println(params);
  conf.setDescription(params);
  Serial.println("read config");
  conf.readConfig();
  serverName = "https://slack.com/api/users.profile.get?user=" + String(conf.values[3].c_str());
  slackToken = "Bearer " + String(conf.values[2].c_str());
  initWiFi();
  //  Serial.println(serverName);
  char dns[30];
  sprintf(dns, "%s.local", conf.getApName());
  if (MDNS.begin(dns)) {
    Serial.println("MDNS responder gestartet");
  }
  server.on("/", handleRoot);
  server.begin();

  runner.addTask(getSlackStatus);

  // We activate the task
  getSlackStatus.enable();
}
void loop() {

  MDNS.update();
  runner.execute();

  deserializeJson(doc, myStatus);

  JsonObject obj = doc.as<JsonObject>();
  String Status = obj["profile"]["status_text"].as<String>();
  wait    = 120;
  display_message(Status); // Display the message
  delay (500);

}
boolean initWiFi() {
  boolean connected = false;
  WiFi.mode(WIFI_STA);
  Serial.print("Connecting to ");
  Serial.print(conf.values[0]);
  if (conf.values[0] != "") {
    WiFi.begin(conf.values[0].c_str(), conf.values[1].c_str());
    uint8_t cnt = 0;
    while ((WiFi.status() != WL_CONNECTED) && (cnt < 20)) {
      delay(500);
      Serial.print(".");
      cnt++;
    }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("IP-Adresse = ");
      Serial.println(WiFi.localIP());
      connected = true;
    }
  }
  if (!connected) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(conf.getApName(), "", 1);
  }
  return connected;
}

void handleRoot(AsyncWebServerRequest *request) {
  conf.handleFormRequest(request);
  if (request->hasParam("SAVE")) {
    uint8_t cnt = conf.getCount();
    Serial.println("*********** Configuration ************");
    for (uint8_t i = 0; i < cnt; i++) {
      Serial.print(conf.getName(i));
      Serial.print(" = ");
      Serial.println(conf.values[i]);
    }

  }
}
String httpGETRequest(String serverName, String token) {
  WiFiClientSecure client;
  client.setInsecure(); //the magic line, use with caution
  client.connect("https://slack.com", 443);
  HTTPClient http;
  http.begin(client, serverName);


  http.addHeader("Authorization", token , true);

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");


  // Send HTTP POST request
  int httpResponseCode = http.GET();
  DynamicJsonDocument doc(400);

  String payload = "{}";

  if (httpResponseCode > 0) {
    //    Serial.print("HTTP Response code: ");
    //    Serial.println(httpResponseCode);
    payload = http.getString();
    deserializeJson(doc, payload);
    JsonObject obj = doc.as<JsonObject>();
    bool Status = obj["ok"].as<bool>();
    if (Status != true) {
      payload = "Failed to fetch user status";

    }
    //    Serial.println(payload);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}


void display_message(String message) {
  for ( int i = 0 ; i < width * message.length() + matrix.width() - spacer; i++ ) {
    //matrix.fillScreen(LOW);
    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically
    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < message.length() ) {
        matrix.drawChar(x, y, message[letter], HIGH, LOW, 1); // HIGH LOW means foreground ON, background OFF, reverse these to invert the display!
      }
      letter--;
      x -= width;
    }
    matrix.write(); // Send bitmap to display
    delay(wait / 2);
  }
}
void httpRequest() {

  myStatus = httpGETRequest(serverName, slackToken);

}
