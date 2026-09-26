#pragma once
#include "renderer.h"
#include "animation.h"

namespace cg {
const Material steel{{.48f,.58f,.67f},64,.95f,.85f};
const Material dark{{.035f,.055f,.072f},20,.18f};
const Material rubber{{.018f,.022f,.025f},8,.04f};
const Material brass{{.49f,.31f,.12f},72,.85f,.75f};
const Material teal{{.045f,.16f,.20f},80,.65f};
const Material cream{{.63f,.64f,.58f},90,.65f};
const Material red{{.26f,.15f,.09f},48,.45f};
const Material glass{{.018f,.041f,.055f},96,.055f};
struct Scene {
    std::vector<Draw> items;
    void shape(int mesh,Mat4 model,Material material) {items.push_back({mesh,model,material});}
    void box(Vec3 p,Vec3 size,Material m,const Mat4& parent=Mat4::identity()) {shape(0,parent*translate(p)*scale(size),m);}
    void disk(Vec3 p,float radius,float depth,Material m,const Mat4& parent=Mat4::identity()) {shape(1,parent*translate(p)*scale({radius,radius,depth}),m);}
    void ball(Vec3 p,float r,Material m,const Mat4& parent=Mat4::identity()) {shape(2,parent*translate(p)*scale({r,r,r}),m);}
    void ring(Vec3 p,float r,float depth,Material m,const Mat4& parent=Mat4::identity()) {shape(4,parent*translate(p)*scale({r,r,depth}),m);}
    void mount(Vec3 p,Vec3 size) {
        box(p,size,dark);
        for(float x:{-1.f,1.f})for(float z:{-1.f,1.f})
            shape(1,translate(p+Vec3{x*size.x*.36f,size.y*.5f+.025f,z*size.z*.33f})*rotateX(PI/2)*scale({.06f,.06f,.05f}),steel);
    }
    void beam(Vec3 a,Vec3 b,float thickness,Material m) {
        Vec3 d=b-a;float len=length(d);if(len<.0001f)return;
        Vec3 y=d*(1/len),x=normalize(cross(y,std::abs(y.z)<.9f?Vec3{0,0,1}:Vec3{1,0,0})),z=cross(x,y);
        Mat4 r=Mat4::identity();r.m[0]=x.x;r.m[1]=x.y;r.m[2]=x.z;r.m[4]=y.x;r.m[5]=y.y;r.m[6]=y.z;r.m[8]=z.x;r.m[9]=z.y;r.m[10]=z.z;
        shape(0,translate((a+b)*.5f)*r*scale({thickness,len,thickness}),m);
    }
    void axleStand(float x,float y,float depth=0) {
        box({x,y*.5f,-.62f+depth},{.22f,y, .3f},dark);
        mount({x,.08f,-.6f+depth},{.9f,.16f,.8f});
        ring({x,y,-.55f+depth},.24f,.3f,steel);
        disk({x,y,-.25f+depth},.1f,1.15f,steel);
        disk({x,y,.4f+depth},.16f,.1f,brass);
    }
    void gear(float x,float radius,int teeth,float angle,Material m) {
        Mat4 root=translate({x,3.25f,0})*rotateZ(angle);
        ring({0,0,0},radius-.09f,.4f,m,root);
        for(int i=0;i<teeth;++i){float a=2*PI*i/teeth;Mat4 tooth=root*rotateZ(a);
            box({radius,0,0},{.19f,.18f,.4f},m,tooth);}
        for(int i=0;i<5;++i){Mat4 spoke=root*rotateZ(2*PI*i/5);box({radius*.31f,0,.22f},{radius*.65f,.12f,.1f},m,spoke);}
        disk({0,0,.28f},.24f,.14f,steel,root);disk({0,0,.365f},.105f,.04f,brass,root);
    }
    void wheel(const Mat4& root,float radius,float width) {
        disk({0,0,0},radius,width,rubber,root);
        for(float side:{-1.0f,1.0f}) {
            float z=side*(width*.5f+.012f);
            disk({0,0,z},radius*.73f,.035f,steel,root);
            disk({0,0,z+side*.025f},radius*.57f,.025f,dark,root);
            for(int i=0;i<6;++i){Mat4 spoke=root*rotateZ(i*PI/3);box({radius*.31f,0,z+side*.055f},{radius*.58f,radius*.1f,.06f},steel,spoke);}
            disk({0,0,z+side*.07f},radius*.19f,.08f,brass,root);
        }
        // Small cube treads make the rotation visible from every camera angle.
        for(int i=0;i<32;++i)box({radius-.007f,0,0},{.025f,.075f,width*.91f},dark,root*rotateZ(i*2*PI/32));
    }
    void car(const Pose& p) {
        // All wheels, windows and body pieces inherit the chassis transform.
        Mat4 root=translate({9.0f,.03f,.4f})*rotateY(-PI/2)*translate({p.carTravel,0,0});
        box({0,.72f,0},{3.35f,.36f,1.45f},teal,root);
        box({0,.37f,0},{2.85f,.17f,1.35f},dark,root);
        shape(3,root*translate({-.28f,1.1f,0})*scale({1.08f,.78f,1.27f}),glass);
        box({-.3f,1.51f,0},{1.26f,.065f,1.22f},teal,root);
        box({1.02f,.935f,0},{1.05f,.055f,1.35f},teal,root);
        for(float z:{-.65f,.65f}) {
            box({-.38f,1.12f,z},{.07f,.62f,.04f},teal,root);
            box({-.32f,.73f,z*1.13f},{1.1f,.025f,.025f},steel,root);
            box({-.15f,.63f,z*1.14f},{.015f,.15f,.018f},dark,root);
            box({-.67f,.78f,z*1.125f},{.28f,.035f,.04f},steel,root);
            ball({.43f,1.12f,z*1.22f},.1f,teal,root);
            for(float x:{-1.05f,1.05f})shape(5,root*translate({x,.43f,z*1.18f})*scale({.52f,.52f,.08f}),teal);
        }
        box({1.71f,.54f,0},{.08f,.18f,1.37f},steel,root);
        box({-1.71f,.54f,0},{.08f,.18f,1.37f},steel,root);
        box({1.72f,.7f,0},{.05f,.13f,.52f},dark,root);
        for(float z:{-.49f,.49f}){box({1.73f,.77f,z},{.05f,.16f,.27f},{{.85f,.8f,.58f},96,.8f},root);box({-1.73f,.76f,z},{.05f,.12f,.27f},{{.42f,.015f,.01f},64,.6f},root);}
        for(int i=0;i<5;++i)box({1.75f,.66f,(i-2)*.08f},{.025f,.09f,.025f},steel,root);
        for(float x:{-1.05f,1.05f})for(float z:{-.77f,.77f})wheel(root*translate({x,.43f,z})*rotateZ(p.carSpin),CAR_WHEEL_RADIUS,.22f);
    }
    void build(const Pose& p,float /*time*/) {
        items.clear();
        box({0,-.53f,0},{200,.25f,200},{{.018f,.025f,.032f},20,.025f});
        box({-.5f,-.22f,0},{23.5f,.42f,4.2f},dark);
        box({-.5f,-.005f,0},{23.3f,.025f,4.0f},{{.045f,.058f,.065f},25,.1f});
        box({9,-.22f,4.4f},{3.3f,.42f,4.8f},dark);
        box({9,.01f,4.4f},{3.1f,.05f,4.8f},{{.045f,.058f,.065f},25,.1f});
        for(float x:{-11.f,-6.f,-1.f,4.f,10.f})for(float z:{-1.7f,1.7f})mount({x,.025f,z},{.35f,.04f,.3f});
        for(float x:{7.95f,10.05f})box({x,.05f,4.45f},{.055f,.06f,4.7f},steel);
        axleStand(-10,3.25f);axleStand(-8.25f,3.25f);
        gear(-10,.70f,12,p.gearSmall,brass);
        gear(-8.25f,1.05f,18,p.gearLarge+PI/18,steel);
        Mat4 cam=translate({-8.25f,3.25f,0})*rotateZ(p.gearLarge+radians(-100));
        disk({.85f,0,.5f},.12f,.4f,brass,cam);
        // Lever local origin is its axle. Its right end retracts the gate.
        axleStand(-6.7f,3.55f,-.32f);
        Mat4 lever=translate({-6.7f,3.55f,.55f})*rotateZ(p.lever);
        box({.5f,0,0},{2.8f,.16f,.25f},brass,lever);
        box({-.9f,0,0},{.55f,.21f,.5f},red,lever);
        disk({0,0,.16f},.18f,.13f,steel,lever);
        disk({1.9f,0,-.48f},.08f,1.1f,steel,lever);
        float release=-p.lever/radians(32);
        Vec3 leverEnd=transform(lever,{1.9f,0,-.95f});
        beam(leverEnd,{-4.88f,1.8f-release*.96f,-.4f},.07f,steel);
        box({-4.88f,1.43f-release*.96f,0},{.14f,.86f,1.05f},red);
        // Track surface is offset from the wheel centre by its normal radius.
        Vec3 trackA{-6.3f,1.31f,0},trackB{-1.3f,.18f,0};
        Vec3 trackD=trackB-trackA;float trackAngle=std::atan2(trackD.y,trackD.x);
        Mat4 track=translate((trackA+trackB)*.5f)*rotateZ(trackAngle);
        // A narrow slot permits the stop gate to retract through the deck.
        box({-1.8865f,-.07f,0},{1.353f,.14f,.92f},steel,track);
        box({.7815f,-.07f,0},{3.563f,.14f,.92f},steel,track);
        for(float z:{-.55f,.55f})box({0,.015f,z},{length(trackD),.1f,.09f},brass,track);
        for(float x:{-6.0f,-4.0f,-2.0f}) {float y=1.31f+(x+6.3f)*(-1.13f/5);box({x,y*.5f,-.05f},{.17f,y,.6f},dark);}
        wheel(translate(p.wheel)*rotateZ(p.wheelSpin),WHEEL_RADIUS,.46f);
        // Transparent-looking windshield uses an open frame to avoid hiding
        // contacts or introducing blending/sorting into this course milestone.
        for(float x:{-.45f,1.55f})box({x,1.25f,-.56f},{.075f,2.5f,.09f},steel);
        box({.55f,2.5f,-.56f},{2.08f,.075f,.09f},steel);
        box({.55f,.06f,-.56f},{2.08f,.09f,.12f},dark);
        Mat4 wiper=translate({-.45f,.32f,.07f})*rotateZ(p.wiper);
        box({0,.95f,0},{.1f,1.9f,.12f},steel,wiper);
        box({0,1.83f,.04f},{.13f,.65f,.17f},rubber,wiper);
        disk({0,0,.1f},.19f,.25f,brass,wiper);
        // Wheel pushes the paddle; a visible rod turns the wiper crank.
        float sweep=-p.wiper/radians(65);
        Vec3 paddle{-1.05f+.3f*sweep,.72f,.12f};
        box(paddle,{.1f,.75f,.65f},brass);
        beam({paddle.x,.47f,.42f},transform(wiper,{-.28f,0,.35f}),.065f,steel);
        box({-1.13f,.15f,0},{.17f,.22f,1.03f},rubber);
        // Tipping bar strikes the crank's left paddle at the start of its turn.
        Mat4 bar=translate({1.6f,.075f,.1f})*rotateZ(p.bar);
        box({0,.98f,0},{.19f,1.96f,.32f},red,bar);
        box({-.16f,1.23f,0},{.48f,.16f,.4f},brass,bar);
        disk({1.6f,.1f,.1f},.15f,.68f,steel);
        // Crank and slider: rotary input, fixed-length rod, linear piston.
        axleStand(CRANK_CENTER.x,CRANK_CENTER.y);
        Mat4 crank=translate(CRANK_CENTER)*rotateZ(p.crank);
        ring({0,0,0},.79f,.18f,steel,crank);
        for(int i=0;i<4;++i)box({.35f,0,0},{.7f,.09f,.16f},steel,crank*rotateZ(i*PI/2));
        box({CRANK_RADIUS,0,.05f},{.23f,.70f,.34f},brass,crank);
        disk({0,0,.15f},.19f,.25f,brass,crank);
        Vec3 pin=crankPin(p.crank),slider=sliderPin(p.crank);
        beam(pin+Vec3{0,0,.27f},slider+Vec3{0,0,.27f},.14f,steel);
        for(Vec3 point:{pin,slider}){ring(point+Vec3{0,0,.27f},.16f,.12f,brass);disk(point+Vec3{0,0,.36f},.075f,.09f,steel);}
        for(float z:{-.32f,.52f})beam({4.55f,1.55f,z},{6.65f,1.55f,z},.09f,steel);
        for(float x:{4.7f,6.45f}){mount({x,.09f,.1f},{.55f,.18f,1.15f});box({x,.75f,-.3f},{.12f,1.5f,.15f},dark);}
        box(slider,{.42f,.48f,.63f},steel);
        shape(1,translate(slider+Vec3{.37f,0,0})*rotateY(PI/2)*scale({.13f,.13f,.52f}),brass);
        box(slider+Vec3{.52f,.2f,0},{.15f,.42f,.22f},steel);
        // Crosshead nose pushes the catch's low input tab. Its upper finger
        // rotates down and removes the counterweight's supporting ledge.
        Mat4 latch=translate({6.7f,1.68f,.1f})*rotateZ(p.catchAngle);
        box({.4f,0,0},{.9f,.1f,.5f},brass,latch);
        box({-.05f,.19f,0},{.1f,.32f,.45f},steel,latch);
        disk({6.7f,1.68f,.39f},.12f,.16f,steel);
        mount({6.7f,.1f,-.4f},{.65f,.2f,.7f});box({6.7f,.9f,-.4f},{.14f,1.6f,.16f},dark);

        // Guided counterweight and an overhead cable raise the exit barrier.
        const Material cable{{.10f,.075f,.04f},20,.12f};
        for(float x:{6.69f,7.73f}){box({x,1.65f,-.16f},{.08f,3.3f,.12f},steel);mount({x,.08f,-.16f},{.38f,.16f,.5f});}
        box({7.21f,1.95f-p.weightDrop,.1f},{.78f,.5f,.62f},dark);
        for(float y:{-.16f,0.f,.16f})box({7.21f,1.95f-p.weightDrop+y,.43f},{.67f,.025f,.02f},steel);
        ring({7.21f,2.24f-p.weightDrop,.1f},.10f,.09f,steel);
        axleStand(7.65f,3.2f,-.55f);
        Mat4 pulley=translate({7.65f,3.2f,.1f})*rotateZ(p.pulleySpin);
        ring({0,0,0},.44f,.2f,steel,pulley);
        for(int i=0;i<4;++i)box({.2f,0,0},{.4f,.06f,.14f},brass,pulley*rotateZ(i*PI/2));
        disk({0,0,.16f},.13f,.15f,brass,pulley);
        beam({7.21f,2.24f-p.weightDrop,.1f},{7.21f,3.2f,.1f},.025f,cable);
        for(int i=0;i<24;++i){float a=PI*i/24,b=PI*(i+1)/24;
            beam({7.65f+.44f*std::cos(a),3.2f+.44f*std::sin(a),.1f},
                 {7.65f+.44f*std::cos(b),3.2f+.44f*std::sin(b),.1f},.025f,cable);}
        beam({8.09f,3.2f,.1f},{8.09f,2.98f,.1f},.025f,cable);
        // Two small fixed guide pulleys turn the cable into the car bay.
        for(float z:{.32f,1.98f}){
            Mat4 guide=translate({8.09f,2.98f,z})*rotateY(PI/2)*rotateZ(-p.weightDrop/.22f);
            ring({0,0,0},.22f,.12f,steel,guide);disk({0,0,0},.065f,.22f,brass,guide);
            for(int i=0;i<4;++i)box({.1f,0,0},{.2f,.03f,.09f},steel,guide*rotateZ(i*PI/2));
            beam({8.09f,2.98f,z},{7.85f,2.98f,z},.08f,steel);
        }
        for(int i=0;i<12;++i){float a=PI-i*PI/24,b=PI-(i+1)*PI/24;
            beam({8.09f,2.98f+.22f*std::sin(a),.32f+.22f*std::cos(a)},
                 {8.09f,2.98f+.22f*std::sin(b),.32f+.22f*std::cos(b)},.025f,cable);
            a=PI/2-i*PI/24;b=PI/2-(i+1)*PI/24;
            beam({8.09f,2.98f+.22f*std::sin(a),1.98f+.22f*std::cos(a)},
                 {8.09f,2.98f+.22f*std::sin(b),1.98f+.22f*std::cos(b)},.025f,cable);}
        beam({8.09f,3.2f,.32f},{8.09f,3.2f,1.98f},.025f,cable);
        beam({8.09f,2.98f,2.2f},{8.09f,.36f+p.gateLift,2.2f},.025f,cable);
        beam({7.85f,3.0f,-.2f},{7.85f,3.0f,2.2f},.1f,dark);
        for(float x:{7.95f,10.05f}){mount({x,.08f,2.2f},{.34f,.16f,.5f});box({x,1.02f,2.2f},{.1f,2.04f,.13f},steel);}
        box({9,.36f+p.gateLift,2.2f},{2.1f,.16f,.13f},brass);
        for(float x:{7.95f,10.05f})box({x,.36f+p.gateLift,2.2f},{.19f,.3f,.23f},dark);
        // The car travels toward the viewer along +Z, on the same bench level.
        car(p);
    }
};
}
