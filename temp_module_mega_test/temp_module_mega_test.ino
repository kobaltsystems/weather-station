#include <dht11.h>
#include <PubSubClient.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <Losant.h>

// Losant credentials.
const char* LOSANT_DEVICE_ID = "5a83ad48ba2caf00072fb396";
const char* LOSANT_ACCESS_KEY = "b1754235-c104-468d-9518-8bc3ea637747";
const char* LOSANT_ACCESS_SECRET = "c6725a90de6b0cc65c46836cdb44c60c456820b333d8f6256c58a298d8a4aa0a";

// Create an instance of a Losant device.
LosantDevice device(LOSANT_DEVICE_ID);

EthernetClient EthClient;

// Temperature sensor settings
dht11 DHT11;
#define DHT11PIN 2
const int WAIT_TIME = 5 * 1000;
int status;
int failedConnectionAttempCounter;

void setup()
{
Serial.begin(9600);
Serial.println("DHT11 Temperature Sensor Program");
Serial.print("DHT11 library version: ");
Serial.println(DHT11LIB_VERSION);
Serial.println();
connectToInternet();
}


void connectToInternet(){
//if (client.connected())
//{
//client.stop();
//}
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
   // Read the analog input. Typically this will also involve some math
    // to convert the voltage to a temperature, which is specific to the
    // temperature sensor being used.
 
    // Build a JSON object with the state to report.
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& state = jsonBuffer.createObject();
    state["temperature"] = temperature;
    state["humidity"] = humidity;
    // Report the state to Losant.
    device.sendState(state);
}



