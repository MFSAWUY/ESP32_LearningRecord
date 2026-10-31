#include <Arduino.h>
// 明确标记为仿真对象：不控制实际电机，不把模型输出称为测量。
float integral=0,previousY=0,y=0;
constexpr float dt=0.02f,Kp=1.2f,Ki=0.8f,Kd=0.05f,target=1.0f;
float clamp(float x,float lo,float hi){return x<lo?lo:(x>hi?hi:x);}
void setup(){Serial.begin(115200);Serial.println("simulation_time_s,target,model_y,control_u");}
void loop(){
  static uint32_t last=0,step=0;if(millis()-last<20){delay(1);return;}last=millis();
  float error=target-y,candidate=integral+error*dt;
  float derivative=-(y-previousY)/dt;
  float proposed=Kp*error+Ki*candidate+Kd*derivative;
  // 饱和且误差继续推向饱和端时停止积分，反向误差允许积分恢复。
  if((proposed>=0&&proposed<=2)||(proposed>2&&error<0)||(proposed<0&&error>0))integral=candidate;
  float u=clamp(Kp*error+Ki*integral+Kd*derivative,0,2);
  previousY=y;y+=dt*(u-y)/0.5f; // 一阶模型 dy/dt=(u-y)/tau
  Serial.printf("%.2f,%.2f,%.4f,%.4f\n",step++*dt,target,y,u);
}
