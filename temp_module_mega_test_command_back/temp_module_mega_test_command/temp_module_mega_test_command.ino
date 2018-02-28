#include <PubSubClient.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <Losant.h>

// Losant credentials.
const char* LOSANT_DEVICE_ID = "5a9374d64d3280000637e563";
const char* LOSANT_ACCESS_KEY = "a9e6aaa8-2474-4cc6-a357-d0f46d5788b1";
const char* LOSANT_ACCESS_SECRET = "5e1811fd8dfa44e3102ccab835de12142392852034161828a125b0bb76b92e8f";


// Create an instance of a Losant device.
LosantDevice device(LOSANT_DEVICE_ID);

// create an ethernet instance
EthernetClient EthClient;

const int BUTTON_PIN = 7;
const int LED_PIN = 8;

bool ledState = false;

// Toggles and LED on or off.
void doStuff() {
  Serial.println("Doing stuff like turning on an LED.");
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
}

// Called whenever the device receives a command from the Losant platform.
void handleCommand(LosantCommand *command) {
  
  // { "foo" : 10 }
  JsonObject& payload = *command->payload;
    String stringOne = payload["timeStamp"]; 

  payload.printTo(Serial); // print the entire payload
  
  Serial.print(" Command received: ");
  Serial.println(command->name);
  Serial.print("Command payload: ");
  Serial.println(stringOne);
  
  
  if(strcmp(command->name, "doStuff") == 0) {
    doStuff();
  }
}

void setup()
{
Serial.begin(9600);
  delay(100);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  // Register the command handler to be called when a command is received
  // from the Losant platform.
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

void buttonPressed() {
  Serial.println("Button Pressed!");

  // Losant uses a JSON protocol. Construct the simple state object.
  // { "button" : true }
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["button"] = true;

  // Send the state to Losant.
  device.sendState(root);
}

int buttonState = 0;

void loop() {

  bool toReconnect = false;


  if(!device.connected()) {
    Serial.println("Disconnected from Losant");
    toReconnect = true;
  }

  if(toReconnect) {
    connectToInternet();
  }

  device.loop();

  int currentRead = digitalRead(BUTTON_PIN);

  if(currentRead != buttonState) {
    buttonState = currentRead;
    if(buttonState) {
      buttonPressed();
    }
  }

  delay(100);
}
