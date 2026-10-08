#pragma once
#include "math3d.h"
#include <array>

namespace cg {
// Seconds of active simulation. GLFW supplies actual elapsed time, not frame count.
constexpr int ACTION_COUNT=17;
constexpr std::array<double,ACTION_COUNT+1> STAGE_TIME{{0,6,10,14,18,21,24,29,34,38,42,46,50,53,56,60,65,70}};
constexpr float CAR_START=70, DEMO_END=77, WHEEL_RADIUS=.68f;
constexpr float CRANK_RADIUS=.65f, ROD_LENGTH=2.15f, CAR_WHEEL_RADIUS=.43f;
constexpr float CRANK_INITIAL=150.0f*PI/180.0f;
constexpr Vec3 CRANK_CENTER{3.55f,1.55f,.1f};
constexpr float BALL_RADIUS=.27f, PENDULUM_LENGTH=1.65f;
constexpr Vec3 BALL_START{5.55f,2.25f,3.15f}, BALL_END{-2.6f,1.17f,3.15f};
constexpr Vec3 CUP_PIVOT{-2.10f,.80f,3.15f}, PENDULUM_PIVOT{-4.30f,2.65f,3.15f};
constexpr Vec3 ROCKER_PIVOT{-6.8f,1.85f,3.15f}, RELEASE_PIN{-7.815f,1.86f,3.70f};
inline Mat4 cupMatrix(float angle){return translate(CUP_PIVOT)*rotateZ(angle);}
inline Vec3 pendulumBob(float angle){return transform(translate(PENDULUM_PIVOT)*rotateZ(angle),{0,-PENDULUM_LENGTH,0});}
constexpr float RACK_LINK_LENGTH=1.4f;
inline Vec3 rackFollower(float rack){float dx=.4f+.95f*rack;return {5.9f,2.25f+std::sqrt(RACK_LINK_LENGTH*RACK_LINK_LENGTH-dx*dx),.78f};}
inline Vec3 rockerTip(float angle){return transform(translate(ROCKER_PIVOT)*rotateZ(angle),{-1.12f,0,.55f});}
inline float phase(float t,float start,float duration){return clamp01((t-start)/duration);}
inline Vec3 crankPin(float a){return CRANK_CENTER+Vec3{CRANK_RADIUS*std::cos(a),CRANK_RADIUS*std::sin(a),0};}
inline Vec3 sliderPin(float a){float y=CRANK_RADIUS*std::sin(a);
    return {CRANK_CENTER.x+CRANK_RADIUS*std::cos(a)+std::sqrt(ROD_LENGTH*ROD_LENGTH-y*y),CRANK_CENTER.y,.1f};}
inline int stageAt(double t){int s=0;while(s<ACTION_COUNT&&t>=STAGE_TIME[s+1])++s;return s;}
inline const char* stageName(int s){
    static const char* names[]={"GLOVE / INSERT STARTER PIN","GEARS / WIND SPRING ONCE","CAM / LOWER STOP LEVER",
        "WHEEL / ROLL TO TRIGGER","WIPER / SWEEP TO BAR","BAR / ENGAGE PUMP",
        "CRANK / LIFT BALL STOP","BALL / ROLL INTO CUP","CUP / UNLATCH STRIKER",
        "PENDULUM / STRIKE ROCKER","ROCKER / PRESS PIN COLLAR","PIN / FREE CABLE DRUM",
        "CABLE DRUM / WITHDRAW BOLT","CATCH / RELEASE WEIGHT",
        "COUNTERWEIGHT / RAISE BARRIER","DRAWBRIDGE / LOWER DECK","STARTER / CRANK ENGINE","CAR / ARROWS TO DRIVE"};
    return names[std::clamp(s,0,ACTION_COUNT)];
}
enum class Operation { Release, Hold, QuarterTurn };
struct StageControl {int key;Operation operation;const char* instruction;};
// One source for the input handlers, live prompt, guide and validation.
inline const StageControl& stageControl(int stage){
    static const StageControl controls[]={
        {'G',Operation::Hold,"HOLD G / GRIP AND CARRY THE STARTER PIN"},
        {'Z',Operation::QuarterTurn,"TAP Z / WIND ONE QUARTER TURN / FOUR TAPS"},
        {'X',Operation::Hold,"HOLD X / ROTATE CAM AND LOWER WHEEL STOP"},
        {'F',Operation::Release,"F / RELEASE THE WHEEL DOWN ITS RAMP"},
        {'V',Operation::Hold,"HOLD V / SWEEP WIPER INTO THE CLUTCH BAR"},
        {'F',Operation::Release,"F / TIP THE BAR ONTO THE PUMP CLUTCH"},
        {'P',Operation::Hold,"HOLD P / TURN CRANK THROUGH COMPRESSION"},
        {'F',Operation::Release,"F / LET THE BALL ROLL INTO THE CUP"},
        {'U',Operation::Hold,"HOLD U / TIP LOADED CUP TO UNLATCH STRIKER"},
        {'J',Operation::Release,"J OR F / RELEASE THE PENDULUM STRIKER"},
        {'K',Operation::Hold,"HOLD K / ROCK THE ARM INTO THE PIN COLLAR"},
        {'O',Operation::Hold,"HOLD O / LIFT PIN CLEAR OF THE CABLE DRUM"},
        {'F',Operation::Release,"F / WIND CABLE DRUM AND PULL CATCH BOLT"},
        {'F',Operation::Release,"F / SWING CATCH CLEAR OF COUNTERWEIGHT"},
        {'F',Operation::Release,"F / LOWER WEIGHT AND RAISE EXIT BARRIER"},
        {'F',Operation::Release,"F / LOWER DRAWBRIDGE TO THE EXIT APRON"},
        {'F',Operation::Release,"F / TURN STARTER AND RETRACT ITS COUPLING"}
    };
    return controls[std::clamp(stage,0,ACTION_COUNT-1)];
}
struct Pose {
    float gearSmall=0,gearLarge=0,cam=0,lever=0,wheelTravel=0,wheelSpin=0;
    Vec3 wheel{-5.65f,1.85f,0};
    float wiper=0,paddle=0,wiperIndex=0,bar=0,crank=CRANK_INITIAL,rack=0,bolt=0;
    float ballGate=0,ballTravel=0,ballSpin=0,cup=0,pendulum=radians(45),rocker=0,releasePin=0,releaseDrum=0;
    Vec3 relayBall=BALL_START;
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
    // One winding turn, one wiper sweep and one compression stroke.
    p.cam=radians(150)*smooth(phase(t,10,4));
    p.gearLarge=2*PI*phase(t,6,4)+p.cam;p.gearSmall=-1.5f*p.gearLarge;
    p.lever=radians(-32)*smooth(phase(t,12.4f,1.6f));
    p.paddle=smooth(phase(t,17.2f,.8f));
    p.wiper=-radians(65)*smooth(phase(t,18,3));
    p.wiperIndex=smooth(phase(t,18,3));
    float travel=(3.87f*smooth(phase(t,14,3.2f))+.30f*p.paddle)/4.17f;
    Vec3 start{-5.65f,1.85f,0},end{-1.48f,.908f,0};
    p.wheel=start+(end-start)*travel;p.wheelTravel=length(end-start)*travel;p.wheelSpin=-p.wheelTravel/WHEEL_RADIUS;
    p.bar=radians(-84)*smooth(phase(t,21,3));
    p.crank=CRANK_INITIAL*(1-smooth(phase(t,24,5)));
    p.rack=(sliderPin(p.crank).x-sliderPin(CRANK_INITIAL).x)/(sliderPin(0).x-sliderPin(CRANK_INITIAL).x);
    p.ballGate=rackFollower(0).y-rackFollower(p.rack).y;
    float roll=smooth(phase(t,29,5));
    p.relayBall=BALL_START+(BALL_END-BALL_START)*roll;
    p.ballTravel=length(BALL_END-BALL_START)*roll;p.ballSpin=p.ballTravel/BALL_RADIUS;
    p.cup=radians(32)*smooth(phase(t,34,4));
    if(t>=34)p.relayBall=transform(cupMatrix(p.cup),BALL_END-CUP_PIVOT);
    p.pendulum=lerp(radians(45),radians(-45),smooth(phase(t,38,4)));
    p.rocker=-radians(25)*smooth(phase(t,42,4));
    p.releasePin=.65f*smooth(phase(t,46,4));
    p.bolt=.7f*smooth(phase(t,50,3));p.releaseDrum=-p.bolt/.42f;
    p.catchAngle=radians(-88)*smooth(phase(t,53,3));
    p.weightDrop=1.4f*smooth(phase(t,56,4));p.gateLift=p.weightDrop;p.pulleySpin=p.weightDrop/.44f;
    p.bridge=radians(75)*(1-smooth(phase(t,60,5)));
    p.starter=2*PI*smooth(phase(t,65,3));p.coupling=.65f*smooth(phase(t,68,2));
    p.carTravel=9*smooth(phase(t,CAR_START,7));p.carSpin=-p.carTravel/CAR_WHEEL_RADIUS;
    p.carPosition.z+=p.carTravel;
    return p;
}
struct Simulation {
    double time=0;bool running=false,interactive=false,waiting=false;
    double turnTarget=0;
    void reset(){time=turnTarget=0;running=false;waiting=interactive;}
    bool operate(int key){int s=stageAt(time);
        if(!interactive||s>=ACTION_COUNT)return false;
        const auto& control=stageControl(s);
        if(key!=control.key&&!(key=='F'&&control.operation==Operation::Release))return false;
        if(control.operation==Operation::QuarterTurn){
            // A tap authorizes one 90-degree gear turn; held key repeats do not.
            turnTarget=std::min(STAGE_TIME[s+1],std::max(time,turnTarget)+(STAGE_TIME[s+1]-STAGE_TIME[s])*.25);
        }else if(!waiting)return false;
        waiting=false;running=true;return true;
    }
    bool activate(){return operate('F');}
    void update(double dt,int heldKey=0){
        if(!running||waiting||dt<=0)return;
        int s=stageAt(time);double end=interactive&&s<ACTION_COUNT?STAGE_TIME[s+1]:double(DEMO_END);
        if(interactive&&s<ACTION_COUNT){const auto& control=stageControl(s);
            if(control.operation==Operation::Hold&&heldKey!=control.key)return;
            if(control.operation==Operation::QuarterTurn)end=std::min(end,turnTarget);
        }
        time=std::min(end,time+dt);
        if(interactive&&s<ACTION_COUNT&&time>=end){waiting=stageAt(time)<ACTION_COUNT;running=!waiting;}
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
        if(n==2){a=.30f;b=.42f;d=12.5f;target={5.1f,1.45f,1.1f};}
        else if(n==3){a=-.22f;b=.3f;d=12;target={-7,2,0};}
        else if(n==4){a=.7f;b=.65f;d=20;target={9,.8f,6};}
        else if(n==6){a=-.10f;b=.36f;d=8.8f;target={-5.2f,1.55f,3.15f};}
        else if(n==5){a=.04f;b=.42f;d=16;target={-1.4f,1.30f,3.1f};}
        else {a=.22f;b=.70f;d=22.4f;target={-.35f,.80f,1.0f};}
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
