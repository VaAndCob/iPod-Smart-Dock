//WiFi
#define ssidSize 3  // size of preconfig ssid array
//String ssid[ssidSize] = { "WARTCHEERANON_2.4G", "P.S.TAWEESUP #1", "" };
//String password[ssidSize] = { "Java2549", "pstaweesup028672211", "" };
String ssid[ssidSize] = { "", "", "" };
String password[ssidSize] = { "", "", "" };

uint8_t SSIDindex = 0; /*index to choose SSID*/

/*================================================*/
void WiFiSetup() {

    Serial.println(F("\r\n[ Configuration Mode ]"));     
    WiFi.mode(WIFI_AP);//switch to server mode
    WiFi.disconnect();
    
    const char* APname = "iPod Smart Dock";
    WiFi.softAP(APname, "");//start soft ap
    //HTTP GET hanlder
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      String config_page = "<!DOCTYPE html><html><head><title>iPod Smart Dock Configuration</title><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><style>body{font-family:Arial,sans-serif;background-color:#f7f7f7;margin:0;padding:0;display:flex;justify-content:center;align-items:center;height:100vh;}#content{max-width:600px;padding:20px;background-color:lightgrey;";
      config_page += "border:1px solid #ddd;border-radius:5px;box-shadow:0 2px 5px rgba(0,0,0,0.1);}fieldset{border:1px solid blue;padding:10px;border-radius:5px;margin-bottom:20px;}legend{font-weight:bold;color:blue;}label{font-weight:bold;display:inline-block;width:100px;}input[type=\"text\"]{width:95%;padding:5px;border:1px solid #ccc;border-radius:3px;margin-bottom:10px;}input[type=\"reset\"],input[type=\"submit\"]";
      config_page += "{background-color:#009688;color:#fff;border:none;padding:8px 20px;border-radius:3px;cursor:pointer;}input[type=\"reset\"]:hover{background-color:green;}input[type=\"submit\"]:hover{background-color:#00796b;}h3{margin-top:10px;color:#d32f2f;}</style></head>";
      config_page += "<body><div id=\"content\"><h2 style=\"color:green\">iPod Smart Dock<br>Configuration</h2><p style=\"color:black\">Created by Va&amp;Cob</p><form action=\"save\" method=\"post\" name=\"device\"><fieldset><legend>Pre-config WiFi SSID/Password</legend><label>SSID 1:</label><input type=\"text\" name=\"ssid0\" maxlength=\"32\"";
      config_page += "value=\""+ssid[0]+"\"><br><label>Password:</label><input type=\"text\" name=\"pwd0\" maxlength=\"32\" value=\""+password[0]+"\"><br><br><label>SSID 2:</label><input type=\"text\" name=\"ssid1\" maxlength=\"32\"";
      config_page += "value=\""+ssid[1]+"\"><br><label>Password:</label><input type=\"text\" name=\"pwd1\" maxlength=\"32\" value=\""+password[1]+"\"><br><br><label>SSID 3:</label><input type=\"text\" name=\"ssid2\" maxlength=\"32\"";
      config_page += "value=\""+ssid[2]+"\"><br><label>Password:</label><input type=\"text\" name=\"pwd2\" maxlength=\"32\" value=\""+password[2]+"\"><br><br><h3>Remark: Incorrect SSID and Password will cause the device unable to work!</h3></fieldset><p><input type=\"reset\" value=\"Reset\"> <input type=\"submit\" value=\"Save & Restart\"></p></form></div></body></html>";

      request->send(200, "text/html", config_page);
     });//http get
    //HTTP POST Handler
    server.on("/save", HTTP_POST, [](AsyncWebServerRequest* request){

        if (request->hasArg("ssid0") && request->hasArg("pwd0") && request->hasArg("ssid1") && request->hasArg("pwd1") && request->hasArg("ssid2") && request->hasArg("pwd2")) {
          String ssid0 = request->arg("ssid0");
          String ssid1 = request->arg("ssid1");
          String ssid2 = request->arg("ssid2");
          String pwd0 = request->arg("pwd0");
          String pwd1 = request->arg("pwd1");
          String pwd2 = request->arg("pwd2");   
          Serial.print(ssid0);Serial.print(",");Serial.println(pwd0);
          Serial.print(ssid1);Serial.print(",");Serial.println(pwd1);
          Serial.print(ssid2);Serial.print(",");Serial.println(pwd2);
       
        //save ssids, password to littleFS
        File file = LittleFS.open("/wifi_credentials.txt", "w");
        if (!file) {
          Serial.println("Failed to open file for writing!");
          request->send(400, "text/plain", "Error: unable to save!");
        } else { 
          file.println(ssid0+","+pwd0);//ssid,pwd\n
          file.println(ssid1+","+pwd1);//ssid,pwd\n
          file.println(ssid2+","+pwd2);//ssid,pwd\n      
          file.close();
          Serial.println("Saved...Restart...");
          request->send(200, "text/plain", "<html><body><h1>Configuration Complete<br>Restarting...</h1></body></html>");
          ESP.restart(); 
        }//if file 
      }//if request  
    });//http post

    server.begin();//start config server
    Serial.println(F("Access Point started!"));
    IPAddress myIP = WiFi.softAPIP();
    Serial.print(F("IP Address = "));
    Serial.print(myIP);
    Serial.printf("\nHostname = %s\n",APname);
    while (1) {//keep blinking and wait event handler not go to main loop
      delay(500);digitalWrite(LED,HIGH);
      delay(500);digitalWrite(LED,LOW);//on
    }//while
}//wifisetup()
/*----------------------------------------*/
//wifi conneciton
bool WiFiConnect(void) {

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  // Scan wifi AP
  Serial.println(F("\r\nPreconfig AP SSID to connect..."));
  for (byte i = 0; i < ssidSize; ++i) {
    Serial.print(i + 1);
    Serial.print(F(": "));
    Serial.println(ssid[i]);
  }  //show all preconfig AP
  Serial.println(F("\r\nAP Scan Start..."));
  byte n = WiFi.scanNetworks();
  SSIDindex = n;

  if (n == 0) {//ssid not match then go to Wifi Setup
    Serial.println(F("No networks found"));
    while (1) {
      delay(200);digitalWrite(LED,HIGH);
      delay(800);digitalWrite(LED,LOW);//on
    }

  } else {//found ssid, try to connect
    Serial.print(n);
    Serial.println(F(" networks found"));
    for (byte i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(F(": "));
      Serial.print(WiFi.SSID(i));
      Serial.print(F(" ("));
      Serial.print(WiFi.RSSI(i));
      Serial.println(F(")"));
      //      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
      delay(10);
      //match ssid with the preconfig ssid
      if (WiFi.SSID(i) == ssid[0]) { SSIDindex = 0; }
      if (WiFi.SSID(i) == ssid[1]) { SSIDindex = 1; }
      if (WiFi.SSID(i) == ssid[2]) { SSIDindex = 2; }
    }

    if (SSIDindex > 2) {//not match any ssid in list of 3 ssid
      Serial.println("No network matched. Switch to configuration mode.");
      setupmode = true;
      return false;//cannot connect wifi
    } else {
    //connect to AP by list compare to given ssid
    WiFi.begin(ssid[SSIDindex], password[SSIDindex]);
    Serial.print("\r\nConnecting to ");
    Serial.println(ssid[SSIDindex]);
    uint8_t trial = 0;
    while (WiFi.status() != WL_CONNECTED) {
      delay(200);digitalWrite(LED,HIGH);
      delay(800);digitalWrite(LED,LOW);//on
      Serial.print(F("."));
      trial++;
      if (trial >10) ESP.restart();
    }
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    Serial.println(F("Connected"));
    Serial.print(F("IP Address: "));
    Serial.println(WiFi.localIP());
    setupmode = false;
    return true;//connect successful
    }//SSIDIndex >2

  }//else
}//WiFiConnect

