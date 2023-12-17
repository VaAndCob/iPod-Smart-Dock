/* iPod Smart Dock by Va&Cob
Features:
- Control old ipod classic gen3-5 and display status over web page http://ipodsmartdock.local
- 12 Volt charger.
Parts:
* ESP-01;
* iPod /iphone 30-pin
* 30 pin apple male dock connector
* Switching power supply 12vdc to 5vdc
* AMS1117 low-dropout linear voltage regulator
* Adaptor 220vac to 12vdc

Web Client <-> ESP-01 <-> iPod

* https://www.youtube.com/channel/UCLcOnyKh3VDaC52iIOKcVdg
* https://www.facebook.com/VaAndCob
*/

//#define debug_mem
//#define debug_ipod_resp_serial

#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncWebServer.h>
#include <Hash.h>
#include <LittleFS.h>

//PIN Define
#define LED 2 //led on pin GIP02
#define setupPIN 0 //config setupPIN on pind GIPO0

//global variable
#define version 0.9 //sketch version
bool setupmode = false;//config mode flag
uint8_t ipod_incoming[64] = {};//ipod raw message buffer
String ipod_debug = "";//debug message send to view on web client
bool header1 = false;//check 0xFF
bool header2 = false;//check 0x55
uint8_t pointer = 0;//ipod incoming message byte pointer
uint8_t total_byte_length = 0;//no of byte of ipod incomfing message
long lasttime;//for debugging

const char* host = "ipodsmartdock";//DNS hostname
WebSocketsServer webSocket = WebSocketsServer(81);
AsyncWebServer server(80);

//Include Library
#include "ipod_control.h" //ipod control code
#include "player_web_client.h" //web client
#include "wifimanager.h" //wifi configuration

/*----------------------------------------*/
//websocket event handler
void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {

  switch (type) {
    case WStype_ERROR: {
         Serial.printf("\n[%u] WebSocket Error!\n", num);

      }  break;
    case WStype_DISCONNECTED: {
         Serial.printf("\n[%u] Disconnected!\n", num);

      }  break;

    case WStype_CONNECTED://web socket connected
      {
        //IPAddress ip = webSocket.remoteIP(num);  
        Serial.printf("\n[%u] Connected!\n", num); 
        String splashText = "iPod Smart Dock is ready!\n\n    📵 iPod is not connected ❗\nTo connect to an iPod.\nPlug in 30 pin dock connector\nthen click 'CONNECT' button.\n\n╔═══╗       iPod       ╔═══╗\n║WEB║ --->  Smart ---> ║███║\n║APP║ <---  Dock  <--- ║(O)║♫ ♪ ♫ ♪\n╚═══╝       ====       ╚═══╝\n▄ █ ▄ █ ▄ ▄ █ ▄ █ ▄ █\n\nCreated by Va&Cob";  //send init screen after coonect
        webSocket.sendTXT(num,splashText);//send splash screen to client
      } break;

    case WStype_TEXT: { //Message Send from Web Client
      // Serial.printf("%s\n", payload);
      // webSocket.sendTXT(num, "ipod_incoming here");//send text on specific client
      // webSocket.broadcastTXT("ipod_incoming here");  //send text to all connected clients
         String response = byteArrayToString(payload, length);
         EXEC_CLIENT_COMMAND(response);  //read incoming command from Web CLient
    
    } break;

    case WStype_BIN: { //incoming byte from client (for manually send byte array command 0xff,0x55,...)
      // webSocket.sendBIN(num, payload, length);
      Serial.printf("\nCLIENT[%u] ", num);//client number
      for (byte i=0;i<length;i++) {
        Serial.print(payload[i],HEX);
        Serial.print("|");
        }
      Serial.printf(" [%d]", length);//byte array length      
   
      if ((payload[0] == 0xff) && (payload[1] == 0x55)) {//check if ipod command 
         Serial.println("-[iPod Command]");//correct ipod command
         SEND_COMMAND(payload);//send command to an iPod
      } else {
         Serial.println("-[Bad Command]");//bad command
      }
    } break;
  }//switch
}//websocket event


/*############################################*/
void setup() {
  Serial.begin(19200);//iPod baudrate 19200
  Serial.println(F("\r\niPod Smart Dock"));
//stop ipod sending elasped time. this make esp-01 crashe
  SEND_COMMAND(POLL_STOP);//stop ipod send track time message that could lead esp-01 to crash
  SEND_COMMAND(MODE_SIMPLE_CONTROL);//force connected ipod to exit AiR mode
//pin mode
  pinMode(LED, OUTPUT);     //status led output
  pinMode(setupPIN, INPUT_PULLUP);//config setupPIN input
  digitalWrite(LED, HIGH);  //turn OFF LED

//read saved wifi ssid /password from littleFS
  if (!LittleFS.begin()) {
    Serial.println(F("\nLittleFS mount failed\nFormatting LittleFS filesystem"));
    LittleFS.format();
  }
  File file = LittleFS.open("/wifi_credentials.txt", "r");
  if (!file) {
    Serial.println("\nFailed to open file for reading");
  } else {
  uint8_t i = 0;
  while (file.available()) {//read file
    String line = file.readStringUntil('\n');//ssid,pwd\n
    int8_t index = line.indexOf(',');//separate by comma
    if (index != -1) {
      ssid[i] = line.substring(0,index);
      password[i] = line.substring(index+1,line.length()-1);//excluded \n
      i++;
    }
  }//while
  file.close();
  }//if file
  //wait 5 sec to enter setup mode by push SETUP button
  Serial.print(F("\r\nPress SETUP buton to enter setup mode -> "));
  for (byte i=5; i >0; i=i-1) {//count down blinking & wait 5 sec to switch to setup mode
  if (digitalRead(setupPIN) == LOW)  {setupmode = true;break;}
    delay(250);digitalWrite(LED,HIGH);
    delay(250);digitalWrite(LED,LOW);//on
    delay(250);digitalWrite(LED,HIGH);
    delay(250);digitalWrite(LED,LOW);//on
    Serial.print(String(i)+". ");
  } 
  
//mode selection  
  if (setupmode) { //start ACcess point to setup ssid and password
    WiFiSetup();
  } else { //normal mode 
    if (WiFiConnect()) {;//connect to wifi
      //Start MDNS, WebSocket, Server  
      webSocket.begin();//start websocket
      webSocket.onEvent(webSocketEvent);

      //handle server http get
      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", ipod_player_page);
      });
      //handle server not found
      server.onNotFound([](AsyncWebServerRequest *request){
        request->send_P(404, "text/html", not_found_page);
      }); 
      server.begin();//start server

  //use mdns for host name resolution*/
      if (!MDNS.begin(host))  // http://ipodsmartdock.local
        Serial.println(F("Error setting up MDNS responder!"));   
      else
        Serial.printf("Hostname = http://%s.local\n", host);
      MDNS.addService("http", "tcp", 80);  // Add service to MDNS-SD
     // MDNS.addService("ws", "tcp", 81);//I think no need
    } else {//Unable to connect to WiFi then go to wifi setup mode (maybe wrong ssid & password)
      WiFiSetup();//go configuration mode  
    }
  }//if setupmode
}//setup
//############################################
void loop() {

  MDNS.update();    //handle mdns
  webSocket.loop(); //handle web socket

//------------------------------------------
//Get ipod music database 
//ipod_music_db_flag = TRUE, ipod_music_db_step = 0 , flag will be set to TRUE by ipod response read function.
  if (ipod_music_db_flag) {//ipod init
    switch(ipod_music_db_step) {
       //ipod_music_db_flag = 0 to query song database
      case 0://QUERY_IPOD_INFO_STEP_NO
        SEND_COMMAND(REQ_IPOD_NAME);
        break; 
      case 1:
        SEND_COMMAND(REQ_IPOD_FWVER);
        break;
      case 2:
        SEND_COMMAND(REQ_IPOD_SERIAL);
        break; 
      case 3:
        SEND_COMMAND(REQ_IPOD_MODEL);
        break;        
      case 4:
        TYPE = 0x01;//get playlist count
        REQ_COUNT_TYPE[6] = TYPE;//playlist
        SEND_COMMAND(REQ_COUNT_TYPE);
        break;
      case 5:
        TYPE = 0x02;//get artist count
        REQ_COUNT_TYPE[6] = TYPE;
        SEND_COMMAND(REQ_COUNT_TYPE);
        break;
      case 6:
        TYPE = 0x03;//get album count
        REQ_COUNT_TYPE[6] = TYPE;
        SEND_COMMAND(REQ_COUNT_TYPE);
        break;
      case 7:
        TYPE = 0x05;//get track count
        REQ_COUNT_TYPE[6] = TYPE;
        SEND_COMMAND(REQ_COUNT_TYPE);
        break;//get shuffle mode
      case 8:
        SEND_COMMAND(REQ_SHUFFLE_MODE);//get shuffle mode
        break;//get shuffle mode
      case 9:
        SEND_COMMAND(REQ_REPEAT_MODE);//get repeat mode       
        break;//get repeat mode
      case 10:
        TYPE = 0x01;
        listByType(TYPE,0,total_playlist);//query all playlist
        break;    
      case 11://switch to playlist 0 track 0
        TYPE = 0x01;//switch playlist
        SWITCH_TYPE_NO[6] = TYPE;
        for (byte i=0;i<4;i++) SWITCH_TYPE_NO[i+7] = 0;
        SEND_COMMAND(SWITCH_TYPE_NO);//0x00,0x17+ type(1) + number(4)   
        for (byte i=0;i<4;i++) EXCUTE_PL_JUMP_SONGNO[i+6] = 0xFF;//switch to selected playlist with song number 1 0xff,0xff,0xff,0xff
        SEND_COMMAND(EXCUTE_PL_JUMP_SONGNO);//0x00,0x28 +0xff,0xff,0xff,0xff jumpt to first track of playhlist  
        SEND_COMMAND(CMD_STOP_PLAY);//force stop playing (if play)
        break;
 
//---------------------------
     //ipod_music_db_step = 8 to query track information
      case 16://QUERY_TRACK_INFO_STEP_NO 
        SEND_COMMAND(REQ_CUR_POS);//get song cur position
        break;//switch to home
      case 17://get cur pose album title
        REQ_ALBUM_SONG_NO[6] = ((cur_song_pos) >> 24) & 0xFF;
        REQ_ALBUM_SONG_NO[7] = ((cur_song_pos) >> 16) & 0xFF;
        REQ_ALBUM_SONG_NO[8] = ((cur_song_pos) >> 8) & 0xFF;
        REQ_ALBUM_SONG_NO[9] = (cur_song_pos) & 0xFF;
        SEND_COMMAND(REQ_ALBUM_SONG_NO);//get cur pose album title
        break;//get repeat mode  
      case 18://get cur pose artist name
        REQ_ARTIST_SONG_NO[6] = ((cur_song_pos) >> 24) & 0xFF;
        REQ_ARTIST_SONG_NO[7] = ((cur_song_pos) >> 16) & 0xFF;
        REQ_ARTIST_SONG_NO[8] = ((cur_song_pos) >> 8) & 0xFF;
        REQ_ARTIST_SONG_NO[9] = (cur_song_pos) & 0xFF;
        SEND_COMMAND(REQ_ARTIST_SONG_NO);
        break;
      case 19://get cur pos song title
        REQ_TITLE_SONG_NO[6] = ((cur_song_pos) >> 24) & 0xFF;
        REQ_TITLE_SONG_NO[7] = ((cur_song_pos) >> 16) & 0xFF;
        REQ_TITLE_SONG_NO[8] = ((cur_song_pos) >> 8) & 0xFF;
        REQ_TITLE_SONG_NO[9] = (cur_song_pos) & 0xFF;
        SEND_COMMAND(REQ_TITLE_SONG_NO);
        break;
      case 20://get track length/elapsed time/ status 
        SEND_COMMAND(REQ_TIME_STATUS);    
        break;
      case 21:
        update_ipod_screen();//update ipod screen after change track
        break;

    }//switch
    
    ipod_music_db_flag = false;//set flag to no query
    ipod_music_db_step++; //add next step query
    if (ipod_music_db_step > 25) ipod_music_db_step = 25;//limit maximum

  }//if
//------------------------------------------
//Polling ipod message by checking header and message legnth
  while (Serial.available()) {
    byte incomingByte = Serial.read();//read as byte
    
    if ((incomingByte == 0xff) && !header1) {//1st byte
      header1 = true;
      ipod_incoming[0] = incomingByte;
      String hexString = String(incomingByte, HEX) + "|";//convert byte to hex string
      ipod_debug.concat(hexString);
    } 

    if ((incomingByte == 0x55) && header1 && !header2) {//2nd byte
      header2 = true;
      ipod_incoming[1] = incomingByte;
      String hexString = String(incomingByte, HEX) + "|";//convert byte to hex string
      ipod_debug.concat(hexString);
      pointer = 0;
    } else if (header1 && header2) {//correct heder

      if (pointer == 0) {//length byte (3rd byte)
          total_byte_length = incomingByte + 2;//= all bytes excluding 2 byte header(length + mode+ para+ crc)
      }
    
      ipod_incoming[pointer+2] = incomingByte;//incoming[2.....]
      String hexString = String(incomingByte, HEX)+"|";//convert byte to hex string
      ipod_debug.concat(hexString);
      pointer++;//12345
 
      if (pointer == total_byte_length) {//end of message
        if (debug) {//if debug mode
          String brdcast =  "#"+ipod_debug;//add debug header
          webSocket.broadcastTXT(brdcast);//send ipd response message to web client debug screen (right)
        }  
        READ_IPOD_RESPONSE(ipod_incoming);//interpret ipod message
        //reset variable
        header1 = false;
        header2 = false;
        total_byte_length = 0;
        ipod_debug = "";

        break;
      }//if pointer
    }//else if header1 header2
  }//while serial
//------------------------------------------
 //timer to 10 sec to return to playing screen
  if (((millis()-lasttime) > 10000) && !pollStatus) {
    SEND_COMMAND(POLL_START);
    pollStatus = true;
  }
  
//check memory for debugging purpose only
#ifdef debug_mem
  if ((millis()-lasttime) > 1000) {
    Serial.printf("\nFree Mem Heap = %d|Stack = %d|",ESP.getFreeHeap(),ESP.getFreeContStack());
  lasttime = millis();
  }
#endif

}//loop
//############################################
