// Import required libraries
#include <Wire.h>
#include <Adafruit_BNO055.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

//ボールが運ばれるところは半分の速さで
#define SENSOR_WIRE_GYRO (Wire)
#define OMEGAMAX (6.28 * 0.25) //L1,R1を押したときの角速度　回るときおそすぎたら0.25を0.5とかにする
#define VYMAX (1.0)
#define VXMAX (1.0)
#define BALLMAX (1.0)
#define BALL2MAX (1.0)
#define VELOCITY_LIMIT_ANGLAR (6.28 * 2.0) //pidの最大出力 90度回転（L2,R2）のとき　あんまり変更しない　ここを変更するとOMEGAMAXやゲインにも影響が出る

#include "gamepad.h"
#include "html.h"
#include "pin.h"
#include "Motor.h"
#include "diffdrive.h"
#include "imu.h"
#include "pid.h"

void test_loop();
GamePad gamepad;
Motor motor1(MOTER_R1,MOTER_R2);//★タイヤの正回転をここで合わせる
Motor motor2(MOTER_L2,MOTER_L1);
Motor motor3(MOTER_S1,MOTER_S2);
Motor motor4(MOTER1_1,MOTER1_2);
Motor motor5(MOTER2_1,MOTER2_2);
DiffDrive omni(0.4,&motor1, &motor2, &motor3);
Imu im(IMU_RESET);
PID pid_yaw(10.0, 0, 0.5, VELOCITY_LIMIT_ANGLAR);//★Pではやくなる　フラフラするときはDを上げる

// Replace network credentials with your own WiFi details
const char *ssid = "kata2024yui";
const char *password = "katarobo-2024";

bool GPIO_State = 0;
const int Led_Pin = 2;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

IPAddress gateway(192, 168, 0, 1);
IPAddress local_IP(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

void notifyClients() {
    ws.textAll(String(GPIO_State));
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len) {
        // for (int i = 0; i < len; ++i) {
        //     Serial.printf("%d ", data[i]);
        // }
        // Serial.println("");
        gamepad.update(data);
    };
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

void initWebSocket() {
    ws.onEvent(onEvent);
    server.addHandler(&ws);
}

String processor(const String &var) {
    Serial.println(var);
    if (var == "STATE") {
        if (GPIO_State) {
            return "ON";
        } else {
            return "OFF";
        }
    }
}

void setup() {
    // Serial port for debugging purposes
    Serial.begin(115200);
    pinMode(Led_Pin, OUTPUT);
    motor1.init();
    motor2.init();
    motor3.init();
    motor4.init();
    motor5.init();
    im.init();//ジャイロセンサをつけてないときここコメントアウト④
    // return;//ジャイロ単体テスト
    


    // Start AP Server
    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(local_IP, gateway, subnet);

    // Print ESP Local IP Address
    Serial.println(WiFi.softAPIP());

    initWebSocket();

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", html_page, processor);
    });

    // Start server
    server.begin();
}

void loop() {
  static float aim_yaw = 0;
  static unsigned long push_90_time_stamp = 0;//unsigned→符号なし long→64ビットの整数
  static unsigned long prev_loop_ms = 0;
  // test_loop();//ジャイロ単体テスト
  // return;//ジャイロ単体テスト
  static int i = 0;
  ws.cleanupClients();
  // 100Hz
  unsigned long now_loop_ms = millis();
  if((now_loop_ms - prev_loop_ms) >= 10){
    prev_loop_ms = now_loop_ms;
    // 10Hz
    if (++i == 10) {//wifi接続のとき青い光がチカチカしてる
      digitalWrite(Led_Pin, HIGH);
    }else if (i == 20) {
      digitalWrite(Led_Pin, LOW);
      i = 0;
    }
    if(gamepad.receive){
          // 10/06 テスト用下三行
      // motor1.setSpeed(gamepad.joy(RY));//モーターの正方向を確認　右のモーター
      // motor2.setSpeed(gamepad.joy(LY));//左のモーター
      // motor3.setSpeed(gamepad.joy(LX));//真ん中のモーター
      float yaw = im.getYaw();//ここコメントアウトした①

      // DiffDrive
      // L1,R1の旋回動作
      float vt = 0;
      if(gamepad.btn(L1)){
        vt = OMEGAMAX;
      }else if(gamepad.btn(R1)){
        vt = -OMEGAMAX;
      }

      aim_yaw += vt * 0.01;//100Hz

      // L2,R2の90度回転
      unsigned long nowtimestamp = millis();//現在時刻をミリ秒で取得
      if( (nowtimestamp-push_90_time_stamp) > 1000 ){//ボタンをおした瞬間だけみる
        if(gamepad.btn(L2)){
          aim_yaw += M_PI/2;
          push_90_time_stamp = nowtimestamp;
        }else if(gamepad.btn(R2)){
          aim_yaw += -M_PI/2;
          push_90_time_stamp = nowtimestamp;
        }
      }

      // y軸移動
      float vy = gamepad.joy(LY);
      if(gamepad.btn(U)){
        vy = VYMAX;
      }else if(gamepad.btn(D)){
        vy = -VYMAX;
      }

      //x軸移動
      float vx = gamepad.joy(LX);
      if(gamepad.btn(R)){
        vx = VXMAX;
      }else if(gamepad.btn(L)){
        vx = -VXMAX;
      }

      //機構 ボールを転がす
      float ball = 0;//ジョイスティックで動かすときは　float ball = gamepad.joy(RY)
      if(gamepad.btn(Y)){
        ball = BALLMAX;
      }else if(gamepad.btn(A)){
        ball = -BALLMAX;
      }

      //機構２
      float ball2 = 0; //ジョイスティックで動かすときは　float ball = gamepad.joy(RX)
      if(gamepad.btn(B)){
        ball2 = BALL2MAX;
      }else if(gamepad.btn(X)){
        ball2 = -BALL2MAX;
      }

      vt = pid_yaw.update(normalizeAngle(aim_yaw-yaw));//コメントアウトした②
      omni.setRobotSpeed(vx,vy,vt,0);//テスト中にコメントアウト③
      motor4.setSpeed(ball); 
      motor5.setSpeed(ball2);     
    }
  }
}
void test_loop(){
  Serial.println(im.getYaw());//ジャイロセンサの角度
  delay(100);
  
}