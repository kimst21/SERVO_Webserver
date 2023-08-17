#include <WiFi.h>

const int servoPin = 48;  // GPIO48 

const char* ssid = "  "; // 라우터의 SSID 추가 
const char* password = "  "; //비밀번호 추가 

int dutyCycle = 0;

// PWM 속성 설정
const int PWMFreq = 50;
const int PWMChannel = 0;
const int PWMResolution = 8;
const int MAX_DUTY_CYCLE = (int)(pow(2, PWMResolution) - 1);

WiFiServer espServer(80); // 포트 번호가 80인 WiFiServer 인스턴스
// 80은 HTTP 웹 서버의 포트 번호입니다.

// 들어오는 HTTP GET 요청을 캡처할 문자열입니다. 
String request;

void setup()
{  
  Serial.begin(115200);
  ledcSetup(PWMChannel, PWMFreq, PWMResolution);
  // LED PWM 채널을 GPIO 핀에 연결합니다. 
  ledcAttachPin(servoPin, PWMChannel);
  ledcWrite(PWMChannel, dutyCycle);

  Serial.print("\n");
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA); // STA 모드에서 ESP32 구성 
  WiFi.begin(ssid, password); // 위의 SSID 및 비밀번호를 기반으로 Wi-Fi에 연결합니다. 
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print("*");
    delay(100);
  }
  Serial.print("\n");
  Serial.print("Connected to Wi-Fi: ");
  Serial.println(WiFi.SSID());
  delay(2000);

  Serial.print("\n");
  Serial.println("Starting ESP32 Web Server for Servo Control...");
  espServer.begin(); /* Start the HTTP web Server */
  Serial.println("ESP32 Servo Web Server Started");
  Serial.print("\n");
  Serial.print("The URL of ESP32 Servo Web Server is: ");
  Serial.print("http://");
  Serial.println(WiFi.localIP());
  Serial.print("\n");
  Serial.println("Use the above URL in your Browser to access ESP32 Servo Web Server\n");
}
void loop()
{
  WiFiClient client = espServer.available(); /* Check if a client is available */
  if(!client)
  {
    return;
  }

  Serial.println("New Client!!!");
  boolean currentLineIsBlank = true;
  while (client.connected())
  {
    if (client.available())
    {
      char c = client.read();
      request += c;
      Serial.write(c);
        // 줄 끝에 도달한 경우(새 줄을 받은 경우) 
        // 문자)를 입력했는데 줄이 비어 있으면 http 요청이 종료된 것입니다,
        // 그래서 답장을 보낼 수 있습니다
      if (c == '\n' && currentLineIsBlank)
      {
        client.println("HTTP/1.1 200 OK");
        client.println("Content-type:text/html");
        client.println("Connection: close");
        client.println();

        client.println("<!DOCTYPE html>");
        client.println("<html>");
        
        client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
        client.println("<link rel=\"icon\" href=\"data:,\">");

        // 텍스트 및 슬라이더용 CSS 스타일링 
        client.println("<style>body { font-family: \"Courier New\"; margin-left:auto; margin-right:auto; text-align:center;}");
        
        client.println(".slidecontainer { width: 100%;}");
        client.println(".slider { -webkit-appearance: none;");
        client.println("width: 30%; height: 20px; background: #d3d3d3;");
        client.println("outline: none; opacity: 0.7; -webkit-transition: .2s; transition: opacity .2s;}");
        client.println(".slider:hover { opacity: 1; }");

        client.println(".slider::-webkit-slider-thumb { -webkit-appearance: none;");
        client.println("appearance: none; width: 15px; height: 28px;");
        client.println("border-radius: 30%; background: #4CAF50; cursor: pointer;}");
        client.println(".slider::-moz-range-thumb { width: 25px; height: 25px; background: #4CAF50; cursor: pointer;}</style>");
        
        client.println("<script src=\"https://code.jquery.com/jquery-3.6.0.min.js\"></script>");
        //실제 웹 페이지
        client.println("</head><body><h2>ESP32 Web Controlled Servo</h2>");
        client.println("<p>Drag the slider to rotate the Servo.</p>");
        client.println("<input type=\"range\" min=\"0\" max=\"180\" class=\"slider\" id=\"servoRange\" onchange=\"servo(this.value)\"/>");
        client.println("<p>Angle: <span id=\"servoPos\"></span></p>");
        client.println("<script>");
        client.println("var slider = document.getElementById(\"servoRange\");");
        client.println("var output = document.getElementById(\"servoPos\");");
        client.println("output.innerHTML = slider.value;");
        client.println("slider.oninput = function(){output.innerHTML = this.value;}");
        client.println("$.ajaxSetup({timeout:1000}); function servo(angle) { ");
        client.println("$.get(\"/servovalue=\" + angle); {Connection: close};}</script>");
                
        client.println("</body></html>");   
        
        // 요청은 다음과 같은 형식입니다. * GET /서버값=143 /HTTP/1.1
        if(request.indexOf("GET /servovalue=") != -1)
        {
          int position1 = request.indexOf('='); // 요청 문자열에서 '='의 위치를 찾습니다.
          String angleStr = request.substring(position1+1); // 다음 2/3 문자는 원하는 각도를 알려줍니다. 
          int angleValue = angleStr.toInt();
          dutyCycle = map(angleValue, 0, 180, 5, 32);
          ledcWrite(PWMChannel, dutyCycle); 
        }
        client.println();
        break;
      }

        if(c == '\n')
        {
          currentLineIsBlank = true;
        }
        else if(c != '\r')
        {
          currentLineIsBlank = false;
        }
    }
  }
 
  delay(1);
  request = "";
  client.stop();
  Serial.println("Client disconnected");
  Serial.print("\n");
}