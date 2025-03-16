//import lib for WiFi
#include <WiFi.h>

#include "esp_wpa2.h"
// #include "esp_eap_client.h"
#include "esp_wifi.h"


//Replace with your network credentials
// TODO define your wifi credientials

//school WiFi credentials:
//  #define WIFI_USERNAME "wangca36"
//  #define WIFI_PASSWORD "BlueLemonadeCats87/"

//  #define WIFI_SSID "UofT"


const char* ssid = "UofT";
const char* username = "wangca36";
const char* password = "BlueLemonadeCats87/";

//home WiFi credentials:
// #define ssid "networkname"
// #define password "networkpassword"

//make a new webserver and set it to run on port 80
WiFiServer server(80);

//stores HTTP packet
String header;

//keep track of the light being on or off. Start with off for now
bool light_on = false;

//GPIO pin number to control our light
#define LIGHT_PIN 23

void setupWiFiEnterprise(const char* ssid, const char* username, const char* password) {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);

    esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)username, strlen(username));
    esp_wifi_sta_wpa2_ent_set_username((uint8_t *)username, strlen(username));
    esp_wifi_sta_wpa2_ent_set_password((uint8_t *)password, strlen(password));
    esp_wifi_sta_wpa2_ent_enable();

    WiFi.begin(ssid);

    Serial.print("Connecting to Wi-Fi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }

    Serial.println("\nConnected to Wi-Fi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void setup() {

  //set up the serial monitor so we can see debug messages
  Serial.begin(115200);
  
  //Initialize our LED pin as output to control the light
  pinMode(LIGHT_PIN, OUTPUT);
  
  //initially switch the light off
  digitalWrite(LIGHT_PIN, LOW);

    //Attempt to connect to WiFi
    Serial.print("Attempting connection to WiFi ");
    // WiFi.begin(ssid, password);

    //use this block of code until "STOP" comment below to use your home/mobile hotspot WiFi
    //Attempt to connect to WiFi
    // Serial.print("Connecting to WiFi ");
    // WiFi.begin(ssid, password);

    // //Keep looping until we get a successful connection
    // while (WiFi.status() != WL_CONNECTED) {
    //   delay(500); //wait 500 miliseconds before each attempt
    //   Serial.print(".");
    // }
    // STOP


    //use this block of code until "STOP" comment below to use school WiFi
    WiFi.disconnect(true);  //disconnect form wifi to set new wifi connection
    WiFi.mode(WIFI_STA);
    
    // esp_wifi_sta_wpa2_ent_set_username((uint8_t *)WIFI_USERNAME, strlen(WIFI_USERNAME));
    // esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)WIFI_USERNAME, strlen(WIFI_USERNAME)); // identity is the same as username identity
    // esp_wifi_sta_wpa2_ent_set_password((uint8_t *)WIFI_PASSWORD, strlen(WIFI_PASSWORD));
    // esp_wifi_sta_wpa2_ent_enable(); //set config settings to enable function
    // WiFi.begin(WIFI_SSID); //connect to wifi

    setupWiFiEnterprise(ssid, username, password);

    int counter = 0;
    //keep trying to connect to wifi until sucessful
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000); //wait 1000 milliseconds (1 second) before we attempt next
      Serial.print(".");
      counter++;
      if(counter>=60){ //after 30 seconds timeout - reset board
        ESP.restart();
      }
    }
    //STOP


  //Print local IP address and display it so we can
  //connect to the ESP32 with a web browser
  Serial.println("");
  Serial.println("Connection successful!");
  Serial.println("Please use IP address: ");
  Serial.println(WiFi.localIP());
  server.begin(); //start the web server now
}

void loop(){
  WiFiClient client = server.available(); //Block/wait until we get a new client connecting to the ESP32

  if (client) { //Start handling thenew client if we get a successful connection
    Serial.println("New client connected!");    
    String currentLine = ""; //keep track of data from HTTP packet
    while (client.connected()){ //keep processing until the client disconnects/leaves the connection
      if (client.available()){ //read data sent by client, if anything is sent
        
         //read the client input per-byte
        char c = client.read();            
        Serial.write(c);                    
        header += c;
        if (c == '\n') { //if the byte is a newline character, then we have a full line, process what we have here
          
          //Initial 
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /light/toggle") >= 0) {
              Serial.println("switching light");
              light_on = !light_on;

              if (light_on){
                digitalWrite(LIGHT_PIN, HIGH);
              }else{
                digitalWrite(LIGHT_PIN, LOW);
              }
            }
            
            // This code defines the HTTP file sent to the client. Modify the CSS/structure as you need!
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println("</style></head>");
            
            //website header + button to toggle
            client.println("<body><h1>UTM Robotics Club LED Web Server</h1>");
            client.println("<p><a href=\"/light/toggle\"><button class=\"button\">Click Me!</button></a></p>");
                          

            client.println("</body></html>");
            client.println();
            // Break out of the while loop since we're done sending the webpage. Wait for further client input from top of loop.
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') { //if you got anything else but a carriage return character,
          currentLine += c; //add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected");
    Serial.println("");
  }
}


// /*
//  * This ESP32 code is created by esp32io.com
//  *
//  * This ESP32 code is released in the public domain
//  *
//  * For more detail (instruction and wiring diagram), visit https://esp32io.com/tutorials/esp32-sound-sensor
//  */

// #define SENSOR_PIN 5 // ESP32 pin GPIO18 connected to the OUT pin of the sound sensor

// int lastState = HIGH;  // the previous state from the input pin
// int currentState;      // the current reading from the input pin

// void setup() {
//   // initialize serial communication at 9600 bits per second:
//   Serial.begin(9600);
//   // initialize the ESP32's pin as an input
//   pinMode(SENSOR_PIN, INPUT);
// }

// void loop() {
//   // read the state of the the ESP32's input pin
//   currentState = analogRead(SENSOR_PIN);
//   Serial.println("Sound data below");

//   Serial.println(currentState);

//   // if (lastState == HIGH && currentState == LOW)
//   //   Serial.println("The sound has been detected");
//   // else if (lastState == LOW && currentState == HIGH)
//   //   Serial.println("The sound has disappeared");

//   // // save the the last state
//   // lastState = currentState;
// }
