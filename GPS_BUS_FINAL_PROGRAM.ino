
#include <TinyGPS++.h>                                  // Tiny GPS Plus Library
#include <SoftwareSerial.h>                             // Software Serial Library so we can use other Pins for communication with the GPS module
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

static const int RXPin = 12, TXPin = 13;                // Ublox 6m GPS module to pins 12 and 13 (RX to D7, TX to D6)
static const uint32_t GPSBaud = 9600;    
// Ublox GPS default Baud Rate is 9600

const char* ssid2 = "eternallousy";
const char* password2 = "taolatrum";

const double classLat = 37.3461703;
const double classLng = -121.9346195;
TinyGPSPlus gps;                                        // Create an Instance of the TinyGPS++ object called gps
SoftwareSerial ss(RXPin, TXPin);                        // The serial connection to the GPS device
int numberOfPeople = 0;
boolean flag = false;
boolean buttonFlag = false;
int directionNo = 0;
long lastUpdateTime = millis();
void setup()
{  
  //COnnecting to Wifi
  
  
  Serial.begin(115200);
  pinMode( 5, INPUT); //D1
  pinMode( 4, INPUT); //D2
  pinMode( 2, INPUT); //D4
  
   //COnnecting to Wifi
 
  delay(10);

  // We start by connecting to a WiFi network
  
  Serial.println();
  Serial.println();
  
  WiFi.begin(ssid2, password2);
  
  Serial.print("Connecting to ");
  Serial.println(ssid2);
  
  WiFi.begin(ssid2, password2);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  delay(1000);
  Serial.println("Starting GPS module");
  ss.begin(GPSBaud);                                    // Set Software Serial Comm Speed to 9600    
}

void loop()
{   
  // put your main code here, to run repeatedly:
  
  int inputState1 = digitalRead(5);
  int inputState2 = digitalRead(4); 
  int buttonPress = digitalRead(2);
  if (buttonPress == HIGH && !buttonFlag) {
    buttonFlag = true;
    directionNo = (directionNo+1)%2;
  } 
  if (buttonPress == LOW && buttonFlag){
    buttonFlag = false;
  }
  if  (inputState1 == LOW && inputState2 == LOW) flag = true; 
  if (inputState1 == LOW && inputState2 == HIGH && flag) {
    numberOfPeople--;
    if (numberOfPeople <= 0) numberOfPeople =0;
    flag = false;
  }
  
  if (inputState1 == HIGH && inputState2 == LOW && flag) {
    numberOfPeople++;
    flag = false;
  }
  if (inputState1 == HIGH && inputState2 == HIGH) flag = false;
  Serial.print("people: ");
  Serial.println(numberOfPeople);
  Serial.print("direction: ");
  Serial.println(directionNo);
  Serial.println("sending");
      Serial.print("Latitude  : ");
      Serial.println(gps.location.lat());
      Serial.print("Longitude : ");
      Serial.println(gps.location.lng());                  // Seconds
      Serial.print("Speed     : ");
      Serial.println(gps.speed.mph());
  
  smartDelay(100);                                      // Run Procedure smartDelay*/
  //delay(200);
  long nowTime = millis();
  //Serial.println(nowTime);
  if ((nowTime - lastUpdateTime) >= 30000){
      Serial.println("sending");
      Serial.print("Latitude  : ");
      Serial.println(gps.location.lat());
      Serial.print("Longitude : ");
      Serial.println(gps.location.lng());                  // Seconds
      Serial.print("Speed     : ");
      Serial.println(gps.speed.mph());
      //smartDelay(200);
      double posLat, posLng,speedD;
      if (millis() > 5000 && gps.charsProcessed() < 10){
        Serial.println(F("No GPS data received: check wiring. Using hard code location"));
        posLat = classLat;
        posLng = classLng;
        speedD = 0;
      }  
      else {
        posLat = gps.location.lat();
        posLng = gps.location.lng();
        speedD = gps.speed.mph();
      }
      String jsonBody = "{\"token\":\"zUFjpZmcr42u2pVpoC83GugrY2N8LwY8\",\"bus_id\":888, \"route_id\":999,\"latitude\":\""+String(posLat,7)+"\",\"longitude\":\""+String(posLng,7)+"\",\"speed\":"+String(speedD,4)+",\"occupied_seats\":"+numberOfPeople+",\"direction\":"+ directionNo+"}"; 
      Serial.println("sending");
      HTTPClient http;
      String path = "http://shivanageshchandra.com:3000/api/buslog";
      http.begin(path);
      http.addHeader("Content-Type", "application/json");
      int httpCode = http.POST(jsonBody);
      if (httpCode > 0 ){
        Serial.println("Success");
        //String payload = http.getString();
        //Serial.println(payload);
      } else {
        Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
      lastUpdateTime = nowTime;
  }
}

static void smartDelay(unsigned long ms)                // This custom version of delay() ensures that the gps object is being "fed".
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}


