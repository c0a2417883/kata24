#pragma once

#include <cmath>
#include "Motor.h"


class DiffDrive {
    float wheel_separation_;      // ロボット中心からタイヤまでの距離
    Motor *motor1_, *motor2_ , *motor3_;

   public:
    DiffDrive(float wheel_separation, Motor *motor1, Motor *motor2, Motor *motor3)
        : wheel_separation_(wheel_separation),
          motor1_(motor1),
          motor2_(motor2),
          motor3_(motor3)
        {
        }
    // vt→ω(角速度) vx,vy→グローバル座標の速度ベクトル t→yaw角（単位はrad）
    void setRobotSpeed(float vx, float vy, float vt, float t) {//tを０しておけばローカル座標になる
        // 回転行列
        float c = cosf(-t);
        float s = sinf(-t);
        float vx_local = vx * c - vy * s;
        float vy_local = vx * s + vy * c;
        
        float v1 = -vy_local-(wheel_separation_/2)*vt;
        float v2 = vy_local-(wheel_separation_/2)*vt;
        float v3 = vx_local;

        motor1_->setSpeed(v1);
        motor2_->setSpeed(v2);
        motor3_->setSpeed(v3);
    }
};
