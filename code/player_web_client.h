/* iPod Web Server */

char ipod_player_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
<title>iPod Smart Dock</title>
<link rel="icon" type="image/x-icon" href="data:image/x-icon;base64,
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" />
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
  body {
    background-color: #000;
    font-family: Arial, Helvetica, sans-serif;
    margin: 0;
    padding: 0;
    display: flex;
    align-items: center;
    justify-content: center;
    height: 100vh;
  }
  .ipod-container {
    background-color: beige;
    border-radius: 30px;
    box-shadow: 0px 0px 20px rgba(0, 0, 0, 0.5);
    padding: 25px;
    display: flex;
    flex-direction: column;
    align-items: center;
    box-sizing: border-box;
  }

.list-container {
  background-color: #a3a375;
  border: 2px solid #999;
  border-radius: 10px;
  box-sizing: border-box;
  width: 380px;
  height: 300px;
  overflow-x: none;
  overflow-y: none;
  display: flex;
  flex-direction: column;
  align-items: left;
  justify-content: center;
  font-size: 16px;
  font-family: "Courier New", monospace;
  font-weight: bold;
}

.item-list {
  list-style: none;
  padding: 0;
  margin: 0;
}

.item-list li {
  padding: 1px;
  cursor: pointer;
  white-space: pre;
  overflow: hidden;
  text-overflow: ellipsis;
}

.highlighted{
  background-color: lightgrey;
}

.item-list .selected {
  background-color: #007bff;
  color: #fff;
}
  #button-grid {
    margin: 0 auto; /* Center the container horizontally */
    display: grid;
    grid-template-columns: repeat(5, 1fr); /* 5 columns */
    grid-gap: 5px;
  }
  .matrix-button {  
    background-color: beige;
    color: Crimson;
    border: 1px solid #ccc;
    padding: 10px 1px;
    text-align: center;
    text-decoration: none;
    display: inline-block;
    font-size: px;
    border-radius: 5px;
    cursor: pointer;
  }  
  .matrix-button:active {
    color:OrangeRed;
    background-color:lightgrey;
    box-shadow: 0 1px #666;
    transform: translateY(1px);
  }
  .config-button {  
    background-color: beige;
    color: green;
    border: 1px solid #ccc;
    padding: 10px 1px;
    text-align: center;
    text-decoration: none;
    display: inline-block;
    font-size: px;
    border-radius: 5px;
    cursor: pointer;
    font-weight: normal;
  }   
  .config-button:active {
    color: red;
    background-color: lightgrey;
    box-shadow: 0 1px #666;
    transform: translateY(1px);
  }
  .navigate-button {  
    background-color: beige;
    color: black;
    border: 1px solid #ccc;
    padding: 10px 1px;
    text-align: center;
    text-decoration: none;
    display: inline-block;
    font-size: px;
    border-radius: 5px;
    cursor: pointer;
    font-weight: 900;
  }   
  .navigate-button:active {
    color: red;
    background-color: lightgrey;
    box-shadow: 0 1px #666;
    transform: translateY(1px);
  }
   .right-panel {
    background-color: #b3b3ff;
    border-radius: 25px;
    box-shadow: 0px 0px 20px rgba(0, 0, 0, 0.5);
    padding: 20px;
    display: flex;
    flex-direction: column;
    align-items: center;
    box-sizing: border-box;
    height: 700px;
  }   
  #terminal {   
    flex-direction: column;  
    border: 1px solid #ccc;
    font-size: 16px;
    margin-bottom: 20px;
    background: white;
    color: black;
    padding:  10% 20px;
    height: 60%;
    width: 90%;
  }
  .fontbtn {
  border: 1px solid grey;
  background-color: beige;
  color: black;
  font-size: 14px;
  cursor: pointer;
  text-align: center;
  text-decoration: none;
  font-size: 16px;
  border-radius: 20%;
}  
</style>
</head>
<body>

<div class="ipod-container">
  <div class="list-container">
    <ul class="item-list" id="scrollable-list">
     <li>__________________________________</li>
     <li> </li>
     <li> </li>
     <li> </li>
     <li> </li>
     <li> </li>
     <li>Connecting... Please wait...</li>
     <li> </li>
     <li> </li>
     <li> </li>
     <li> </li>
     <li> </li>
     <li>__________________________________</li>
  
   </ul>
  </div>
    <p style="text-align:center; font-family:'Verdana'; color:grey;">
      <b>░░░░░░░░ iPod Smart Dock ░░░░░░░░</b>
    </p>
    <div id="button-grid">
      <button class="config-button" >Buy ☕︎</button>
      <button class="config-button" >Font -</button>
      <button class="config-button" >= Font =</button>
      <button class="config-button" >Font +</button>
      <button class="config-button" >Info</button>

      <button class="matrix-button" disabled>CONNECT</button>
      <button class="matrix-button" disabled>DISCONT</button>
      <button class="matrix-button" disabled>7</button>
      <button class="matrix-button" disabled>8</button>
      <button class="matrix-button" disabled>9</button>   

      <button class="matrix-button" disabled>PLAYLIST</button>      
      <button class="matrix-button" disabled>SONG</button>
      <button class="matrix-button" disabled>4</button>
      <button class="matrix-button" disabled>5</button>      
      <button class="matrix-button" disabled>6</button>

      <button class="matrix-button" disabled>ALBUM</button>
      <button class="matrix-button" disabled>ARTIST</button>
      <button class="matrix-button" disabled>1</button>
      <button class="matrix-button" disabled>2</button>
      <button class="matrix-button" disabled>3</button> 

      <button class="matrix-button" disabled>SHUFFLE</button>
      <button class="matrix-button" disabled>REPEAT</button>
      <button class="matrix-button" disabled>CLEAR</button>
      <button class="matrix-button" disabled>0</button>
      <button class="matrix-button" disabled>ENTER</button>
    
      <button class="navigate-button" disabled>|<<</button>
      <button class="navigate-button" disabled><<</button>
      <button class="navigate-button" disabled>> ||</button>
      <button class="navigate-button" disabled>>></button>
      <button class="navigate-button" disabled>>>|</button>
    </div>
 
<p style="font-size:12px"><label ondblclick="toggle_debug()">♫ Created by Va&Cob ♫</p>  
</div> 

<div class="right-panel" id="right-panel" style="display:none">
        <textarea readonly id="terminal"></textarea>           
         Send Command Here<br> 
        <input type="text" id="input" style="width:100%;height:20px;font-size:12pt;" placeholder="Enter command...0xff,0x55,length,mode0,mode1,....exclude CRC">
        <button id="sendButton" style="height:20px;font-size:10pt;">Send</button>    
</div>

<script>

 const itemList = document.getElementById('scrollable-list');
 const listItems = itemList.getElementsByTagName("li");//update each list by listItems[0...12]
 
 const buttons = document.querySelectorAll('.matrix-button'); 
 const playcontrol = document.querySelectorAll('.navigate-button');
 const configButton = document.querySelectorAll('.config-button');
 const terminal = document.getElementById('terminal');
 const input = document.getElementById('input');
 const sendButton = document.getElementById('sendButton');
 let fontNo = 0; 
        
//web socket      
 socket = new WebSocket("ws:/" + "/" + location.host + ":81");
 socket.onopen = function(e) { 
    console.log("[socket] socket.onopen ");//socket opened
    if (socket.readyState === 1) {//wait until socket is ready
      const buttonGrid = document.getElementById('button-grid');
      const elements = buttonGrid.querySelectorAll('*'); // Select all elements within the div
      elements.forEach(element => {
        element.disabled = false;//enable matrix buttons
      });
    }
 };
 socket.onerror = function(e) {
    console.error("[socket] socket.onerror ");
    terminal.innerHTML += "Websocket: "+e.type;
    terminal.innerHTML += '\n';
    terminal.scrollTop = terminal.scrollHeight;   
 };
 socket.onclose = function(e) { 
    terminal.innerHTML += "Websocket: "+e.type;
    terminal.innerHTML += '\n';
    terminal.scrollTop = terminal.scrollHeight;
    alert("📵 Lost connection with iPod Smart Dock❗");
    location.reload();//refresh page
 }; 

//incomming message from esp-01
 socket.onmessage = function(event) {
   const receivedText = event.data;
   const firstCharacter = receivedText.charAt(0);//Get the first character of the string
    // Compare the first character with "#"
   if (firstCharacter === "#") {//ipod debug message
     terminal.innerHTML += receivedText;
     terminal.innerHTML += '\n';
     terminal.scrollTop = terminal.scrollHeight;  
     //console.log(receivedText);
   } else {
     const inputString = decodeAndReplaceUTF8(receivedText);//decode UTF-8
     const arrayOfLines = inputString.split('\n');
     for (i=0;i<13;i++) listItems[i].textContent = arrayOfLines[i];
     //podscreen.value = receivedText;//get screen casting from esp-01
   }//if (firstchar)  
};

//button click 
 buttons.forEach(button => {
 button.addEventListener('click', () => {
 socket.send(button.textContent);
 });
 });
playcontrol.forEach(button => {
  button.addEventListener('click', () => {
    const text = button.textContent;
    if (text === "|<<" || text === "> ||" || text === ">>|") {
      socket.send(button.textContent);
    }
  });

  button.addEventListener('mousedown', () => {
    const text = button.textContent;
    if (text === "<<" || text === ">>") {
      socket.send(button.textContent);
    }
  });

  button.addEventListener('mouseup', () => {
    const text = button.textContent;
    if (text === "<<" || text === ">>") {
      socket.send("release");
    }
  });
});

 configButton.forEach(button => {
 button.addEventListener('click', () => {
	if (button.textContent === "Font -") decreaseFontSize();
  if (button.textContent === "Font +") increaseFontSize();
  if (button.textContent === "= Font =") changeFontStyle();
  if (button.textContent === "Info") socket.send(button.textContent);
  if (button.textContent === "List" ) socket.send(button.textContent);
   
 });
 });

//send button
 sendButton.addEventListener('click', function() {
   const byteArrayText = input.value.replaceAll(/\s/g, '');//Remove whitespace
   const stringArray = byteArrayText.split(',');
   const byteArray = new Uint8Array(stringArray.length);
   for (let i = 0; i < stringArray.length; i++) {
    byteArray[i] = parseInt(stringArray[i], 16);//parst hex to byte array
   }
   console.log(byteArray);
   socket.send(byteArray);//send byte array to esp-01
 });  
// Function to increase font size
function increaseFontSize() {
    var currentSize = window.getComputedStyle(itemList, null).getPropertyValue('font-size');
    var newSize = parseInt(currentSize) + 1; // Increase font size by 4 pixels
    itemList.style.fontSize = newSize + 'px';
}
// Function to decrease font size
function decreaseFontSize() {
    var currentSize = window.getComputedStyle(itemList, null).getPropertyValue('font-size');
    var newSize = parseInt(currentSize) - 1; // Increase font size by 4 pixels
    itemList.style.fontSize = newSize + 'px';
}
// Function to change font style
function changeFontStyle() {
    fontNo++;
    if (fontNo === 5) fontNo = 0;
    switch (fontNo) {
      case 0:itemList.style.fontFamily = "Courier New";break;
      case 1:itemList.style.fontFamily = "monospace";break;
      case 2:itemList.style.fontFamily = "Monaco";break;
      case 3:itemList.style.fontFamily = "PT Mono";break;
      case 4:itemList.style.fontFamily = "Menlo";break;
    }
}
// Function to toggle debug screen    
function toggle_debug() {
  var x = document.getElementById('right-panel');
  if (x.style.display === "none") {
    x.style.display = "block";//open debug screen
    socket.send("DEBUG_ON");//send to esp-01 to start send ipod response
  } else {
    x.style.display = "none";//close debug screen
    socket.send("DEBUG_OFF");//send to esp-01 to stop send ipod reponse
  }
}
//Function decodeUTF-8 to character 3 bytes E0E881 -> "ก"
function decodeAndReplaceUTF8(inputString) {
  const regex = /([0-9A-Fa-f]{6})/g;
  const decodedString = inputString.replace(regex, (match, hexBytes) => {
    const byte1 = parseInt(hexBytes.substr(0, 2), 16);
    const byte2 = parseInt(hexBytes.substr(2, 2), 16);
    const byte3 = parseInt(hexBytes.substr(4, 2), 16);
    const utf8Character = String.fromCharCode(((byte1 & 0x0F) << 12) | ((byte2 & 0x3F) << 6) | (byte3 & 0x3F));
    return utf8Character;
  });
  return decodedString;
}

// script.js


</script>
</body>
</html>

)=====";

//***************************************
char not_found_page[] PROGMEM = R"=====(
"<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Oops! Page Not Found</title>
<style>
  body {
    font-family: Arial, sans-serif;
    background-color: #f4f4f4;
    text-align: center;
  }
  .container {
    padding: 50px;
  }
  .emoji {
    font-size: 50px;
  }
  h1 {
    font-size: 28px;
    margin-bottom: 10px;
  }
  p {
    font-size: 18px;
  }
</style>
</head>
<body>
<div class="container">
  <div class="emoji">😿</div>
  <h1>There is nothing here!</h1>

  <a href="/">Go to iPod Smart Dock</a>
</div>
</body>
</html>
)=====";