/* Nodemcu Oil Tank Gauge v3 using NewPing Library*/
// IP Address - Static 10.1.1.3
// Vera Host - 10.1.1.2

// Library includes
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <math.h>
#include <NewPing.h>

// Initialization


// I am taking 10 samples to weed out some bad values when the oil level is near the top
// as the sensor is blind when the level is 0-24cm.
// This takes a little bit longer (~1 second) to get a reading but I get less errors this way.

const char* ssid = "MY_SSID";
const char* password = "MY_WIFI_PW";
const char* veraHost = "10.1.1.2"; //this is the IP adress of our VERA
const int httpPort = 3480;
const int VeraDeviceNum = 5;	//this is the device number on the VERA
const int maxSample = 10; // Number of Samples to take to get the current level


String startUrl = "/data_request?id=variableset&DeviceNum=";
String midUrl = "&serviceId=urn:vera:serviceId:MySensor1&Variable=CurrentLevel&Value=";
String page = "";

// sensor sits up 2cm and depth is really 113 to bottom from ping sensor.  Using 102 gives a fuel reserve. 

const int tankEmptyDepth = 102; // This MUST be no greater than 450 cm (500cm for the HC-SR04)!
const int tankFullDepth = 2; // This should be at least 25 cm, if possible (2cm for the HC-SR04)

// Change the above tankEmptyDepth and tankFullDepth constants to be the distance (in centimeters):
//    - tankEmptyDepth  = between the ultrasonic sensor and the empty level (i.e. bottom of tank)
//    - tankFullDepth   = between the sensor and the liquid when the tank is full (MINIMUM 25cm)
// Note that the ultrasonic sensor works only between 25cm and 450cm, so the min tankFullDepth = 25.
// For my tank, the tankFullDepth = 15, which is okay... BUT it means that when the tank is full, 
// I will probably get incorrect readings until the level drops 10cm. This isn't a problem in my case 
// since I I don't care about accurate level readings when the fuel tank is full! But it means that 
// after getting the tank filled, the level will read near-empty or "Error: Timeout".
//
// You could also use the HC-SR04, which is larger but has a min depth of 2cm and max of 500cm.
// Note however that it's not waterproof! I chose the JSN-SR04T-2.0 for that reason.
//
// Note also that you might want to set tankEmptyDepth to be less than the bottom of your tank,
// esp if you have a vertical feed sucking liquid from, say, 5cm above the bottom of the tank.
// For example, my tank is 163cm deep from the sensor, so I set tankEmptyDepth to 153. This ensures
// that when my LevelMatic reads 0%, I should have 10cm of fuel left in the tank.
//
// If measuring in inches: 1 inch = 2.54 cm




// These vars hold the current and last-measured level from the sensor
unsigned long duration, distance;
int currentDistance = 0;
int lastLevel = 999;
int lastDistance = 0;
int errorCount = 0;
bool firstTime = 1;

long scanInterval = 300000;           // interval at which to run the scans (every 5 minutes)
//long scanInterval = 60000;        // used this for testing purposes
long previousMillis = 0;

// ultrasonic sensor GPIO pins
const int trigPin = 14; //D5 of nodemcu
const int echoPin = 12; //D6 of nodemcu
const int maxDistance=113; //Maximum Distance

ESP8266WebServer server(8888);   //instantiate server at port 8888
IPAddress ip(10, 1, 1, 3); 
IPAddress gateway(10, 1, 1, 1);
IPAddress subnet(255, 255, 255, 0);

NewPing sonar(trigPin,echoPin,maxDistance);

void setup() {

  // Set up Serial Output
  Serial.begin(115200);
  
  // Set up GPIO pins for ultrasonic sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(2, OUTPUT); // Initialize GPIO2 pin (other LED)

  digitalWrite(2, LOW); // Turn other LED on

  // Set trigger pin for sensor to LOW
  digitalWrite(trigPin, LOW);
  
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.config(ip, gateway, subnet);

  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  // Print the IP address
  Serial.print("My IP Address is :");
  Serial.print(WiFi.localIP());
  Serial.println();

  // Turn off the other LED and turn on main LED to signal WIFI connectivity

  digitalWrite(2, HIGH);
  digitalWrite(LED_BUILTIN, LOW);


  page = "<h1>Simple NodeMCU Web Server</h1><p>Hi!!!</p>";

  server.on("/", [](){
    server.send(200, "text/html", page);
  });
  server.on("/status.json", [](){
        StaticJsonBuffer<300> JSONbuffer;
        JsonObject& root = JSONbuffer.createObject();
        root["oiltank"] = lastLevel;
        root["distance"] = lastDistance;

        String jsonString;
        root.printTo(jsonString);
        Serial.print("Oil Tank Status Check - ");
        Serial.println(jsonString);
        server.send(200, "application/json", jsonString);
    });

  server.begin();
  Serial.println("Web server started!");

  previousMillis = millis();
  Serial.println("Hello. Initialization Complete..");
  
}

void VeraUpdate(int myValue) {
    // Use WiFiClient class to create TCP connections
    WiFiClient client;

    Serial.println(" Updating Vera");  
    if (!client.connect(veraHost, httpPort)) {
      Serial.println("connection failed");
      return;
    }

   // This will send the update request to the server
   client.print(String("GET ") + startUrl + VeraDeviceNum + midUrl + myValue + " HTTP/1.1\r\n" +
                "Host: " + veraHost + "\r\n" +
               "Connection: close\r\n\r\n");               
    Serial.println("closing Vera connection");
}

void showError() {
// **** Will need to flag the VERA possibly in this case
Serial.println("*** Error ***");
VeraUpdate(999);
}

void getcurrentPct() {
  WiFiClient client;
  float pctLevel;
  int OilLevel;
  // Subtracting tankFullDepth gives us a precise ratio between full/empty, as if the ultrasonic sensor
  // would be 0 cm away from the liquid level when the tank is full.
  // Also, currentDistance contains height of "emptiness" above the liquid, so to get liquid level we do:
  //   absolute value (1 - (currentDistance/tankEmptyDepth)).
  if (currentDistance < tankEmptyDepth) {
    pctLevel=fabs(((float)currentDistance-(float)tankFullDepth) / ((float)tankEmptyDepth-(float)tankFullDepth)); 
    OilLevel = round((1.00 - pctLevel) * 100.0);
  } else {
      OilLevel = 0;
  }
  if (OilLevel != lastLevel) {
       lastLevel = OilLevel;
       Serial.print("Current Distance: ");
       Serial.print(currentDistance,DEC);
       Serial.print(" ** ");
       Serial.print("Oil Tank Fuel Level: ");
       Serial.print(OilLevel, DEC);
       Serial.println("%");
       VeraUpdate(OilLevel);
  } 
}

void loop() {
  // Initialize variables
  int sampleCounter;
 
  server.handleClient();         //Handling of incoming requests

 // If this is your first time, make sure you run now
 // Or if you have exceeded the time limit for the next scan.
  if (((millis() - previousMillis) > scanInterval) || (firstTime==1))  {
    firstTime=0;
    // save the last time you ran the scan 
    previousMillis = millis();
  
//  errorCount=0;
  currentDistance=0;

 // Scan to the Max Sample Size (10) to get an accurate reading.
 // If you get an error, increase the error counter to keep track.
 // If the values differ by more than 2cm than assume the sensor is to close
 // and set the default level to minimum. I'm not worried about getting exact
 // measurements when the tank is near full so approximate at that point.

  Serial.print("Reading: ");

  duration = sonar.ping_median(10); //median default - 5 values ; finding 10 values weeds out bad readings
  currentDistance = sonar.convert_cm(duration); //convert that to cm
  
  Serial.print(currentDistance);
  Serial.println();
  lastDistance=currentDistance;
  if (currentDistance==0) {
      showError();
  } else {
      getcurrentPct();
  }        
  }
}
