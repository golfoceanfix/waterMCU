#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <SoftwareSerial.h>

//#define button D1
//#define pressed LOW

const char* ssid = "Sitthiphong";
const char* pwd = "12344321";
const char* mqtt_server = "broker.mqttdashboard.com";
char* Topic = "water1";
char* recvTopic = "callbackmqtt";
char* sendMessage = "SwitchOn";

WiFiClient espClient;
PubSubClient client(espClient);

byte statusLed    = 13;
byte sensorInterrupt = 0;  // 0 = digital pin 2 
byte sensorPin       = 2;

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 4.5;
volatile byte pulseCount;  
float flowRate;

unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
unsigned long oldTime;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid,pwd);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while(!client.connected()) {
      Serial.print("Attempting MQTT connection...");
      if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server,1883);
  
   if (!client.connected()) {
    reconnect();
  }
 pinMode(D2, INPUT); 
 pinMode(D3, OUTPUT); 
  //NodeSerial.begin(57600);
  Serial.println();
  Serial.println();
  Serial.println("NodeMCU/ESP8266 Run");
  // Initialize a serial connection for reporting values to the host
  Serial.begin(38400);
   
  // Set up the status LED line as an output
  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, HIGH);  // We have an active-low LED attached
  
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);

  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;

 
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
   if((millis() - oldTime) > 1000)    // Only process counters once per second
  { 
  
    detachInterrupt(sensorInterrupt);
  
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
  
    oldTime = millis();
   
    flowMilliLitres = (flowRate / 60) * 1000;
    
    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
      
    unsigned int frac;
    
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print(".");             // Print the decimal point
    // Determine the fractional part. The 10 multiplier gives us 1 decimal place.
    frac = (flowRate - int(flowRate)) * 10;
    Serial.print(frac, DEC) ;      // Print the fractional part of the variable
    Serial.print("L/min");
    // Print the number of litres flowed in this second
    Serial.print("  Current Liquid Flowing: ");             // Output separator
    Serial.print(flowMilliLitres);
    Serial.print("mL/Sec");

    // Print the cumulative total of litres flowed since starting
    Serial.print("  Output Liquid Quantity: ");             // Output separator
    Serial.print(totalMilliLitres);
    Serial.println("mL"); 

    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;
    
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }
  Json();
}
void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}  

void Json(){
  if(totalMilliLitres =! ""){
 StaticJsonBuffer<200> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();
root["RootID"] =  totalMilliLitres;
root["MAC"] = WiFi.macAddress();  
  char JSONmessageBuffer[100];
  root.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  Serial.println("Sending message to MQTT topic..");
  Serial.println(JSONmessageBuffer);

      if (client.publish(Topic,JSONmessageBuffer) == true) {
    Serial.println("Success sending message");
    delay(1000);
    } else {
    Serial.println("Error sending message");
  }  
 }
}
