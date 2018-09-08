/*
* SmartDeskControl v1.1
*
* Written by Dimitri Tzika
*
* Control 4 relais via a simple html page or via 4 buttons
*
*/

#include <EtherCard.h>

// Network Settings
static byte myMac[] = { 0x00,0x01,0x01,0x01,0x01,0x01 };
static byte myIp[] = { 192,168,123,200 };

# define bufferSize 1200
byte Ethernet::buffer[bufferSize];
BufferFiller bfill;

#define csPin   10

// Relais, Buttons, LightSensor Settings
#define relay1 2
#define relay2 3
#define relay3 4
#define relay4 5

#define button1 6
#define button2 7
#define button3 8
#define button4 9

#define lightSensor A0

// Relais (Disabled), Buttons, LightSensor Status
bool relayStatus1 = true;
bool relayStatus2 = true;
bool relayStatus3 = true;
bool relayStatus4 = true;

int buttonStatus1 = 0;
int buttonStatus2 = 0;
int buttonStatus3 = 0;
int buttonStatus4 = 0;

bool buttonCalmDown1 = false;
bool buttonCalmDown2 = false;
bool buttonCalmDown3 = false;
bool buttonCalmDown4 = false;

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

void interfacePage() {

  bfill.emit_p(PSTR("$F"
    "<head>"
      "<meta http-equiv='refresh'content='5'>"
      "<title>Smart Desk Control</title>"
      "<style>"
        "a{text-decoration:none;display:inline-block;font-size:30px;font-weight:bold;}"
        "hr{clear:both;visibility:hidden;}"
        ".header{background-color:#C63D0F;width:100%;padding:25px 0px;text-align:center;color:#000000;}"
        ".btn-group{position:relative;left:50%;margin:0px -290px;}"
        ".btn-group .button{background-color: #C63D0F;border:1px solid #A61D00;padding:15px 32px;font-weight:bold;font-size:20px;}"
        ".btn-group .button:hover{background-color:#E65D2F;}"
      "</style>"
    "</head>"
    "<body bgcolor='#2B2728'>"
    "<a href='' class='header'>Smart Desk Control</a>"
    "<hr>"
    "<div class='btn-group'>"
      "<a href='?comRelay1=$F'><button class='button'>$F</button></a>"
      "<a href='?comRelay2=$F'><button class='button'>$F</button></a>"
      "<a href='?comRelay3=$F'><button class='button'>$F</button></a>"
      "<a href='?comRelay4=$F'><button class='button'>$F</button></a>"
    "</div>"),
  http_OK,
  relayStatus1?PSTR("off"):PSTR("on"),
  relayStatus1?PSTR("<font color=\"black\"><b>Speakers</b></font>"):PSTR("<font color=\"yellow\">Speakers</font>"),
  relayStatus2?PSTR("off"):PSTR("on"),
  relayStatus2?PSTR("<font color=\"black\"><b>PC-MONITOR</b></font>"):PSTR("<font color=\"yellow\">PC-MONITOR</font>"),
  relayStatus3?PSTR("off"):PSTR("on"),
  relayStatus3?PSTR("<font color=\"black\"><b>LIGHT</b></font>"):PSTR("<font color=\"yellow\">LIGHT</font>"),
  relayStatus4?PSTR("off"):PSTR("on"),
  relayStatus4?PSTR("<font color=\"black\"><b>LED</b></font>"):PSTR("<font color=\"yellow\">LED</font>"));

}

void interfacePageActions() {

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
      relayStatus1 = true;
      bfill.emit_p(http_Found);
    }

    else if (strncmp("?comRelay2=on ", data, 14) == 0) {
      relayStatus2 = true;
      bfill.emit_p(http_Found);
    }

    else if (strncmp("?comRelay3=on ", data, 14) == 0) {
      relayStatus3 = true;
      bfill.emit_p(http_Found);
    }

    else if (strncmp("?comRelay4=on ", data, 14) == 0) {
      relayStatus4 = true;
      bfill.emit_p(http_Found);
    }

    else if (strncmp("?comRelay1=off ", data, 15) == 0) {
      relayStatus1 = false;
      bfill.emit_p(http_Found);
    }

    else if (strncmp("?comRelay2=off ", data, 15) == 0) {
      relayStatus2 = false;
      bfill.emit_p(http_Found);
    }

    else if (strncmp("?comRelay3=off ", data, 15) == 0) {
      relayStatus3 = false;
      bfill.emit_p(http_Found);
    }

    else if (strncmp("?comRelay4=off ", data, 15) == 0) {
      // Check For Bright Light
      // if (lightSensorStatus < 1)
        relayStatus4 = false;
      bfill.emit_p(http_Found);
    }

    else
      bfill.emit_p(http_Unauthorized); // Error

  }

    // Send HTTP Response
    ether.httpServerReply(bfill.position());

}

void buttonActions() {
  if (buttonStatus1 && !buttonCalmDown1){
    buttonCalmDown1 = true;
    if (relayStatus2)
      relayStatus2 = false;
    else
      relayStatus2 = true;
  }
  else if (!buttonStatus1)
    buttonCalmDown1 = false;

  if (buttonStatus2 && !buttonCalmDown2){
    buttonCalmDown2 = true;
    if (relayStatus3)
      relayStatus3 = false;
    else
      relayStatus3 = true;
  }
  else if (!buttonStatus2)
    buttonCalmDown2 = false;

  if (buttonStatus3 && !buttonCalmDown3){
    buttonCalmDown3 = true;
    if (relayStatus4)
      relayStatus4 = false;
    else
      relayStatus4 = true;
  }
  else if (!buttonStatus3)
    buttonCalmDown3 = false;

  if (buttonStatus4 && !buttonCalmDown4){
    buttonCalmDown4 = true;
    if (relayStatus1)
      relayStatus1 = false;
    else
      relayStatus1 = true;
  }
  else if (!buttonStatus4)
    buttonCalmDown4 = false;

}

void setup() {

  // Show Status
  Serial.begin(115200);
  Serial.println("[SmartDeskControl]");

  // Setup Network
  ether.begin(bufferSize, myMac, csPin);
  ether.staticSetup(myIp);

  // Setup Relais & Buttons
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);

  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(button3, INPUT);
  pinMode(button4, INPUT);

}

void loop() {

  // Update Relais
  digitalWrite(relay1, relayStatus1);
  digitalWrite(relay2, relayStatus2);
  digitalWrite(relay3, relayStatus3);
  digitalWrite(relay4, relayStatus4);
  
  // Take Buttons & LightSensor Inputs
  buttonStatus1 = digitalRead(button1);
  buttonStatus2 = digitalRead(button2);
  buttonStatus3 = digitalRead(button3);
  buttonStatus4 = digitalRead(button4);

  lightSensorStatus = analogRead(lightSensor);

  // If Bright Light Turn OFF Relay
  // relayStatus4 = (relayStatus4 || lightSensorStatus > 10);

  //  Check for Ethernet Packet And Then Check For TCP Packet
  tcp = ether.packetLoop(ether.packetReceive());

  // If TCP Packet Exists Then Response
  if (tcp)
    interfacePageActions();

  // Check Buttons If Pressed Then Toggle
  buttonActions();

}
