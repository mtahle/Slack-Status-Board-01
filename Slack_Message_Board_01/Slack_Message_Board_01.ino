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
#include "configs.h"

// We declare the function that we are going to use
void httpRequest();

// We create the Scheduler that will be in charge of managing the tasks
Scheduler runner;

// We create the task indicating that it runs every 500 milliseconds, forever, and call the led_blink function
Task TareaLED(60000, TASK_FOREVER, &httpRequest);

int pinCS = D4; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays   = 1;
char time_value[20];
String message, myStatus;

//################# DISPLAY CONNECTIONS ################
// LED Matrix Pin -> ESP8266 Pin
// Vcc            -> 3v  (3V on NodeMCU 3V3 on WEMOS)
// Gnd            -> Gnd (G on NodeMCU)
// DIN            -> D7  (Same Pin for WEMOS)
// CS             -> D4  (Same Pin for WEMOS)
// CLK            -> D5  (Same Pin for WEMOS)

//################ PROGRAM SETTINGS ####################

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);
int wait   = 75; // In milliseconds between scroll movements
int spacer = 1;
int width  = 6 + spacer; // The font width is 5 pixels
String SITE_WIDTH =  "1000";

void setup() {
  Serial.begin(115200); // initialize serial communications
  WiFi.begin(ssid, password);             // Connect to the network
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

  message = "Connecting to " + String(ssid);
  display_message(message);
  message = "Connecting...";
  WiFi.begin(ssid, password);             // Connect to the network

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');

    display_message(message); // Display the message
    message += String(".");
  }

  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
  // We add the task to the task scheduler
  runner.addTask(TareaLED);

  // We activate the task
  TareaLED.enable();
}
void loop() {
  runner.execute();

  DynamicJsonDocument doc(400);
  deserializeJson(doc, myStatus);

  JsonObject obj = doc.as<JsonObject>();
  String Status = obj["profile"]["status_text"].as<String>();
  wait    = 120;
  display_message(Status); // Display the message
delay (500);

}

String httpGETRequest(String serverName) {
  WiFiClientSecure client;
  client.setInsecure(); //the magic line, use with caution
  client.connect(serverName, 443);
  HTTPClient http;
  http.begin(client, serverName);

  http.addHeader("Authorization", slackToken, true);

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");


  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
    Serial.println(payload);
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

  myStatus = httpGETRequest(serverName);

}
