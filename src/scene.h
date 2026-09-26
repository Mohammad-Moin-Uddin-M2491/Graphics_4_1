#pragma once
#include "renderer.h"
#include "animation.h"

namespace cg {
const Material steel{{.48f,.58f,.67f},64,.95f,.85f};
const Material dark{{.035f,.055f,.072f},20,.18f};
const Material rubber{{.018f,.022f,.025f},8,.04f};
const Material brass{{.9f,.46f,.09f},72,.85f,.75f};
const Material teal{{.02f,.48f,.43f},80,.65f};
const Material cream{{.82f,.85f,.76f},90,.65f};
const Material red{{.72f,.12f,.055f},48,.45f};
struct Scene {
    std::vector<Draw> items;
    void shape(int mesh,Mat4 model,Material material) {items.push_back({mesh,model,material});}
    void box(Vec3 p,Vec3 size,Material m,const Mat4& parent=Mat4::identity()) {shape(0,parent*translate(p)*scale(size),m);}
    void disk(Vec3 p,float radius,float depth,Material m,const Mat4& parent=Mat4::identity()) {shape(1,parent*translate(p)*scale({radius,radius,depth}),m);}
    void ball(Vec3 p,float r,Material m,const Mat4& parent=Mat4::identity()) {shape(2,parent*translate(p)*scale({r,r,r}),m);}
    void beam(Vec3 a,Vec3 b,float thickness,Material m) {
        Vec3 d=b-a;float len=length(d);if(len<.0001f)return;
        Vec3 y=d*(1/len),x=normalize(cross(y,std::abs(y.z)<.9f?Vec3{0,0,1}:Vec3{1,0,0})),z=cross(x,y);
        Mat4 r=Mat4::identity();r.m[0]=x.x;r.m[1]=x.y;r.m[2]=x.z;r.m[4]=y.x;r.m[5]=y.y;r.m[6]=y.z;r.m[8]=z.x;r.m[9]=z.y;r.m[10]=z.z;
        shape(0,translate((a+b)*.5f)*r*scale({thickness,len,thickness}),m);
    }
    void axleStand(float x,float y,float depth=0) {
        box({x,y*.5f,-.62f+depth},{.22f,y, .3f},dark);
        box({x,.08f,-.6f+depth},{.9f,.16f,.8f},steel);
        disk({x,y,-.25f+depth},.1f,1.15f,steel);
        disk({x,y,.4f+depth},.16f,.1f,brass);
    }
    void gear(float x,float radius,int teeth,float angle,Material m) {
        Mat4 root=translate({x,3.25f,0})*rotateZ(angle);
        disk({0,0,0},radius-.09f,.28f,m,root);
        for(int i=0;i<teeth;++i){float a=2*PI*i/teeth;Mat4 tooth=root*rotateZ(a);
            box({radius,0,0},{.19f,.18f,.32f},m,tooth);}
        disk({0,0,.17f},radius*.65f,.055f,dark,root);
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
    void car() {
        // All wheels, windows and body pieces inherit the chassis transform.
        Mat4 root=translate({8.65f,.0f,0});
        box({0,.62f,0},{3.35f,.55f,1.45f},cream,root);
        box({0,.37f,0},{2.85f,.17f,1.35f},dark,root);
        shape(3,root*translate({-.28f,1.1f,0})*scale({1.08f,.78f,1.27f}),teal);
        box({-.3f,1.51f,0},{1.26f,.065f,1.22f},cream,root);
        for(float z:{-.65f,.65f}) {
            box({-.38f,1.12f,z},{.07f,.62f,.04f},cream,root);
            box({-.67f,.78f,z*1.125f},{.28f,.035f,.04f},steel,root);
            ball({.43f,1.12f,z*1.22f},.1f,cream,root);
        }
        box({1.71f,.54f,0},{.08f,.18f,1.37f},steel,root);
        box({-1.71f,.54f,0},{.08f,.18f,1.37f},steel,root);
        box({1.72f,.7f,0},{.05f,.13f,.52f},dark,root);
        for(float z:{-.49f,.49f}){box({1.73f,.77f,z},{.05f,.16f,.27f},brass,root);box({-1.73f,.76f,z},{.05f,.12f,.27f},red,root);}
        for(float x:{-1.05f,1.05f})for(float z:{-.77f,.77f})wheel(root*translate({x,.43f,z}),.43f,.22f);
        // Car remains parked in this milestone. Its release is the final phase.
        box({10.38f,.24f,0},{.16f,.48f,1.5f},brass);
    }
    void build(const Pose& p,float time) {
        items.clear();
        box({0,-.53f,0},{200,.25f,200},{{.018f,.025f,.032f},20,.025f});
        box({.4f,-.22f,0},{25.7f,.42f,5.6f},dark);
        box({.4f,-.005f,0},{25.3f,.025f,5.2f},{{.065f,.09f,.105f},25,.12f});
        box({.4f,-.17f,2.82f},{25.4f,.045f,.035f},teal);
        for(int x=-12;x<=12;++x)box({float(x),.014f,0},{.012f,.01f,5.0f},{{.17f,.22f,.24f},4,0});
        for(int z=-2;z<=2;++z)box({.4f,.014f,float(z)},{25.1f,.01f,.012f},{{.17f,.22f,.24f},4,0});
        // Four unobtrusive zone strips on the front of the exhibit.
        box({-8.7f,.04f,2.18f},{5.4f,.05f,.09f},brass);
        box({-3.5f,.04f,2.18f},{4.4f,.05f,.09f},teal);
        box({3.0f,.04f,2.18f},{5.4f,.05f,.09f},red);
        box({9,.04f,2.18f},{4.4f,.05f,.09f},steel);
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
        // Tipping bar and dominoes rotate around local bottom-edge pivots.
        Mat4 bar=translate({1.6f,.075f,.1f})*rotateZ(p.bar);
        box({0,.98f,0},{.19f,1.96f,.32f},red,bar);
        box({-.16f,1.23f,0},{.48f,.16f,.4f},brass,bar);
        disk({1.6f,.1f,.1f},.15f,.68f,steel);
        for(int i=0;i<3;++i){float x=3.05f+i*1.05f;
            Mat4 panel=translate({x,.07f,.1f})*rotateZ(p.domino[i]);
            box({0,.825f,0},{.13f,1.65f,1.03f},i==1?cream:teal,panel);
            box({.074f,1.35f,0},{.025f,.12f,.93f},brass,panel);
            for(int j=0;j<=i;++j)disk({0,.54f+j*.2f,.533f},.048f,.024f,brass,panel);
            for(int j=0;j<=i;++j)shape(1,panel*translate({.078f,.55f+j*.23f,0})*rotateY(PI/2)*scale({.065f,.065f,.018f}),brass);
            disk({x,.07f,.1f},.105f,1.28f,steel);
        }
        // The final panel depresses a receiver. The static rod previews the
        // planned car release, which is deliberately not claimed as complete.
        float receiver=smooth(phase(time,RECEIVER_START,.22f));
        box({6.68f,.24f-.08f*receiver,.1f},{.46f,.11f,.9f},brass);
        beam({6.68f,.1f,-.7f},{10.38f,.1f,-.7f},.065f,steel);
        box({8.8f,.035f,0},{4.8f,.045f,2.22f},{{.065f,.095f,.11f},25,.15f});
        car();
    }
};
}
