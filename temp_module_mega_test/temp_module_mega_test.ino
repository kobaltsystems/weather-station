#include <dht11.h>
#include <PubSubClient.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <Losant.h>
#include <SPI.h> //SD Bus
#include <SD.h> //SD Card

// Losant credentials.
const char* LOSANT_DEVICE_ID = "5a9374d64d3280000637e563";
const char* LOSANT_ACCESS_KEY = "a9e6aaa8-2474-4cc6-a357-d0f46d5788b1";
const char* LOSANT_ACCESS_SECRET = "5e1811fd8dfa44e3102ccab835de12142392852034161828a125b0bb76b92e8f";

// Create an instance of a Losant device.
LosantDevice device(LOSANT_DEVICE_ID);

// create an SD card instance
File myFile;

// create an ethernet instance
EthernetClient EthClient;

// Temperature sensor settings
dht11 DHT11;
#define DHT11PIN 2
const int WAIT_TIME = 5 * 1000;
int status;
int failedConnectionAttempCounter;


void setup()
{

// setup serial and DHT11
Serial.begin(9600);
Serial.println("DHT11 Temperature Sensor Program");
Serial.print("DHT11 library version: ");
Serial.println(DHT11LIB_VERSION);
Serial.println();
delay(500);

  // SD Card Initialisation
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
delay(500);

  // Register the command handler to be called when a command is received from the Losant platform.
  device.onCommand(&handleCommand);

delay(500);

//start ethernet and grab IP from network
connectToInternet();

}

void connectToInternet(){
if (EthClient.connected())
{
EthClient.stop();
}
Serial.println("Connecting to the internet via ethernet...");
// the media access control (ethernet hardware) address for the shield
// Leave this as is if your MAC address is not labelled on your ethernet shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

if (Ethernet.begin(mac) == 0) {
  Serial.println("Failed to configure Ethernet using DHCP");

}
  Serial.println(Ethernet.localIP());
// Connect the device instance to Losant using TLS encryption.
  device.connect(EthClient, LOSANT_ACCESS_KEY, LOSANT_ACCESS_SECRET);

  while(!device.connected()) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected!");

}

void loop()
{

bool toReconnect = false;


  if(!device.connected()) {
    Serial.println("Disconnected from Losant");
    toReconnect = true;
  }

  if(toReconnect) {
    connectToInternet();
  }

  device.loop();  

  //DHT11 diagnostics
Serial.println("\n");
int dht11ReadingStatus = DHT11.read(DHT11PIN);
Serial.print("Reading sensor...");
switch (dht11ReadingStatus)
{
case DHTLIB_OK:
Serial.println("Success!");
break;
case DHTLIB_ERROR_CHECKSUM:
Serial.println("Checksum error");
break;
case DHTLIB_ERROR_TIMEOUT:
Serial.println("Timeout error");
break;
default:
Serial.println("Unknown error");
break;
}

// call voids 
//ReportToLCD(DHT11.temperature, DHT11.humidity);
ReportToSerialOut(DHT11.temperature, DHT11.humidity);
ReportToLosant(DHT11.temperature, DHT11.humidity);

delay(WAIT_TIME);
}

// send temp data to serial
void ReportToSerialOut(int temperature, int humidity)
{
    Serial.print("Temperature (C): ");
    Serial.println((float)temperature, 2);
    Serial.print("Humidity (%): ");
    Serial.println((float)humidity, 2);

}

void ReportToLosant(int temperature, int humidity)
{
    // Build a JSON object with the state to report.
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& state = jsonBuffer.createObject();
    state["temperature"] = temperature;
    state["humidity"] = humidity;
    // Report the state to Losant.
    device.sendState(state);
    }


// Called whenever the device receives a command from the Losant platform.
void handleCommand(LosantCommand *command) {
  
  JsonObject& payload = *command->payload;
    String timeStamp = payload["timeStamp"]; 
    float temperature = payload["temperature"];
    float humidity = payload["humidity"];
  
  payload.printTo(Serial); // print the entire payload
  
  Serial.print(" Command received: ");
  Serial.println(command->name);
  //writes JSON values to SD
  if(strcmp(command->name, "doStuff") == 0) {
       myFile = SD.open("test.txt", FILE_WRITE);
  if (myFile) {    
    myFile.print(timeStamp);
    myFile.print(",");    
    myFile.print(temperature);
    myFile.print(",");    
    myFile.println(humidity);
    myFile.close(); // close the file
  }
  // if the file didn't open, print an error:
  else {
    Serial.println("error opening test.txt");
  }
}
  
}

