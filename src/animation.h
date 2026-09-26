#pragma once
#include "math3d.h"
#include <array>

namespace cg {
// One deterministic simulation clock. Pausing freezes it; reset reconstructs
// every pose. Overlapping intervals let the previous part finish moving.
constexpr float DEMO_END=17.5f;
constexpr float WHEEL_RADIUS=.68f;
constexpr float DOMINO_START=11.28f;
constexpr float DOMINO_DELAY=.77f;
constexpr float RECEIVER_START=14.3f;
struct Pose {
    float gearSmall=0, gearLarge=0, lever=0, wheelTravel=0, wheelSpin=0;
    Vec3 wheel{-5.65f,1.85f,0};
    float wiper=0, bar=0;
    std::array<float,3> domino{};
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
    // 1.05 spacing / 1.65 height: contact at about 35 degrees.
    // A panel reaches that angle ~.77 s into its 1.7 s fall.
    // At rest, adjacent panels touch: spacing * cos(angle) = thickness.
    for(int i=0;i<3;++i) p.domino[i]=-std::acos(.13f/1.05f)*smooth(phase(t,DOMINO_START+i*DOMINO_DELAY,1.7f));
    return p;
}
inline int stageAt(float t) {
    if(t>=DEMO_END) return 7;
    if(t>=RECEIVER_START) return 6;
    if(t>=DOMINO_START) return 5;
    if(t>=10.15f) return 4;
    if(t>=8.05f) return 3;
    if(t>=4.5f) return 2;
    if(t>=3.05f) return 1;
    return 0;
}
inline const char* stageName(int s) {
    static const char* names[]={"01 / GEARS ENGAGE","02 / LEVER RELEASE","03 / WHEEL TRANSFER",
        "04 / WIPER SWEEP","05 / TIPPING BAR","06 / DOMINO CASCADE","07 / RECEIVER ENGAGED","DEMO COMPLETE"};
    return names[std::clamp(s,0,7)];
}
struct Simulation {
    float time=0, speed=1;
    bool running=false;
    void update(float dt) { if(running) {time=std::min(DEMO_END,time+std::max(0.0f,dt)*speed);if(time>=DEMO_END)running=false;} }
    void reset() {time=0;running=false;}
};
}
