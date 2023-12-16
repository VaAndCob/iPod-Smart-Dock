/*library iPod control */

#include "ipod_command.h" //ipod  command list

//Control Variable
#define QUERY_IPOD_INFO_STEP_NO 0  //0-10 = to get music database
#define QUERY_TRACK_INFO_STEP_NO 16 //16-21 = to get current track information
uint8_t ipod_music_db_step = QUERY_IPOD_INFO_STEP_NO;//ipod music database and track query control
bool ipod_music_db_flag = false;

//General Variable
String list = "";               //track,album,playlist,artist LIST  buffer to display on screen

const String ipodscreen_header = "░░░░░░░░| iPod Smart Dock |░░░░░░░░\n";//header screen
String bar = "\n \n|                                  |\n";//bar display
String ipod_model = "-";        //ipod model
String ipod_capacity = "-";     //ipod harddisk capacity
String ipod_name = "-";         //ipod name
String ipod_firmware = "-";     //ipod firmware
String ipod_serial = "-";       //ipod serial no
String play_status = "⏹";      //play status icon
uint8_t playing = 0x00;         //status=0x00 stop, 0x01 playing, 0x02 paused
uint8_t total_playlist = 1;     //total playlist count
uint16_t total_song = 1;        //total song count
uint16_t total_artist = 1;      //total artist count
uint16_t total_album = 1;       //total album count
bool track_pos_0 = true;        //check track elapsed time position 0
uint16_t cur_song_pos = 0;      //current song position
uint16_t cur_playlist_pos = 0;  //current playlist position
uint16_t cur_artist_pos = 0;    //current artist position
uint16_t cur_album_pos = 0;     //current album position
uint16_t start_pointer = 0;     //pointer to start quering
uint16_t read_item_count = 10;   //range to read item from start pointer
String song_title = " ";        //song title
String song_artist = " ";       //artis name
String song_album = " ";        //album name
String playlist_title[24] = {};     //title of playlist keep in string array
uint32_t elapse_time_sec = 0;   //elapsed time in second
uint32_t remain_time_sec = 0;   //remaining time in second
uint32_t total_time_sec = 0;    //total track length in second
uint16_t noofsong_pl = 1;       //number of song in  current play list
uint8_t shuffle_mode = 0x00;    //shuffe mode
uint8_t repeat_mode = 0x00;     //repeat mode
String shuffle_text = " ";      //shuffle mode text
String repeat_text = " ";       //repeat mode text

String number_string = "0";     //hold number pad input
bool debug = false;             //debug mode, if true, will send ipod reponse message to web client

bool pollStatus = true;         //polling flag to toggle info screen

//--------------------------------------
//shorten string to 28 letters
String shorten28(String source) {
  if (source.length() > 28) return source.substring(0,27) + "...";
  else return source;
}  
//--------------------------------------
//convert seconds to HH:MM:SS
struct StringAndByte {
  String text;
  uint8_t length;
};

StringAndByte elapsedTime,remainTime;

StringAndByte mill2HMS(int totalSeconds) {
  StringAndByte result;
  String hours = String(totalSeconds / 3600);
  String minutes = String((totalSeconds % 3600) / 60);
  String seconds = String(totalSeconds % 60);
  if (hours == "0") hours = ""; else hours.concat(":");
  if (minutes.length()==1) minutes = "0" + minutes;
  if (seconds.length()==1) seconds = "0" + seconds;
  result.text = hours+minutes+":"+seconds;
  result.length = result.text.length();
  return result;
}
//--------------------------------------
//convert integer to byte array 
void integerToByteArray(int value, byte* byteArray) {
  byteArray[0] = (value >> 24) & 0xFF;
  byteArray[1] = (value >> 16) & 0xFF;
  byteArray[2] = (value >> 8) & 0xFF;
  byteArray[3] = value & 0xFF;
}
//--------------------------------------
//convert byte array to integer number
//byte array = [1,2,3,4] -> integer number
int byteArrayToInteger(byte* byteArray,byte length) {
  int result = 0;
  for (int i = 0; i <length; i++) {
    result |= (byteArray[length-1-i] << (8 * i));
  }
  return result;
}
//--------------------------------------
String byteArrayToString(byte *byteArray, size_t length) {
  String result = "";
  for (size_t i = 0; i < length; i++) {
    result += char(byteArray[i]);
  }
  return result;
}
//--------------------------------------
//calcualte CRC = ( (0x100 - (length+mode+command)) & 0xFF)
uint8_t CRC(uint8_t arrays[]) {
  uint16_t sum = 0;
  uint8_t length = arrays[2];
  //Serial.println(length);
  for (uint8_t i=2;i<2+length+1;i++) {
    sum = sum + arrays[i];
    //Serial.print(arrays[i],HEX);   
  }
  uint16_t crc = (0x100 - sum) & 0xFF;
  //Serial.print("CRC = ");
  //Serial.println(crc,HEX);
  return crc;
}
//--------------------------------------
//Send command to control ipod (without parameter)
void SEND_COMMAND(uint8_t command[]) {
  uint8_t length = command[2];//get byte length
  uint8_t crc = CRC(command);
//send to serial
  for (uint8_t i=0;i<length+3;i++) {
    Serial.write(command[i]);
  }
  Serial.write(crc);
}
//--------------------------------------
//encode UTF-8 (title,album,song title in other language cause web client UTF-8 decoding error)
//code will be decode with Javascript in web page "ก" -> E0E881 (3 bytes)
String encodeToUTF8(const String &input) {
  String sanitized = "";
  for (size_t i = 0; i < input.length()-1; i++) {//remove suffix 00 by minus length with 1
    char c = input.charAt(i);
    if (isPrintable(c) && c != '\0') { // Remove NULL and printable characters
      sanitized += c;//skip ascii char
    } else {
      char hexString[3];   // 2 characters for the hex representation + 1 for the null terminator
      snprintf(hexString, sizeof(hexString), "%02X", (unsigned char)c); // Convert character to hex and store in hexString
      sanitized += hexString;
    }
  }
  return sanitized;
}

//--------------------------------------
//update ipod screen content to web client
void update_ipod_screen() {
  String content=ipodscreen_header;
  content.concat("\n____________________________________");
  content.concat("\nPlaylist: ");
  content.concat(String(cur_playlist_pos+1));
  content.concat("/");
  content.concat(String(total_playlist));
  content.concat(" | ");
  content.concat(playlist_title[cur_playlist_pos]);
  content.concat("\nArtist: ");
  content.concat(String(total_artist));
  content.concat(" | Album: ");
  content.concat(String(total_album));
  content.concat("\n|");
  content.concat(play_status);
  content.concat("|🔀: ");
  content.concat(shuffle_text);
  content.concat("|🔁: ");
  content.concat(repeat_text);
  content.concat("|🎶: ");
  content.concat(cur_song_pos+1);
  content.concat("/");
  content.concat(total_song);
  content.concat("\n____________________________________");
  content.concat("\nSong  | ");
  content.concat(song_title);
  content.concat("\nArtist| ");
  content.concat(song_artist);
  content.concat("\nAlbum | ");
  content.concat(song_album);
 
  //add bar and elapsed time text
  content.concat(bar); //progress bar
  content.concat(elapsedTime.text);//show elapsed time
  String space = "";
  uint8_t letters = 35-elapsedTime.length-remainTime.length;
  for (byte i=0;i<letters;i++)
     space.concat(" ");//add space between elapsed and remain  
  content.concat(space);
  content.concat("-");
  content.concat(remainTime.text);//show minus remain time
  //content.concat('\0');//add null byte 0x00;

  webSocket.broadcastTXT(content);//send screen content update to web client
 
}
//--------------------------------------
//list item type from start and range to ipod screen
void listByType(uint8_t type,uint16_t start_index,uint16_t total_type) {//type ,  start index,  total number of type
      SEND_COMMAND(POLL_STOP);//stop polling
      pollStatus = false;
      lasttime = millis();//reset screen delay timer
      uint8_t no_item_to_read = 0;
      //over range checking
      if (start_index+read_item_count > total_type) no_item_to_read = total_type - start_index;//range is over total
      else no_item_to_read = read_item_count;//10 default range = 10;
      REQ_NAME_TYPE_OFFSET_RANGE[6] = type;
      byte byteArray[4]; // Your 4-byte array
      integerToByteArray(start_index, byteArray);//start point
      for (byte i=0;i<4;i++) REQ_NAME_TYPE_OFFSET_RANGE[i+7] = byteArray[i];
      integerToByteArray(no_item_to_read, byteArray);//range to query
      for (byte i=0;i<4;i++) REQ_NAME_TYPE_OFFSET_RANGE[i+11] = byteArray[i];
      SEND_COMMAND(REQ_NAME_TYPE_OFFSET_RANGE);
}     
//################################################
//===== CONTROL PANEL FORM WEB CLIENT
//check web client ipod button click control
void EXEC_CLIENT_COMMAND(String command) {
   //Serial.println(command);
   if (command == "MODEL") {//get ipod model
     SEND_COMMAND(REQ_IPOD_MODEL);
   }
   //------------------
   if (command == "CONNECT") {//Connect and switch to main play list and start play
      elapsedTime = mill2HMS(elapse_time_sec);
      remainTime = mill2HMS(remain_time_sec);    
      SEND_COMMAND(MODE_ADVANCE_REMOTE);//switch to Air Mode    
      cur_playlist_pos = 0;//adjust 0
      cur_song_pos = 0;//set to track pos 0 after switch playlist
      number_string = "0";//clear number after choose type
      SEND_COMMAND(SWITCH_PL_0);//switch main playlist

   } 
   //----------------------
   if (command == "DISCONT") {//user click disconnect to stop ipod
  
      SEND_COMMAND(CMD_EXIT_REMOTE);
      ipod_model = "-";
      ipod_capacity = "-";
      ipod_name = "-";
      ipod_firmware = "-";
      ipod_serial = "-";
      String splashText = "iPod Smart Dock is ready!\n\n    📵 iPod is not connected ❗\nTo connect to an iPod.\nPlug in 30 pin dock connector\nthen click 'CONNECT' button.\n\n╔═══╗       iPod       ╔═══╗\n║WEB║ --->  Smart ---> ║███║\n║APP║ <---  Dock  <--- ║(O)║♫ ♪ ♫ ♪\n╚═══╝       ====       ╚═══╝\n▄ █ ▄ █ ▄ ▄ █ ▄ █ ▄ █\n\nCreated by Va&Cob";  //send init screen after coonect
      webSocket.broadcastTXT(splashText);//send splash screen to client
   } 
   //----------------------
  //switch to type 1 wiht para 4
   if (command == "PLAYLIST") {//list playlist
      TYPE = 0x01;//switch playlist
      listByType(TYPE, cur_playlist_pos, total_playlist); 
   }
   if (command == "ARTIST") {//list artist
      TYPE = 0x02;//switch artist
      listByType(TYPE, cur_artist_pos, total_artist); 
   }
   if (command == "ALBUM") {
      TYPE = 0x03;//switch album
      listByType(TYPE, cur_album_pos, total_album); 
   }
   if (command == "SONG") {
      TYPE = 0x05;//switch song
      listByType(TYPE, cur_song_pos, total_song); 
   }
//number key
    if (command == "0") number_string.concat("0");
    if (command == "1") number_string.concat("1");
    if (command == "2") number_string.concat("2");
    if (command == "3") number_string.concat("3");
    if (command == "4") number_string.concat("4");
    if (command == "5") number_string.concat("5");
    if (command == "6") number_string.concat("6");
    if (command == "7") number_string.concat("7");
    if (command == "8") number_string.concat("8");
    if (command == "9") number_string.concat("9");
    if (command == "CLEAR") number_string = "0";
  //Serial.println(number_string);
  //----------------------
   
   if (command == "POLL ON") SEND_COMMAND(POLL_START);//command from client to start polling

   if (command == "POLL OFF") SEND_COMMAND(POLL_STOP);//command from client to stop polling

//***************************
   if (command == "ENTER") { //execute command
  //   SEND_COMMAND(POLL_STOP);
     SWITCH_TYPE_NO[6] = TYPE;//0x01=Playlist, 0x02=Artist, 0x03=Album, 0x04=Gene 0x05=Track
     byte byteArray[4]; // Your 4-byte array
     uint16_t number = number_string.toInt();//hold actual number from keypad
     if (number == 0) number = 1;//prevent error ,cannot be 0
     switch (TYPE) {
       case 0x01: {//excute play list 
         cur_playlist_pos = number-1;//adjust 0
         if (number > total_playlist) cur_playlist_pos = total_playlist - 1;//if choose more than max playlist, switch to last playlist
         cur_song_pos = 0;//set to track pos 0 after switch playlist
         integerToByteArray(number-1,byteArray);//convert number to byte array
         for (byte i=0;i<4;i++) SWITCH_TYPE_NO[i+7] = byteArray[i];
         SEND_COMMAND(SWITCH_TYPE_NO);//0x00,0x17+ type(1) + number(4)   
         //integerToByteArray(cur_song_pos ,byteArray);//convert number to byte array
         for (byte i=0;i<4;i++) EXCUTE_PL_JUMP_SONGNO[i+6] = 0xFF;//switch to selected playlist with song number 1 0xff,0xff,0xff,0xff
         SEND_COMMAND(EXCUTE_PL_JUMP_SONGNO);//0x00,0x28 +0xff,0xff,0xff,0xff jumpt to first track of playhlist  
         SEND_COMMAND(REQ_NO_SONG_CUR_PL);//get no of song in current playlist
        
         listByType(TYPE, cur_playlist_pos, 1);
  
        } break;//playlist

       case 0x02: {//artist
         cur_artist_pos = number-1;//adjust 0
         if (number > total_artist) cur_artist_pos = total_artist - 1;//if choose more than max artis, switch to last artist
         cur_song_pos = 0;//set to track pos 0 after switch artist
         integerToByteArray(number-1,byteArray);//convert number to byte array
         for (byte i=0;i<4;i++) SWITCH_TYPE_NO[i+7] = byteArray[i];
         SEND_COMMAND(SWITCH_TYPE_NO);//0x00,0x17+ type(1) + number(4)   
         //integerToByteArray(cur_song_pos ,byteArray);//convert number to byte array
         for (byte i=0;i<4;i++) EXCUTE_PL_JUMP_SONGNO[i+6] = 0xFF;//switch to selected artist with song number 1 0xff,0xff,0xff,0xff
         SEND_COMMAND(EXCUTE_PL_JUMP_SONGNO);//0x00,0x28 +0xff,0xff,0xff,0xff jumpt to first track of playhlist  
         SEND_COMMAND(REQ_NO_SONG_CUR_PL);//get no of song in current artist
        } break;//artist

       case 0x03: {//album
         cur_album_pos = number-1;//adjust 0
         if (number > total_album) cur_album_pos = total_album - 1;//if choose more than max artis, switch to last album
         cur_song_pos = 0;//set to track pos 0 after switch album
         integerToByteArray(number-1,byteArray);//convert number to byte array
         for (byte i=0;i<4;i++) SWITCH_TYPE_NO[i+7] = byteArray[i];
         SEND_COMMAND(SWITCH_TYPE_NO);//0x00,0x17+ type(1) + number(4)   
         //integerToByteArray(cur_song_pos ,byteArray);//convert number to byte array
         for (byte i=0;i<4;i++) EXCUTE_PL_JUMP_SONGNO[i+6] = 0xFF;//switch to selected album with song number 1 0xff,0xff,0xff,0xff
         SEND_COMMAND(EXCUTE_PL_JUMP_SONGNO);//0x00,0x28 +0xff,0xff,0xff,0xff jumpt to first track of playhlist  
         SEND_COMMAND(REQ_NO_SONG_CUR_PL);//get no of song in current album
        }break;//album

       case 0x05: {//jump to song no.
         cur_song_pos = number-1;//adjust 0
         if (number > total_song) cur_song_pos = total_song - 1;//if choose more than max songcount, switch to last song
         integerToByteArray(cur_song_pos,byteArray);//convert number to byte array
         for (byte i=0;i<4;i++) JUMP_SONG_NO[i+6] = byteArray[i];
         SEND_COMMAND(JUMP_SONG_NO);

        } break;//track
      }//switch TYPE
      number_string = "0";//clear number after choose type
      ipod_music_db_step = QUERY_TRACK_INFO_STEP_NO;//query track information after change playlist/artist/album/track

   }//EXEC
//***************************
   if (command == "SHUFFLE") {
     shuffle_mode++;
     if (shuffle_mode > 0x02) shuffle_mode = 0x00;
     SET_SHUFFLE_MODE[6] = shuffle_mode;
     SEND_COMMAND(SET_SHUFFLE_MODE);
     SEND_COMMAND(REQ_SHUFFLE_MODE);
     
    }
   //---------------------- 
   if (command == "REPEAT") {
      repeat_mode++;
      if (repeat_mode > 0x02) repeat_mode = 0x00;
      SET_REPEAT_MODE[6] = repeat_mode;
      SEND_COMMAND(SET_REPEAT_MODE); 
      SEND_COMMAND(REQ_REPEAT_MODE);
   }
   //---------play control-------------
   if (command == "|<<") {
      SEND_COMMAND(CMD_PREV_TRACK);
   }
   if (command == "<<") {
      SEND_COMMAND(CMD_FAST_REVERSE);
      play_status = "⏪"; 
      update_ipod_screen();
   }
   if (command == "> ||") {
      SEND_COMMAND(CMD_PLAY_PAUSE);
   }
   if (command == ">>") {
      SEND_COMMAND(CMD_FAST_FORWARD); 
      play_status = "⏩"; 
      update_ipod_screen();
   }
   if (command == ">>|") {
      SEND_COMMAND(CMD_NEXT_TRACK);
   }
   if (command == "release") {//stop ffw /frw
     SEND_COMMAND(CMD_STOP_FAST);
   }
 //debugging
   if (command == "DEBUG_ON") debug = true;//enable debug. send ipod response message to client
   if (command == "DEBUG_OFF") debug = false;

   if (command == "Info") {//return smart dock information

        SEND_COMMAND(POLL_STOP);
        pollStatus = false;
        String ipAdress = WiFi.localIP().toString();
        String freeheap = (String)ESP.getFreeHeap();
        String freestack = (String)ESP.getFreeContStack();

        String info = "           INFORMATION\n \nName     : ";
        info.concat(ipod_name);
        info.concat("\nModel    : ");  
        info.concat(ipod_model);
        info.concat("\nCapacity : ");
        info.concat(ipod_capacity);
        info.concat("\nFirmware : ");
        info.concat(ipod_firmware);
        info.concat("\nSerial No: ");
        info.concat(ipod_serial);
        info.concat("\n \nFree Heap : ");
        info.concat(freeheap);
        info.concat(" bytes\nFree Stack: ");
        info.concat(freestack);
        info.concat(" bytes\nIP Address: ");
        info.concat(ipAdress);
        info.concat("\nVersion   : ");
        info.concat(version);
        
        webSocket.broadcastTXT(info);
        lasttime = millis();//reset screen delay timer
   }//if info

   if (command == "Buy ☕︎") {//buy coffee for me

   }


}//check client command

//################################################
//====== READ MESSAGE RETURNED FROM IPOD
void READ_IPOD_RESPONSE(uint8_t ipod_response[]) {
//ipod_response {0xff,0x55,0x03,0x04,0x00,0x15};

  byte length = ipod_response[2];//content length
  byte mode = ipod_response[3];//mode
 //-------------------------- 
#ifdef debug_ipod_resp_serial//for debubggin ipod response on serial only
 Serial.print(F("\nIPOD > "));
  for (byte i =0;i<length;i++) {
      Serial.print(ipod_response[i],HEX);
      Serial.print("|");
  }  
  Serial.printf(" [%d]\n", length);//byte array length  
#endif      
//-------------------------- 

  if (mode == 0x00) {//General lingo mode
    byte function = ipod_response[4];//ipod function 
    switch (function) {
      case 0x08: //REQ_IPOD_NAME
        ipod_name = "";
        for (byte i=5;i<3+length;i++)
          ipod_name.concat(char(ipod_response[i]));
        ipod_name = encodeToUTF8(ipod_name);   
        playlist_title[0] = ipod_name;//first playlist have ipod name as playlist  
        break;
      case 0x0A: //REQ_IPOD_FWVER
        ipod_firmware = "";
        for (byte i=5;i<3+length;i++) {
          ipod_firmware.concat(ipod_response[i]);
          ipod_firmware.concat(".");
        }  
        ipod_firmware = encodeToUTF8(ipod_firmware);          
        break;
      case 0x0C: //REQ_IPOD_SERIAL
        ipod_serial = "";
        for (byte i=5;i<3+length;i++) {
          ipod_serial.concat(char(ipod_response[i]));
        }  
        ipod_serial = encodeToUTF8(ipod_serial);          
        break;
      case 0x0E: //REQ_IPOD_MODEL
        ipod_model = "";
        byte byteArray[2];
         for (byte i=0;i<2;i++) byteArray[i] = ipod_response[i+5];//we take only first 2 bytes
         uint8_t model_code = byteArrayToInteger(byteArray,2);
         ipod_model = model_list[model_code] + " ";//model name
         for (byte i=0;i<6;i++) 
          ipod_model.concat(char(ipod_response[i+9]));//model number
        break;

    }//switch

  } else { //extended mode 0x04

   byte function = ipod_response[5];//other mode function is in byte5
   switch (function) {
    //---------Acknoledge command response
    case 0x01: {//command response follow by result(1)+command(2) ,0 = succes,2 = fail,4=excess limit

      if (ipod_response[6] == 0x00) {//command ack, OK
        switch (ipod_response[8]) {//read feedback function
          case 0x16: ipod_music_db_step = QUERY_IPOD_INFO_STEP_NO; break;//switch to playlist 0
          case 0x29: ipod_music_db_step = QUERY_TRACK_INFO_STEP_NO; break;//get navigation response -> get track info
          case 0x17: webSocket.broadcastTXT("\n\n\n█▀█ █▀▀ ▄▀█ █▀▄ █ █▄░█ █▀▀ ░ ░\n█▀▄ ██▄ █▀█ █▄▀ █ █░▀█ █▄█ ▄ ▄");break;//swtich main library PL 0
        }//switch  
      } else {//maybe 0x02 = Fail
         switch (ipod_response[8]) {//read feedback function
           case 0x1A: //REQ_NAME_TYPE_OFFSET_RANGE ERROR
             SEND_COMMAND(POLL_START);
             pollStatus = true;
             break;
         }//switch  

      }//else if     
    } break; 
    //--------------
    
    case 0x19: {//REQ_COUNT_TYPE

      switch (TYPE) {
        case 0x01: {
         byte byteArray[4]; // Your 4-byte array
         for (byte i=0;i<4;i++) byteArray[i] = ipod_response[i+6];
         total_playlist = byteArrayToInteger(byteArray,4);
        } break; 

        case 0x02: {//artist
         byte byteArray[4]; // Your 4-byte array
         for (byte i=0;i<4;i++) byteArray[i] = ipod_response[i+6];
         total_artist = byteArrayToInteger(byteArray,4);//convert 4 bytes to 32bit int
        } break;

        case 0x03: {//album
         byte byteArray[4]; // Your 4-byte array
         for (byte i=0;i<4;i++) byteArray[i] = ipod_response[i+6];
         total_album = byteArrayToInteger(byteArray,4);//convert 4 bytes to 32bit int
        } break;

        case 0x05: {//track
         byte byteArray[4]; // Your 4-byte array
         for (byte i=0;i<4;i++) byteArray[i] = ipod_response[i+6];
         total_song = byteArrayToInteger(byteArray,4);//convert 4 bytes to 32bit int
        } break;  

      } //switch
    } break;//case 0x19
    //*************************** DISPLAY LIST
    case 0x1B: {//get string list of given type and range number 4, text the rest
      byte byteArray[4]; // Your 4-byte array
      for (byte i=0;i<4;i++) byteArray[i] = ipod_response[i+6];//get number index
      uint16_t num = byteArrayToInteger(byteArray,4)+1;

      String name = "";
      for (byte i=0;i<length-7;i++) name.concat(char(ipod_response[i+10]));//read name

       name = encodeToUTF8(name);
     
      uint16_t start_index,total_type,no_item_to_read = 0; 
      
      switch (TYPE) {//first line of list
        case 0x01: 
          if (num == cur_playlist_pos+1) list = "Playlist:\n____________________________________\n";
          start_index = cur_playlist_pos;
          total_type = total_playlist;
          playlist_title[num - 1] = name;//save all playlist in LIST
          
          break;
        case 0x02: 
          if (num == cur_artist_pos+1) list = "Artist:\n____________________________________\n";
          start_index = cur_artist_pos;
          total_type = total_artist;
          break;
        case 0x03:
          if (num == cur_album_pos+1) list = "Album:\n____________________________________\n";
          start_index = cur_album_pos;
          total_type = total_album;
          break;
        case 0x05:
          if (num == cur_song_pos+1) list = "Song:\n____________________________________\n";
          start_index = cur_song_pos;
          total_type = total_song;
          break;    
      }//swtich     
      
      if (num == start_index+1) {//first title from first index
        list.concat("▶"+String(num)+":"+name+"\n");//add line
      }
      else list.concat(" "+String(num)+":"+name+"\n");//add line
      //over range checking
      if (start_index+read_item_count > total_type) no_item_to_read = total_type - start_index;//range is over total
      else no_item_to_read = read_item_count;//10 default range = 10;
    
      if (num == start_index + no_item_to_read) {//end of query
        webSocket.broadcastTXT(list);//send to client screen
        list = "";
        //delay(3000);
       // SEND_COMMAND(POLL_START);
       // pollStatus = true;
  
       }    

    } break;//case 0x1B
    //***************************
    case 0x1D: {//Track length in milliseconds, elapsed time in milliseconds, status=0x0 stop, 0x01 playing, 0x02 paused
      byte byteArray[4]; // Your 4-byte array
      for (byte i=0;i<4;i++) byteArray[i] = ipod_response[i+6];
      total_time_sec = byteArrayToInteger(byteArray,4)/1000;//get total track length
      for (byte i=0;i<4;i++) byteArray[i] = ipod_response[i+10]; 
      elapse_time_sec = byteArrayToInteger(byteArray,4)/1000;//get current track time
      playing = ipod_response[14];//status=0x00 stop, 0x01 playing, 0x02 paused
      if (playing == 1) {
        play_status = "▶️";//playing
        SEND_COMMAND(POLL_START);
      }
      if (playing == 2) {
        play_status = "⏸️";//pause
        SEND_COMMAND(POLL_STOP);
      }  
            //get time in text and length
      
      elapsedTime = mill2HMS(elapse_time_sec);
      remain_time_sec = total_time_sec - elapse_time_sec;      
      remainTime = mill2HMS(remain_time_sec);

    } break;//case 0x1C
    //--------------
    case 0x1F: {//cur pos
         byte byteArray[4]; // Your 4-byte array
         for (byte i=0;i<4;i++) byteArray[i] = ipod_response[i+6];
         cur_song_pos =  byteArrayToInteger(byteArray,4);//convert 4 bytes to 32bit int

    } break;  
    //--------------
    case 0x21: {//get song title
      song_title = "";
      for (byte i=6;i<3+length;i++) {
        song_title.concat(char(ipod_response[i]));
      } 
     
     song_title = encodeToUTF8(song_title);

    } break;   
    //-------------- 
    case 0x23: {//get artist name
      song_artist = "";
      for (byte i=6;i<3+length;i++) {
        song_artist.concat(char(ipod_response[i]));  
      }

   song_artist = encodeToUTF8(song_artist);
  
    } break;  
    //--------------
    case 0x25: {//get album title
      song_album = "";
      for (byte i=6;i<3+length;i++) {
        song_album.concat(char(ipod_response[i]));
      }     

    song_album = encodeToUTF8(song_album);
    } break;  
    //--------------
    case 0x2D: {//shuffle mode
        shuffle_mode = ipod_response[6];
        switch (shuffle_mode) {
          case 0x00: shuffle_text = "Off"; break;
          case 0x01: shuffle_text = "Songs"; break;
          case 0x02: shuffle_text = "Albums"; break; 
        }//switch
    } break;
    //--------------
    case 0x30: {//repeat mode
        repeat_mode = ipod_response[6];
        switch (repeat_mode) {
          case 0x00: repeat_text = "Off"; break;
          case 0x01: repeat_text = "One"; break;
          case 0x02: repeat_text = "All"; break; 
        }//switch
    } break;
    //--------------
    case 0x36: {//REQ_NO_SONG_CUR_PL get no of song in current playlist
      byte byteArray[4]; // Your 4-byte array
      for (byte i=0;i<4;i++) byteArray[i] = ipod_response[i+6];
        total_song = byteArrayToInteger(byteArray,4);//convert 4 bytes to 32bit int
      
      if (total_song == 0) total_song = 1;//prevent esp-01 crash
    } break;  
    //--------------
    case 0x27: {//polling song elapsed time
      if (length == 8) {//time elapsed 4 byte, prevent wrong time elapsed read
        byte byteArray[4]; // Your 4-byte array
        for (byte i=0;i<4;i++) byteArray[i] = ipod_response[i+7];
        elapse_time_sec = byteArrayToInteger(byteArray,4)/1000;//convert 4 bytes to 32bit int
      //detect new track play , prevent repeatly query track information
        if ((elapse_time_sec == 0) && (track_pos_0)) {
          track_pos_0 = false;
          ipod_music_db_step = QUERY_TRACK_INFO_STEP_NO;//query track information      
      //  Serial.println(F("\nget track info"));
        } else track_pos_0 = true;
      //create progress bar
        uint8_t count = elapse_time_sec*34/total_time_sec;
        bar = "\n \n|";
        for (byte i=0;i<count;i++) bar.concat("■");//elapsed
        for (byte i=0;i<34-count;i++) bar.concat(" ");//remain
        bar.concat("|\n");
        if (elapse_time_sec  > total_time_sec) elapse_time_sec = 0;//prevent bad draw 
        elapsedTime = mill2HMS(elapse_time_sec);//get elapsed time text
        remain_time_sec = total_time_sec - elapse_time_sec;//calculate reamin time
        remainTime = mill2HMS(remain_time_sec);//get remain time text
      
      update_ipod_screen();//update ipod screen
      }//length == 6 
    } break;

  }//switch
  }//else mode == 0x00 
/*more response
.
.
*/
  ipod_music_db_flag = true;//start query ipod

}//