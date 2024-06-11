#include <WiFi.h>
#include <WebSocketsServer.h>
#include <WebServer.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8" />
<meta name="viewport" content="width=device-width, initial-scale=1.0" />
<title>Document</title>
</head>
<body>
<input type="text" id="ipInput" placeholder="IP" />
<button id="connectButton" onclick="connectWebSocket()">Connect</button>
<div id="connectStatus"></div>
<button id="msgButton" onclick="sendMessage()">Send Message</button>
<script>
const ipInput = document.getElementById("ipInput");
const connectButton = document.getElementById("connectButton");
const connectStatus = document.getElementById("connectStatus");
let ws = null;

function connectWebSocket() {
  ws = new WebSocket(`ws://${ipInput.value}`);
  ws.onopen = () => {
    connectStatus.innerHTML = "connected";
  };
  ws.onclose = () => {
    connectStatus.innerHTML = "disconnected";
  };
}

const msgButton = document.getElementById("msgButton");

let keys = {};
let controls = {
  upDown: 0,
  leftRight: 0,
  message: ""
};

document.addEventListener("keydown", function (event) {
  if (!keys[event.code]) {
    keys[event.code] = true;
  
      //input
    if (event.code === "ArrowUp") {
      controls.upDown++;
    }
    if (event.code === "ArrowDown") {
      controls.upDown--;
    }

    if (event.code === "ArrowRight") {
      controls.leftRight++;
    }
    if (event.code === "ArrowLeft") {
      controls.leftRight--;
    }
    sendInput()
  }
});

document.addEventListener("keyup", function (event) {
  if (keys[event.code]) {
    keys[event.code] = false;
    
    //reverse input values
    if (event.code === "ArrowUp") {
      controls.upDown--;
    }
    if (event.code === "ArrowDown") {
      controls.upDown++;
    }

    if (event.code === "ArrowRight") {
      controls.leftRight--;
    }
    if (event.code === "ArrowLeft") {
      controls.leftRight++;
    }
    sendInput()
  }
});

function sendMessage() {
  const message = prompt("Enter your message:");
  if (message !== null) {
    controls.message = message;
    sendInput();
  }
}

function sendInput() {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify(controls));
  }
}
</script>
</body>
</html>
)rawliteral";


const char *ssid = "artemis";
const char *password = "12345678";

WebSocketsServer webSocket = WebSocketsServer(80);
WebServer clientServer(81);

void setup() {
  Serial.begin(9600);
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

clientServer.on("/", HTTP_GET, []() {
  clientServer.send_P(200, "text/html", index_html);
});
  clientServer.begin();
}

void loop() {
  clientServer.handleClient();
  webSocket.loop();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %s\n", num, ip.toString().c_str());
        break;
      }
    case WStype_TEXT:
      Serial.printf("[%u] Received text: %s\n", num, payload);
      webSocket.sendTXT(num, "Message received");
      break;
  }
}