#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define pinData 2
OneWire ourWire(pinData);

DallasTemperature sensor(&ourWire);

String apiKey = "1CYEUX4FSWZVQRSY";
const char* apiServer = "api.thingspeak.com";

const char* ssid = "LOPEZ 2.4GHz";
const char* password = "00436035103";
//const char* ssid = "LOPEZ 5.8GHz";
//const char* password = "00436035103";
//const char* ssid = "iPhone de Facundo";
//const char* password = "Facu1234";
//const char* ssid = "iPhone de Leila Sofia";
//const char* password = "d0wkf62f3n4zo";

WiFiServer server(80);

// Variable to store the HTTP request
String header;
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 1000;

void setup() {
  
  Serial.begin(115200);
  delay(10);
  
  sensor.begin();
  
  IPAddress ip(192, 168, 10, 200);
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns(192, 168, 1, 249);
  
  WiFi.config(ip, gateway, subnet,dns);
  WiFi.begin(ssid, password);
  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());           //pido al router una IP
  
  server.begin();
}

void loop() {
  WiFiClient client;
  float temperatura = 0;
  float corriente = 0;
  int count = 0;
  sensor.requestTemperatures();
  temperatura = sensor.getTempCByIndex(0);

  float Vp = analogRead(A0)*(3.3/1023.0);
  float Ip = 0.1*((Vp-2.5)/0.66) ;
  
  Serial.println("Temperatura: "+ (String)temperatura );
  Serial.println("Corriente: "+ (String)Ip);
  Serial.println("Lectura analog: " + (String)analogRead(A0));
  while (count < 1500){
    webServerFunc();
    delay(10);
    count += 1;
  }
  if (client.connect(apiServer,80)) {
    String postStr = apiKey;
    postStr +="&field1=";
    postStr += String((float)temperatura);
    postStr +="&field2=";
    postStr += String((float)Ip);
    postStr += "\r\n\r\n";
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
    Serial.println("% send to Thingspeak");
  }
  Serial.println("Waiting…");
  client.stop();
}

void webServerFunc(void){
  
    WiFiClient webClient = server.available();;
    if (webClient.connected()) {// If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (webClient.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (webClient.available()) {             // if there's bytes to read from the client,
        char c = webClient.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            webClient.println("HTTP/1.1 200 OK");
            webClient.println("Content-type:text/html");
            webClient.println("Connection: close");
            webClient.println();
            
            // Display the HTML web page
            webClient.println("<!DOCTYPE html><html>");
            webClient.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, height=device-heigth, viewport-fit=cover\" >");//<meta name="viewport" content="width=device-width, initial-scale=1.0, height=device-heigth, viewport-fit=cover">
            webClient.println("<meta http-equiv=\"refresh\" content=\"20\">");
            webClient.println("<meta name=\"apple-mobile-web-app-capable\" content=\"yes\">");
            webClient.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            webClient.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            webClient.println("body{background-color:  #001a33;}");
            webClient.println("h1{color:  #ffffff;}p{color:  #ffffff;}");
            webClient.println(".button {border-radius: 50px 50px 50px 50px; background-color: #195B6A; border-style: solid; color: white; padding: 64px 130px;");
            webClient.println("border-style: solid;  border-color: #195B6A;");
            webClient.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            webClient.println(".button2 {background-color: #001a33;padding: 64px 115px;}</style></head>");
            
            // Web Page Heading
            webClient.println("<body><h1>Control manual del port&oacuten</h1>");
            webClient.println("</body></html>");
            webClient.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    
    Serial.println("Client disconnected.");
    Serial.println("");
  } 
   webClient.stop();
}
