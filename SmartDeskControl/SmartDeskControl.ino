/*
* SmartDeskControl v1.2
*
* Written by Dimitri Tzika
*
* Control 4 relais via a simple html page or via 4 buttons
*
*/

#include <EtherCard.h>

// Network Settings
static byte myMac[] = {0x00,0x01,0x01,0x01,0x01,0x01};
static byte myIp[] = {192,168,123,200};
# define bufferSize 1400
byte Ethernet::buffer[bufferSize];
BufferFiller bfill;
#define csPin   10

// Relais, Buttons and LightSensor pin Settings
int relays[] = {2, 3, 4, 5};
int buttons[] = {9, 6, 7, 8};
#define lightSensor A0

// Relais, Buttons, LightSensor Status
bool relaysActive[] = {true, true, true, true};
int buttonsPressed[] = {0, 0, 0, 0};
bool buttonsCalmDown[] = {false, false, false, false};
int lightSensorStatus = 0;

// For HTTP Connection
word tcp;

// HTML Page Responses
const char http_OK[] PROGMEM =
"HTTP/1.0 200 OK\r\n"
"Content-Type: text/html\r\n"
"Pragma: no-cache\r\n\r\n";

const char http_Found[] PROGMEM =
"HTTP/1.0 302 Found\r\n"
"Location: /\r\n\r\n";

const char http_Unauthorized[] PROGMEM =
"HTTP/1.0 401 Unauthorized\r\n"
"Content-Type: text/html\r\n\r\n"
"<h1>401 Unauthorized</h1>";

void interfacePage(){
  bfill.emit_p(PSTR("$F"
    "<div class='btn-group'>"
      "<a href='?comRelay1=$F'><button class='button'>$F</button></a>"
      "<a href='?comRelay2=$F'><button class='button'>$F</button></a>"
      "<a href='?comRelay3=$F'><button class='button'>$F</button></a>"
      "<a href='?comRelay4=$F'><button class='button'>$F</button></a>"
    "</div>"),
  http_OK,
  relaysActive[0]?PSTR("off"):PSTR("on"),
  relaysActive[0]?PSTR("<font color=\"black\"><b>Speakers</b></font>"):PSTR("<font color=\"yellow\">Speakers</font>"),
  relaysActive[1]?PSTR("off"):PSTR("on"),
  relaysActive[1]?PSTR("<font color=\"black\"><b>PC-MONITOR</b></font>"):PSTR("<font color=\"yellow\">PC-MONITOR</font>"),
  relaysActive[2]?PSTR("off"):PSTR("on"),
  relaysActive[2]?PSTR("<font color=\"black\"><b>LIGHT</b></font>"):PSTR("<font color=\"yellow\">LIGHT</font>"),
  relaysActive[3]?PSTR("off"):PSTR("on"),
  relaysActive[3]?PSTR("<font color=\"black\"><b>LED</b></font>"):PSTR("<font color=\"yellow\">LED</font>"));
}

void interfacePageActions(){
  bfill = ether.tcpOffset();
  char *data = (char *) Ethernet::buffer + tcp;
  if (strncmp("GET /", data, 5) != 0)
    bfill.emit_p(http_Unauthorized); // Unsupported HTTP Request

  else {

    // Connection Status
    Serial.println("\nBegin");
    Serial.println(data);
    Serial.print("END");

    data += 5;
    if (data[0] == ' ')
      interfacePage(); // Return interfacePage

    // Toggle Pressed Relay
    else if (strncmp("?comRelay1=on ", data, 14) == 0) {
      relaysActive[0] = true;
      bfill.emit_p(http_Found);
    }

    else if (strncmp("?comRelay2=on ", data, 14) == 0) {
      relaysActive[1] = true;
      bfill.emit_p(http_Found);
    }

    else if (strncmp("?comRelay3=on ", data, 14) == 0) {
      relaysActive[2] = true;
      bfill.emit_p(http_Found);
    }

    else if (strncmp("?comRelay4=on ", data, 14) == 0) {
      relaysActive[3] = true;
      bfill.emit_p(http_Found);
    }

    else if (strncmp("?comRelay1=off ", data, 15) == 0) {
      relaysActive[0] = false;
      bfill.emit_p(http_Found);
    }

    else if (strncmp("?comRelay2=off ", data, 15) == 0) {
      relaysActive[1] = false;
      bfill.emit_p(http_Found);
    }

    else if (strncmp("?comRelay3=off ", data, 15) == 0) {
      relaysActive[2] = false;
      bfill.emit_p(http_Found);
    }

    else if (strncmp("?comRelay4=off ", data, 15) == 0) {
      // Check For Bright Light
      // if (lightSensorStatus < 1)
        relaysActive[3] = false;
      bfill.emit_p(http_Found);
    }

    else
      bfill.emit_p(http_Unauthorized); // Error

  }

    // Send HTTP Response
    ether.httpServerReply(bfill.position());
}

void network(){
  //  Check for Ethernet Packet And Then Check For TCP Packet
  tcp = ether.packetLoop(ether.packetReceive());

  // If TCP Packet Exists Then Response
  if (tcp)
    interfacePageActions();
}

void buttonAction(){
  for (unsigned int i = 0; i < sizeof(relays)/sizeof(unsigned int); i++){
    if (buttonsPressed[i] && !buttonsCalmDown[i]){
      buttonsCalmDown[i] = true;
      relaysActive[i] = !relaysActive[i];
    }
    else if (!buttonsPressed[i]){
      buttonsCalmDown[i] = false;
    }
  }
}

void update(){
  // Update Relais
  digitalWrite(relays[0], relaysActive[0]);
  digitalWrite(relays[1], relaysActive[1]);
  digitalWrite(relays[2], relaysActive[2]);
  digitalWrite(relays[3], relaysActive[3]);
  
  // Take Buttons & LightSensor Inputs
  buttonsPressed[0] = digitalRead(buttons[0]);
  buttonsPressed[1] = digitalRead(buttons[1]);
  buttonsPressed[2] = digitalRead(buttons[2]);
  buttonsPressed[3] = digitalRead(buttons[3]);

  lightSensorStatus = analogRead(lightSensor);
}

void setup(){
  Serial.begin(115200);
  Serial.println("[SmartDeskControl]");

  // Setup Network
  ether.begin(bufferSize, myMac, csPin);
  ether.staticSetup(myIp);

  // Setup Relais & Buttons
  pinMode(relays[0], OUTPUT);
  pinMode(relays[1], OUTPUT);
  pinMode(relays[2], OUTPUT);
  pinMode(relays[3], OUTPUT);

  pinMode(buttons[0], INPUT);
  pinMode(buttons[1], INPUT);
  pinMode(buttons[2], INPUT);
  pinMode(buttons[3], INPUT);
}

void loop(){
  update();
  // If Bright Light Turn OFF Relay
  // relayStatus4 = (relayStatus4 || lightSensorStatus > 10);
  network();
  buttonAction();
}
