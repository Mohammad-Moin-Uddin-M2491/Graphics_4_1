#pragma once
#include "math3d.h"
#include <array>

namespace cg {
// Seconds of active simulation. GLFW supplies actual elapsed time, not frame count.
constexpr int ACTION_COUNT=17;
enum Stage {Glove,Gears,Cam,Wheel,Wiper,Bar,Crank,Ball,Cup,Pendulum,Rocker,Pin,Drum,Catch,Weight,Bridge,Starter,CarExit};
constexpr std::array<double,ACTION_COUNT+1> STAGE_TIME{{0,6,10,14,18,21,24,29,36,39,43,45,47,50,53,58,65,70}};
constexpr float CAR_START=float(STAGE_TIME.back()), CAR_EXIT_DURATION=7, DEMO_END=CAR_START+CAR_EXIT_DURATION, WHEEL_RADIUS=.68f;
constexpr double stageDuration(int s){return STAGE_TIME[s+1]-STAGE_TIME[s];}
inline float stageProgress(float t,int s){return clamp01(float((t-STAGE_TIME[s])/stageDuration(s)));}
// Fractions within a stage, never a second independently maintained timeline.
inline float part(float progress,float begin,float end){return clamp01((progress-begin)/(end-begin));}
constexpr float CRANK_RADIUS=.65f, ROD_LENGTH=2.15f, CAR_WHEEL_RADIUS=.43f;
constexpr float CRANK_INITIAL=150.0f*PI/180.0f;
constexpr Vec3 CRANK_CENTER{3.55f,1.55f,.1f};
constexpr float BALL_RADIUS=.27f, PENDULUM_LENGTH=1.65f;
constexpr Vec3 BALL_START{5.55f,2.25f,3.15f}, BALL_END{-2.6f,1.17f,3.15f};
constexpr Vec3 CUP_PIVOT{-2.10f,.80f,3.15f}, PENDULUM_PIVOT{-4.30f,2.65f,3.15f};
constexpr Vec3 ROCKER_PIVOT{-6.8f,1.85f,3.15f}, RELEASE_PIN{-7.815f,1.86f,3.70f};
constexpr Vec3 CUP_FINGER{-.79f,.36f,0}, CUP_FINGER_SIZE{.20f,.18f,.80f};
constexpr Vec3 ROCKER_PAD{.91f,-.44f,0}, ROCKER_PAD_SIZE{.24f,.88f,.64f};
constexpr float STRIKER_RADIUS=.30f, ROCKER_ROLLER_RADIUS=.14f, COLLAR_Y=.675f;
constexpr Vec3 COLLAR_SIZE{.62f,.14f,.46f};
constexpr float PIN_LENGTH=1.30f, DRUM_RADIUS=.42f;
constexpr Vec3 DRUM_CENTER{RELEASE_PIN.x,1,3.70f}, CABLE_BEND_XY{6.75f,.76f,3.70f}, CABLE_BEND_YZ{6.93f,1.57f,3.52f};
constexpr float CABLE_GUIDE_RADIUS=.18f;
inline Vec3 boltCablePoint(float bolt){return {6.93f,1.75f,.575f+bolt};}
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
        {'F',Operation::Release,"F / TIP LOADED CUP TO UNLATCH STRIKER"},
        {'F',Operation::Release,"F / CONTINUE THE STRIKER ARC TO THE ROCKER"},
        {'F',Operation::Release,"F / ROCK THE ARM INTO THE PIN COLLAR"},
        {'F',Operation::Release,"F / LIFT PIN CLEAR OF THE CABLE DRUM"},
        {'F',Operation::Release,"F / WIND CABLE DRUM AND PULL CATCH BOLT"},
        {'F',Operation::Release,"F / SWING CATCH CLEAR OF COUNTERWEIGHT"},
        {'F',Operation::Release,"F / LOWER WEIGHT AND RAISE EXIT BARRIER"},
        {'F',Operation::Release,"F / LOWER DRAWBRIDGE TO THE EXIT APRON"},
        {'F',Operation::Release,"F / TURN STARTER AND RETRACT ITS COUPLING"}
    };
    return controls[std::clamp(stage,0,ACTION_COUNT-1)];
}
inline Vec3 localZ(Vec3 point,Vec3 pivot,float angle){return transform(rotateZ(-angle),point-pivot);}
inline float sphereBoxGap(Vec3 center,float radius,Vec3 boxCenter,Vec3 size){
    Vec3 d=center-boxCenter;
    return length({std::max(0.f,std::abs(d.x)-size.x*.5f),std::max(0.f,std::abs(d.y)-size.y*.5f),
        std::max(0.f,std::abs(d.z)-size.z*.5f)})-radius;
}
// Measure the stop against the resting bob, even after the bob has swung away.
inline float cupStopGap(float cupAngle){return sphereBoxGap(localZ(pendulumBob(radians(45)),CUP_PIVOT,cupAngle),STRIKER_RADIUS,CUP_FINGER,CUP_FINGER_SIZE);}
inline float pendulumReleaseTime(){
    // Find the first small positive clearance using the actual stop dimensions.
    // The cup can finish tipping while its newly released striker is moving.
    static const float fraction=[](){float lo=0,hi=1;
        for(int i=0;i<24;++i){float mid=(lo+hi)*.5f;
            if(cupStopGap(radians(32)*smooth(mid))<.005f)lo=mid;else hi=mid;
        }return hi;
    }();
    return float(STAGE_TIME[Cup]+stageDuration(Cup)*fraction);
}
inline float pendulumProgress(float t){
    float release=pendulumReleaseTime();
    float u=phase(t,release,float(STAGE_TIME[Rocker])-release);
    // Short easing removes the apparent pause at release. Position and speed
    // stay continuous both at release and when the striker reaches the rocker.
    constexpr float ease=.06f;
    if(u<ease)return u*u/(2*ease*(1-ease));
    if(u>1-ease)return 1-(1-u)*(1-u)/(2*ease*(1-ease));
    return (u-ease*.5f)/(1-ease);
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
    float glove=stageProgress(t,Glove);
    if(glove<.25f)p.hand=tray-offset+Vec3{0,.92f*(1-smooth(part(glove,0,.25f))),0};
    else if(glove<.75f)p.hand=tray-offset+(socket-tray)*smooth(part(glove,.25f,.75f));
    else p.hand=socket-offset+Vec3{-.8f,.6f,0}*smooth(part(glove,.75f,1));
    p.attached=glove>=.25f&&glove<.75f;p.pin=glove<.25f?tray:glove<.75f?p.hand+offset:socket;
    p.grip=smooth(part(glove,1.f/6,.25f))*(1-smooth(part(glove,.75f,5.f/6)));
    // One winding turn, one wiper sweep and one compression stroke.
    p.cam=radians(150)*smooth(stageProgress(t,Cam));
    p.gearLarge=2*PI*stageProgress(t,Gears)+p.cam;p.gearSmall=-1.5f*p.gearLarge;
    p.lever=radians(-32)*smooth(part(stageProgress(t,Cam),.6f,1));
    p.paddle=smooth(part(stageProgress(t,Wheel),.8f,1));
    p.wiper=-radians(65)*smooth(stageProgress(t,Wiper));
    p.wiperIndex=smooth(stageProgress(t,Wiper));
    float travel=(3.87f*smooth(part(stageProgress(t,Wheel),0,.8f))+.30f*p.paddle)/4.17f;
    Vec3 start{-5.65f,1.85f,0},end{-1.48f,.908f,0};
    p.wheel=start+(end-start)*travel;p.wheelTravel=length(end-start)*travel;p.wheelSpin=-p.wheelTravel/WHEEL_RADIUS;
    p.bar=radians(-84)*smooth(stageProgress(t,Bar));
    p.crank=CRANK_INITIAL*(1-smooth(stageProgress(t,Crank)));
    p.rack=(sliderPin(p.crank).x-sliderPin(CRANK_INITIAL).x)/(sliderPin(0).x-sliderPin(CRANK_INITIAL).x);
    p.ballGate=rackFollower(0).y-rackFollower(p.rack).y;
    float roll=smooth(stageProgress(t,Ball));
    p.relayBall=BALL_START+(BALL_END-BALL_START)*roll;
    p.ballTravel=length(BALL_END-BALL_START)*roll;p.ballSpin=p.ballTravel/BALL_RADIUS;
    p.cup=radians(32)*smooth(stageProgress(t,Cup));
    if(t>=STAGE_TIME[Cup])p.relayBall=transform(cupMatrix(p.cup),BALL_END-CUP_PIVOT);
    p.pendulum=lerp(radians(45),radians(-45),pendulumProgress(t));
    p.rocker=-radians(25)*smooth(stageProgress(t,Rocker));
    p.releasePin=.65f*smooth(stageProgress(t,Pin));
    p.bolt=.7f*smooth(stageProgress(t,Drum));p.releaseDrum=-p.bolt/DRUM_RADIUS;
    p.catchAngle=radians(-88)*smooth(stageProgress(t,Catch));
    p.weightDrop=1.4f*smooth(stageProgress(t,Weight));p.gateLift=p.weightDrop;p.pulleySpin=p.weightDrop/.44f;
    p.bridge=radians(75)*(1-smooth(stageProgress(t,Bridge)));
    p.starter=2*PI*smooth(part(stageProgress(t,Starter),0,.6f));p.coupling=.65f*smooth(part(stageProgress(t,Starter),.6f,1));
    p.carTravel=9*smooth(phase(t,CAR_START,CAR_EXIT_DURATION));p.carSpin=-p.carTravel/CAR_WHEEL_RADIUS;
    p.carPosition.z+=p.carTravel;
    return p;
}
inline float cupFingerGap(const Pose& p){return sphereBoxGap(localZ(pendulumBob(p.pendulum),CUP_PIVOT,p.cup),STRIKER_RADIUS,CUP_FINGER,CUP_FINGER_SIZE);}
inline float strikerPadGap(const Pose& p){return sphereBoxGap(localZ(pendulumBob(p.pendulum),ROCKER_PIVOT,p.rocker),STRIKER_RADIUS,ROCKER_PAD,ROCKER_PAD_SIZE);}
inline float rollerCollarGap(const Pose& p){return sphereBoxGap(rockerTip(p.rocker),ROCKER_ROLLER_RADIUS,RELEASE_PIN+Vec3{0,COLLAR_Y+p.releasePin,0},COLLAR_SIZE);}
inline float pinDrumGap(const Pose& p){return RELEASE_PIN.y+p.releasePin-PIN_LENGTH*.5f-(DRUM_CENTER.y+DRUM_RADIUS);}
inline float roofClearance(const Pose& p){return .36f+p.gateLift-.08f-(.03f+1.51f+.065f*.5f);}
inline Vec3 carSocket(const Pose& p){return transform(translate(p.carPosition)*rotateY(p.carYaw),{.85f,.8f,-.82f});}
inline Vec3 starterTip(const Pose& p){return {9.88f+p.coupling,.83f,1.25f};}
inline Vec3 starterShoe(const Pose& p){return {9.77f+p.coupling,.25f,1.92f};}
inline Vec3 bridgeEnd(const Pose& p){return transform(translate({9,.03f,3})*rotateX(-p.bridge),{0,0,2.2f});}
inline bool ballSeated(const Pose& p){
    Vec3 local=localZ(p.relayBall,CUP_PIVOT,p.cup);
    return std::abs(local.y-BALL_RADIUS-.10f)<.025f&&local.x>=-.93f+BALL_RADIUS-.025f&&
        local.x<=-.07f-BALL_RADIUS+.025f&&std::abs(local.z)<.36f-BALL_RADIUS+.025f;
}
// Clearance tests use the same dimensions and transforms as Scene. They gate
// real boundary advancement, in both modes, rather than just testing the clock.
inline bool handoffReady(int next,const Pose& p){
    switch(next){
    case Gears:return length(p.pin-Vec3{-10,2.68f,.82f})<.02f&&!p.attached;
    case Cam:return p.gearLarge>=2*PI-.001f;
    case Wheel:return -p.lever>=radians(31.9f);
    case Wiper:return std::abs(p.wheel.x+WHEEL_RADIUS-(-1.10f+.3f*p.paddle))<.025f;
    case Bar:return sphereBoxGap(transform(translate({-.45f,.32f,.07f})*rotateZ(p.wiper),{0,2.155f,.04f}),.03f,{1.44f,1.305f,.1f},{.48f,.16f,.4f})<.02f;
    case Crank:return sphereBoxGap(transform(translate({1.6f,.075f,.1f})*rotateZ(p.bar),{0,1.96f,0}),.03f,{3.5f,.23f,.1f},{.4f,.1f,.45f})<.02f;
    case Ball:return 2.055f+p.ballGate-.475f>p.relayBall.y+BALL_RADIUS+.02f;
    case Cup:return ballSeated(p);
    case Pendulum:return ballSeated(p)&&cupStopGap(p.cup)>.12f;
    case Rocker:{float gap=strikerPadGap(p);return gap>=-.04f&&gap<=.025f;}
    case Pin:{float gap=rollerCollarGap(p);return gap>=-.025f&&gap<=.025f;}
    case Drum:return pinDrumGap(p)>.10f;
    case Catch:return .2f+p.bolt-.375f>.35f+.05f;
    case Weight:return (7.21f-.39f)-(6.7f+.85f*std::cos(p.catchAngle)+.05f*std::abs(std::sin(p.catchAngle)))>.025f;
    case Bridge:return roofClearance(p)>.05f&&p.weightDrop>1.39f;
    case Starter:{Vec3 socket=carSocket(p),tip=starterTip(p);float insertion=socket.x+.10f-tip.x;
        return length(bridgeEnd(p)-Vec3{9,.03f,5.2f})<.02f&&roofClearance(p)>.05f&&
            std::abs(tip.y-socket.y)<.01f&&std::abs(tip.z-socket.z)<.01f&&insertion>=0&&insertion<.10f;}
    case CarExit:return p.starter>=2*PI-.001f&&starterTip(p).x-(carSocket(p).x+.10f)>.5f&&
        starterShoe(p).x-.14f>9.88f+.20f&&roofClearance(p)>.05f&&std::abs(p.bridge)<.005f;
    default:return true;
    }
}
inline const char* handoffFailure(int next){
    static const char* messages[]={"", "STARTER PIN NOT SEATED", "GEAR SPRING NOT WOUND", "WHEEL STOP NOT CLEAR",
        "WHEEL HAS NOT REACHED PADDLE", "WIPER HAS NOT TOUCHED BAR", "BAR HAS NOT PRESSED CLUTCH",
        "BALL STOP STILL BLOCKS SPHERE", "BALL NOT SEATED IN CUP", "CUP FINGER STILL HOLDS STRIKER",
        "STRIKER HAS NOT REACHED PAD", "ROLLER HAS NOT REACHED COLLAR", "PIN STILL BLOCKS DRUM",
        "BOLT STILL BLOCKS CATCH", "CATCH STILL SUPPORTS WEIGHT", "BARRIER NOT CLEAR OF CAR",
        "BRIDGE OR STARTER SOCKET NOT READY", "STARTER OR RELEASE SHOE NOT CLEAR"};
    return messages[std::clamp(next,0,ACTION_COUNT)];
}
struct Simulation {
    double time=0;bool running=false,interactive=false,waiting=false;
    double turnTarget=0;int blockedStage=-1;
    void reset(){time=turnTarget=0;running=false;waiting=interactive;blockedStage=-1;}
    bool checkBoundary(int next,const Pose& observed){
        if(handoffReady(next,observed))return true;
        blockedStage=next;running=false;waiting=false;return false;
    }
    bool operate(int key){int s=stageAt(time);
        if(!interactive||s>=ACTION_COUNT||blockedStage>=0)return false;
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
        if(blockedStage>=0)return;
        // Consume a long frame one boundary at a time: no frame can skip a
        // contact check. Interactive mode always waits for fresh stage input.
        while(running&&!waiting&&dt>0){
            int s=stageAt(time);
            if(s==ACTION_COUNT){time=std::min(double(DEMO_END),time+dt);return;}
            double end=STAGE_TIME[s+1];
            if(interactive){const auto& control=stageControl(s);
                if(control.operation==Operation::Hold&&heldKey!=control.key)return;
                if(control.operation==Operation::QuarterTurn)end=std::min(end,turnTarget);
            }
            double step=std::min(dt,end-time);time+=step;dt-=step;
            if(time<end)return;
            if(time==STAGE_TIME[s+1]&&!checkBoundary(s+1,poseAt(float(time))))return;
            if(interactive){waiting=stageAt(time)<ACTION_COUNT;running=!waiting;return;}
        }
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
