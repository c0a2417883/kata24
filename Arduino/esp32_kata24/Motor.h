#pragma once

class Motor{
  int pin1_;
  int pin2_;
  public:
    Motor(int pin1,int pin2)
      : pin1_(pin1),pin2_(pin2)
    {
    }
    void init(){
      ledcAttach(pin1_,20000,8);//pin, 周波数, pwm分解能
      ledcAttach(pin2_,20000,8);
      setSpeed(0);
    }

    void setSpeed(float speed){
      speed = constrain(speed,-1.f,1.f);
      if(speed > 0){
        ledcWrite(pin1_,speed*255);
        ledcWrite(pin2_,0);
      }else{
        ledcWrite(pin1_,0);
        ledcWrite(pin2_,-speed*255);
      }
    }

};