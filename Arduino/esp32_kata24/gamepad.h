#pragma once

#define A (0)
#define B (1)
#define X (2)
#define Y (3)
#define L1 (4)//左人差し指
#define R1 (5)
#define L2 (6)//左中指
#define R2 (7)
#define VIEW (8)//viewボタン
#define MENU (9)//メニューボタン
#define L3 (10)//左ジョイスティック押しボタン
#define R3 (11)//右ジョイスティック押しボタン
#define U (12)//左の十字キー　上
#define D (13)//左の十字キー下
#define L (14)//左の十字キー　左
#define R (15)//左の十字キー　右
#define LX (1)//ジョイスティックｘ座標
#define LY (2)
#define RX (3)
#define RY (4)

class GamePad{
  uint16_t data_[5];//16ビットの配列が五個
  public:
   GamePad(){

   }
   bool receive = false;
   void update(uint8_t *data){
      memcpy((uint8_t *)data_, data, 10);//10バイトのデータを16ビットから8ビットにする
      receive = true;
   }
   bool btn(int key){
    bool result = false;
    if(key >= 0 && key < 16){
      result = bitRead(data_[0], key);//data_[0]の16ビットの右端から数えてkeyビット目の値を読み取る
    }
    return result;
   }
   float joy(int key){
    float result = 0;
    if(key >= 1 && key < 5){
      result = (float)((int16_t)data_[key]) / 512.f;//data_[1]からdata_[4]のそれぞれの16ビットの値(スマホで512をかけた値)を512.0で割って-1から1にして小数にしてる
    }
    return result;
   }
};
