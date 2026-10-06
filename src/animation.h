#pragma once
#include "math3d.h"
#include <array>

namespace cg {
// Seconds of active simulation. GLFW supplies actual elapsed time, not frame count.
constexpr std::array<double,13> STAGE_TIME{{0,6,18,22,26,34,36,50,54,57,60,65,70}};
constexpr float CAR_START=70, DEMO_END=77, WHEEL_RADIUS=.68f;
constexpr float CRANK_RADIUS=.65f, ROD_LENGTH=2.15f, CAR_WHEEL_RADIUS=.43f;
constexpr float CRANK_INITIAL=150.0f*PI/180.0f;
constexpr Vec3 CRANK_CENTER{3.55f,1.55f,.1f};
inline float phase(float t,float start,float duration){return clamp01((t-start)/duration);}
inline Vec3 crankPin(float a){return CRANK_CENTER+Vec3{CRANK_RADIUS*std::cos(a),CRANK_RADIUS*std::sin(a),0};}
inline Vec3 sliderPin(float a){float y=CRANK_RADIUS*std::sin(a);
    return {CRANK_CENTER.x+CRANK_RADIUS*std::cos(a)+std::sqrt(ROD_LENGTH*ROD_LENGTH-y*y),CRANK_CENTER.y,.1f};}
inline int stageAt(double t){int s=0;while(s<12&&t>=STAGE_TIME[s+1])++s;return s;}
inline const char* stageName(int s){
    static const char* names[]={"GLOVE / INSERT STARTER PIN","GEARS / WIND SPRING","CAM / LOWER STOP LEVER",
        "WHEEL / ROLL TO TRIGGER","WIPER / THREE-PASS INDEX","BAR / ENGAGE PUMP",
        "CRANK / ADVANCE RATCHET RACK","BOLT / UNLOCK CATCH","CATCH / RELEASE WEIGHT",
        "COUNTERWEIGHT / RAISE BARRIER","DRAWBRIDGE / LOWER DECK","STARTER / CRANK ENGINE","CAR / ARROWS TO DRIVE"};
    return names[std::clamp(s,0,12)];
}
struct Pose {
    float gearSmall=0,gearLarge=0,cam=0,lever=0,wheelTravel=0,wheelSpin=0;
    Vec3 wheel{-5.65f,1.85f,0};
    float wiper=0,paddle=0,wiperIndex=0,bar=0,crank=CRANK_INITIAL,rack=0,bolt=0;
    float catchAngle=0,weightDrop=0,pulleySpin=0,gateLift=0,bridge=radians(75),starter=0,coupling=0;
    Vec3 hand{-10.8f,1.8f,1.2f},pin{-10.8f,.48f,1.2f};float grip=0;bool attached=false;
    Vec3 carPosition{9,.03f,.4f};float carYaw=-PI/2,carTravel=0,carSpin=0,steer=0;
};
inline Pose poseAt(float t){
    Pose p;
    // Attach one designated component at a fixed hand-local point during transport.
    Vec3 tray{-10.8f,.48f,1.2f},socket{-10,2.68f,.82f},offset{0,-.4f,0};
    if(t<1.5f)p.hand=tray-offset+Vec3{0,.92f*(1-smooth(t/1.5f)),0};
    else if(t<4.5f)p.hand=tray-offset+(socket-tray)*smooth((t-1.5f)/3);
    else p.hand=socket-offset+Vec3{-.8f*smooth((t-4.5f)/1.5f),.6f*smooth((t-4.5f)/1.5f),0};
    p.attached=t>=1.5f&&t<4.5f;p.pin=t<1.5f?tray:t<4.5f?p.hand+offset:socket;
    p.grip=smooth(phase(t,1,.5f))*(1-smooth(phase(t,4.5f,.5f)));
    // Four winding revolutions, followed by a separate output-cam clutch.
    p.cam=radians(150)*smooth(phase(t,18,4));
    p.gearLarge=8*PI*phase(t,6,12)+p.cam;p.gearSmall=-1.5f*p.gearLarge;
    p.lever=radians(-32)*smooth(phase(t,20.4f,1.6f));
    p.paddle=smooth(phase(t,26,1.5f));
    float u=phase(t,26,8)*3;
    if(u<2)p.wiper=-radians(45)*std::pow(std::sin(PI*(u-std::floor(u))),2);
    else p.wiper=-radians(65)*smooth(u-2);
    p.wiperIndex=std::min(u,3.f);
    float travel=(3.87f*smooth(phase(t,22,4))+.30f*p.paddle)/4.17f;
    Vec3 start{-5.65f,1.85f,0},end{-1.48f,.908f,0};
    p.wheel=start+(end-start)*travel;p.wheelTravel=length(end-start)*travel;p.wheelSpin=-p.wheelTravel/WHEEL_RADIUS;
    p.bar=radians(-84)*smooth(phase(t,34,2));
    p.crank=CRANK_INITIAL-8*PI*phase(t,36,12)-CRANK_INITIAL*smooth(phase(t,48,2));
    // A one-way pawl advances the rack once per compression stroke.
    for(int i=0;i<4;++i)p.rack+=.25f*smooth(phase(t,36.65f+3*i,.85f));
    p.bolt=.7f*smooth(phase(t,50,4));
    p.catchAngle=radians(-88)*smooth(phase(t,54,3));
    p.weightDrop=1.4f*smooth(phase(t,57,3));p.gateLift=p.weightDrop;p.pulleySpin=p.weightDrop/.44f;
    p.bridge=radians(75)*(1-smooth(phase(t,60,5)));
    p.starter=16*PI*phase(t,65,4);p.coupling=.65f*smooth(phase(t,69,1));
    p.carTravel=9*smooth(phase(t,CAR_START,7));p.carSpin=-p.carTravel/CAR_WHEEL_RADIUS;
    p.carPosition.z+=p.carTravel;
    return p;
}
struct Simulation {
    double time=0;bool running=false,interactive=false,waiting=false;
    void reset(){time=0;running=false;waiting=interactive;}
    bool activate(int requested=-1){int s=stageAt(time);
        if(!interactive||!waiting||s>=12||(requested>=0&&requested!=s))return false;
        waiting=false;running=true;return true;}
    void update(double dt){
        if(!running||waiting||dt<=0)return;
        int s=stageAt(time);double end=interactive&&s<12?STAGE_TIME[s+1]:double(DEMO_END);
        time=std::min(end,time+dt);
        if(interactive&&s<12&&time>=end){waiting=stageAt(time)<12;running=!waiting;}
    }
};
// Independent position + yaw/pitch: this is a free camera, not a fixed orbit.
struct Camera {
    Vec3 eye;float yaw=0,pitch=0;int presetId=1;
    Camera(){preset(1);}
    Vec3 forward()const{return {std::sin(yaw)*std::cos(pitch),std::sin(pitch),-std::cos(yaw)*std::cos(pitch)};}
    Vec3 right()const{return normalize(cross(forward(),{0,1,0}));}
    void look(float dx,float dy){yaw+=dx*.004f;pitch=std::clamp(pitch-dy*.004f,-1.52f,1.52f);presetId=0;}
    void move(float f,float r,float up,float dt,bool fast){Vec3 v=forward()*f+right()*r+Vec3{0,up,0};
        if(length(v)>0){eye=eye+normalize(v)*(fast?10.f:4.f)*dt;presetId=0;}}
    void preset(int n){presetId=n;Vec3 target;float a,b,d;
        if(n==2){a=.48f;b=.45f;d=15.5f;target={4.8f,1.45f,1.1f};}
        else if(n==3){a=-.22f;b=.3f;d=12;target={-7,2,0};}
        else if(n==4){a=.7f;b=.65f;d=20;target={9,.8f,6};}
        else {a=.28f;b=.57f;d=24.5f;target={.2f,.9f,2};}
        eye=target+Vec3{std::sin(a)*std::cos(b),std::sin(b),std::cos(a)*std::cos(b)}*d;
        Vec3 f=normalize(target-eye);yaw=std::atan2(f.x,-f.z);pitch=std::asin(f.y);
    }
};
struct CarState {
    Vec3 position{9,.03f,.4f};float yaw=-PI/2,speed=0,steer=0,travel=0;bool manual=false;
    void reset(){*this=CarState{};}
    bool allowed(Vec3 p,float angle)const{
        // Keep all four chassis corners on the bay/bridge or driving apron.
        for(float x:{-1.82f,1.82f})for(float z:{-.92f,.92f}){
            Vec3 q=p+Vec3{std::cos(angle)*x+std::sin(angle)*z,0,-std::sin(angle)*x+std::cos(angle)*z};
            bool bay=q.x>=7.5f&&q.x<=10.5f&&q.z>=-1.5f&&q.z<=6;
            bool apron=q.x>=2&&q.x<=16&&q.z>=5.2f&&q.z<=17;
            if(!bay&&!apron)return false;
        }return true;
    }
    void update(double dt,const Simulation& sim,float throttle,float turn){
        if(sim.time<CAR_START||!sim.running)return;
        if(throttle!=0||turn!=0)manual=true;
        if(!manual&&!sim.interactive){auto p=poseAt(float(sim.time));position=p.carPosition;travel=p.carTravel;return;}
        manual=true;
        // Substeps keep steering and edge checks stable after a slow frame.
        int steps=std::max(1,int(std::ceil(dt/.016)));float h=float(dt/steps);
        for(int i=0;i<steps;++i){
            steer=lerp(steer,turn*.48f,std::min(1.f,h*8));
            if(throttle!=0)speed=std::clamp(speed+throttle*h*3,-2.f,3.5f);
            else speed=std::copysign(std::max(0.f,std::abs(speed)-h*2.5f),speed);
            float ds=speed*h,angle=yaw+ds*std::tan(steer)/2.1f;
            Vec3 next=position+Vec3{std::cos(angle),0,-std::sin(angle)}*ds;
            if(allowed(next,angle)){position=next;yaw=angle;travel+=ds;}else speed=0;
        }
    }
    void apply(Pose& p)const{p.carPosition=position;p.carYaw=yaw;p.carSpin=-travel/CAR_WHEEL_RADIUS;p.carTravel=travel;p.steer=steer;}
};
}
