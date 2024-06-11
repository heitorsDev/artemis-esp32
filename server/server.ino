#include <WiFi.h>
#include <WebSocketsServer.h>
#include <WebServer.h>

const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">

<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>Document</title>
  <style>
    button {

      user-select: none;
    }
  </style>
</head>

<body>
  <button id="connectButton" onclick="connectWebSocket()">Connect</button>
  <div id="connectStatus"></div>
  <div>
    <button id="up1" ontouchstart="upPressed()" ontouchend="upReleased()" onclick="preventDefault(event)">UP</button>
    <button id="down1" ontouchstart="downPressed()" ontouchend="downReleased()"
      onclick="preventDefault(event)">DOWN</button>
    <button id="left1" ontouchstart="leftPressed()" ontouchend="leftReleased()"
      onclick="preventDefault(event)">LEFT</button>
    <button id="right1" ontouchstart="rightPressed()" ontouchend="rightReleased()"
      onclick="preventDefault(event)">RIGHT</button>
  </div>
  <script>
    const ipInput = document.getElementById("ipInput");
    const connectButton = document.getElementById("connectButton");
    const connectStatus = document.getElementById("connectStatus");
    let ws = null;

    function connectWebSocket() {
      ws = new WebSocket(`ws://192.168.4.1:80`);
      ws.onopen = () => {
        connectStatus.innerHTML = "connected";
      };
      ws.onclose = () => {
        connectStatus.innerHTML = "disconnected";
      };
    }

    let keys = {};

    let controls = {
      upDown: 0,
      leftRight: 0,
    };

    document.addEventListener("keydown", function (event) {
      if (!keys[event.code]) {
        keys[event.code] = true;

        if (event.code === "ArrowUp") {
          controls.upDown++;
        }
        if (event.code === "ArrowDown") {
          controls.upDown--;
        }

        console.log(controls.upDown);
      }
    });

    document.addEventListener("keyup", function (event) {
      if (keys[event.code]) {
        keys[event.code] = false;

        if (event.code === "ArrowUp") {
          controls.upDown--;
        }
        if (event.code === "ArrowDown") {
          controls.upDown++;
        }

        console.log(controls.upDown);
      }
    });


    function upPressed() {

      controls.upDown++;
      sendInput();
    }

    function upReleased() {
      controls.upDown--;
      sendInput();
    }

    function downPressed() {
      controls.upDown--;
      sendInput();
    }

    function downReleased() {
      controls.upDown++;
      sendInput();
    }

    function leftPressed() {
      controls.leftRight--;
      sendInput();
    }

    function leftReleased() {
      controls.leftRight++;
      sendInput();
    }

    function rightPressed() {
      controls.leftRight++;
      sendInput();
    }

    function rightReleased() {
      controls.leftRight--;
      sendInput();
    }

    function sendInput() {
      console.log(controls)
      if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify(controls));
      }
    }

    function preventDefault(event) {
      event.preventDefault();
      event.stopPropagation();
    }
  </script>
</body>

</html>
)rawliteral";


const char *ssid = "artemis";
const char *password = "artemis1";

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