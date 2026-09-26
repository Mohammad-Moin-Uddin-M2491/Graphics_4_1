#pragma once
#include "math3d.h"
#include <array>

namespace cg {
// One deterministic simulation clock. Pausing freezes it; reset reconstructs
// every pose. Overlapping intervals let the previous part finish moving.
constexpr float DEMO_END=25.0f;
constexpr float WHEEL_RADIUS=.68f;
constexpr float CRANK_START=11.2f, CATCH_START=13.2f, WEIGHT_START=14.55f, CAR_START=17.3f;
constexpr float CRANK_RADIUS=.65f, ROD_LENGTH=2.15f, CAR_WHEEL_RADIUS=.43f;
constexpr float CRANK_INITIAL=150.0f*PI/180.0f;
constexpr Vec3 CRANK_CENTER{3.55f,1.55f,.1f};
inline Vec3 crankPin(float angle) {return CRANK_CENTER+Vec3{CRANK_RADIUS*std::cos(angle),CRANK_RADIUS*std::sin(angle),0};}
inline Vec3 sliderPin(float angle) {
    float dy=CRANK_RADIUS*std::sin(angle);
    // Fixed connecting-rod length: dx^2 + dy^2 = rodLength^2.
    return {CRANK_CENTER.x+CRANK_RADIUS*std::cos(angle)+std::sqrt(ROD_LENGTH*ROD_LENGTH-dy*dy),CRANK_CENTER.y,.1f};
}
struct Pose {
    float gearSmall=0, gearLarge=0, lever=0, wheelTravel=0, wheelSpin=0;
    Vec3 wheel{-5.65f,1.85f,0};
    float wiper=0, bar=0;
    float crank=CRANK_INITIAL,catchAngle=0,weightDrop=0,pulleySpin=0,gateLift=0,carTravel=0,carSpin=0;
};
inline float phase(float t,float start,float duration) { return clamp01((t-start)/duration); }
inline Pose poseAt(float t) {
    Pose p;
    // Pitch radii .70 and 1.05: speed ratio is exactly 3:2.
    p.gearLarge=radians(150)*smooth(phase(t,0,4));
    p.gearSmall=-1.5f*p.gearLarge;
    p.lever=radians(-32)*smooth(phase(t,3.05f,1.65f));
    p.wiper=radians(-65)*smooth(phase(t,8.05f,2.4f));
    // Roll to the paddle, then keep wheel and paddle in contact while the
    // wiper sweeps. Both use the same progress, so neither jumps ahead.
    float travel=(3.87f*smooth(phase(t,4.5f,3.55f))+.30f*smooth(phase(t,8.05f,2.4f)))/4.17f;
    Vec3 start{-5.65f,1.85f,0},end{-1.48f,.908f,0};
    p.wheel=start+(end-start)*travel;
    p.wheelTravel=length(end-start)*travel;
    p.wheelSpin=-p.wheelTravel/WHEEL_RADIUS;
    p.bar=radians(-84)*smooth(phase(t,10.15f,2.25f));
    // Offset the initial crank from dead centre so a rightward strike creates
    // a clockwise torque. A 150-degree flywheel stroke drives the piston.
    p.crank=CRANK_INITIAL*(1-smooth(phase(t,CRANK_START,3.35f)));
    p.catchAngle=radians(-88)*smooth(phase(t,CATCH_START,1.3f));
    // Equal and opposite cable travel keeps the weight/gate rope length fixed.
    p.weightDrop=1.4f*smooth(phase(t,WEIGHT_START,2.1f));
    p.gateLift=p.weightDrop;p.pulleySpin=p.weightDrop/.44f;
    p.carTravel=4.0f*smooth(phase(t,CAR_START,4.7f));
    p.carSpin=-p.carTravel/CAR_WHEEL_RADIUS;
    return p;
}
inline int stageAt(float t) {
    if(t>=22.0f) return 9;
    if(t>=CAR_START) return 8;
    if(t>=WEIGHT_START) return 7;
    if(t>=CATCH_START) return 6;
    if(t>=CRANK_START) return 5;
    if(t>=10.15f) return 4;
    if(t>=8.05f) return 3;
    if(t>=4.5f) return 2;
    if(t>=3.05f) return 1;
    return 0;
}
inline const char* stageName(int s) {
    static const char* names[]={"01 / GEARS ENGAGE","02 / LEVER RELEASE","03 / WHEEL TRANSFER",
        "04 / WIPER SWEEP","05 / TIPPING BAR","06 / CRANK AND PISTON","07 / CATCH RELEASE",
        "08 / COUNTERWEIGHT LIFT","09 / CAR DEPARTURE","ENDING / CAR PARKED"};
    return names[std::clamp(s,0,9)];
}
struct Simulation {
    float time=0, speed=1;
    bool running=false;
    void update(float dt) { if(running) {time=std::min(DEMO_END,time+std::max(0.0f,dt)*speed);if(time>=DEMO_END)running=false;} }
    void reset() {time=0;running=false;}
};
}
