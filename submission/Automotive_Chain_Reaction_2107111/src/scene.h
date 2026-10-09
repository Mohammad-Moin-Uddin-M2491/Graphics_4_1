#pragma once
#include "renderer.h"
#include "animation.h"

namespace cg {
const Material steel{{.48f,.58f,.67f},64,.95f,.85f};
const Material dark{{.035f,.055f,.072f},20,.18f};
const Material rubber{{.018f,.022f,.025f},8,.04f};
const Material brass{{.49f,.31f,.12f},72,.85f,.75f};
const Material teal{{.035f,.19f,.22f},64,.48f};
const Material cream{{.63f,.64f,.58f},90,.65f};
const Material red{{.26f,.15f,.09f},48,.45f};
const Material glass{{.018f,.041f,.055f},96,.055f};
struct Scene {
    std::vector<Draw> items;
    int activeStage=-1;
    int lampFirst=-1,lampEnd=-1;
    bool isLamp(int item)const{return item>=lampFirst&&item<lampEnd;}
    Material contact(Material m,int stage)const{
        if(activeStage==stage||activeStage==stage+1)m.color=m.color*.55f+Vec3{.90f,.68f,.30f}*.45f;
        return m;
    }
    void shape(int mesh,Mat4 model,Material material) {items.push_back({mesh,model,material});}
    void box(Vec3 p,Vec3 size,Material m,const Mat4& parent=Mat4::identity()) {
        float extent=std::max({size.x,size.y,size.z});
        // Extra vertices sample local point lighting on large surfaces in Gouraud.
        shape(extent>50?7:extent>5?6:0,parent*translate(p)*scale(size),m);
    }
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
        Mat4 root=translate(p.carPosition)*rotateY(p.carYaw);
        // Visible starter socket is a child of the car, exactly aligned with
        // the external shaft. Its hub turns with the starter before departure.
        ring({.85f,.8f,-.82f},.23f,.20f,contact(brass,Starter),root);
        Mat4 socket=root*translate({.85f,.8f,-.94f})*rotateZ(-p.starter);
        disk({0,0,0},.10f,.12f,dark,socket);
        box({0,0,-.07f},{.16f,.045f,.03f},cream,socket);
        box({0,.72f,0},{3.35f,.36f,1.45f},teal,root);
        box({0,.37f,0},{2.85f,.17f,1.35f},dark,root);
        shape(3,root*translate({-.28f,1.1f,0})*scale({1.08f,.78f,1.27f}),glass);
        box({-.3f,1.51f,0},{1.26f,.065f,1.22f},teal,root);
        box({1.02f,.935f,0},{1.05f,.055f,1.35f},teal,root);
        // Restrained body trim, separate glass panes and a readable front end.
        for(float z:{-.61f,.61f}){
            beam(transform(root,{-.97f,.76f,z}),transform(root,{-.98f,1.46f,z}),.045f,steel);
            beam(transform(root,{.73f,.76f,z}),transform(root,{.31f,1.46f,z}),.045f,teal);
            box({0,.51f,z*1.18f},{2.3f,.055f,.035f},steel,root);
            box({.24f,.81f,z*1.2f},{.30f,.035f,.045f},steel,root);
        }
        box({.15f,.971f,0},{.02f,.012f,1.28f},dark,root);
        box({1.77f,.53f,0},{.025f,.13f,.33f},cream,root);
        box({-1.77f,.55f,0},{.025f,.13f,.33f},cream,root);
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
        for(float x:{-1.05f,1.05f})for(float z:{-.77f,.77f})
            wheel(root*translate({x,.43f,z})*rotateY(x>0?p.steer:0)*rotateZ(p.carSpin),CAR_WHEEL_RADIUS,.22f);
    }
    void relay(const Pose& p){
        // Front row runs RIGHT TO LEFT: rack -> ball -> cup -> striker ->
        // rocker -> pin. The low cable returns to the catch on the right.
        const Material cable{{.42f,.30f,.14f},32,.35f};
        Vec3 direction=normalize(BALL_END-BALL_START);
        Vec3 normal{direction.y,-direction.x,0};
        Vec3 a=BALL_START-normal*BALL_RADIUS,b=BALL_END-normal*BALL_RADIUS-direction*.15f;
        // Low rails and a thin bed keep the sphere in visible contact.
        Vec3 track=b-a;Mat4 bed=translate((a+b)*.5f-normal*.03f)*rotateZ(std::atan2(track.y,track.x));
        box({0,0,0},{length(track),.06f,.42f},dark,bed);
        for(float z:{-.19f,.19f})beam(a+Vec3{0,-.035f,z},b+Vec3{0,-.035f,z},.085f,steel);
        for(float u:{0.f,.33f,.67f,1.f}){
            Vec3 q=a+(b-a)*u;
            beam(q+Vec3{0,-.09f,-.3f},q+Vec3{0,-.09f,.3f},.09f,dark);
            box({q.x,q.y*.5f-.07f,2.83f},{.12f,q.y-.14f,.14f},dark);
            mount({q.x,.075f,2.83f},{.45f,.15f,.52f});
        }
        // Rack travel lifts the ball's stop blade through a guide. An exposed
        // rigid diagonal link and cable make this transfer visible from camera 2 or 5.
        box({5.23,2.055f+p.ballGate,3.15f},{.10f,.95f,.68f},brass);
        for(float z:{2.69f,3.61f}){
            box({5.23,1.77f,z},{.12f,3.54f,.12f},dark);
            mount({5.23,.07f,z},{.4f,.14f,.42f});
        }
        beam({5.23,3.54f,2.69f},{5.23,3.54f,3.61f},.13f,steel);
        ring({5.23,3.70f,3.15f},.17f,.13f,steel);
        ring({5.9f,3.95f,.78f},.17f,.13f,steel);
        beam({5.9f,.15f,.60f},{5.9f,3.95f,.60f},.10f,dark);
        // The rack pulls a guided follower down while the stop rises equally.
        Vec3 follower=rackFollower(p.rack);
        box(follower,{.27f,.18f,.25f},brass);
        beam({6.30f+.95f*p.rack,2.25f,.78f},follower,.075f,steel);
        beam(follower,{5.9f,4.12f,.78f},.035f,cable);
        beam({5.9f,4.12f,.78f},{5.23,3.87f,3.15f},.035f,cable);
        beam({5.23,3.87f,3.15f},{5.23,2.53f+p.ballGate,3.15f},.035f,cable);
        // A brass stripe is parented to the rolling sphere, exposing s/r spin.
        Mat4 ballRoot=translate(p.relayBall)*rotateZ(p.ballSpin+p.cup);
        ball({0,0,0},BALL_RADIUS,steel,ballRoot);
        ring({0,0,0},BALL_RADIUS*1.012f,.065f,brass,ballRoot);
        ball({BALL_RADIUS*.76f,0,BALL_RADIUS*.66f},.05f,dark,ballRoot);

        // Captive trigger cup: floor, side cheeks and low front lip are cubes.
        Mat4 cup=cupMatrix(p.cup);
        box({-.50f,.05f,0},{.86f,.10f,.72f},brass,cup);
        // Left wall catches the ball; the right entry remains open to the ramp.
        box({-.905f,.27f,0},{.05f,.44f,.70f},teal,cup);
        box({-.50f,.23f,-.34f},{.86f,.35f,.055f},teal,cup);
        box({-.50f,.145f,.34f},{.86f,.09f,.055f},steel,cup);
        beam(transform(cup,{0,0,-.28f}),transform(cup,CUP_FINGER+Vec3{0,0,-.28f}),.13f,brass);
        box(CUP_FINGER,CUP_FINGER_SIZE,contact(brass,Cup),cup);
        disk({0,0,.41f},.14f,.15f,steel,cup);
        mount({CUP_PIVOT.x,.08f,2.65f},{.65f,.16f,.55f});
        beam({CUP_PIVOT.x,.15f,2.65f},CUP_PIVOT+Vec3{0,0,-.45f},.15f,dark);
        disk(CUP_PIVOT+Vec3{0,0,-.25f},.08f,.7f,steel);

        // Pendulum starts resting against the cup finger. The cup rotates that
        // finger away; a single arc ends at the rocker's strike pad.
        mount({PENDULUM_PIVOT.x,.09f,2.62f},{.8f,.18f,.7f});
        beam({PENDULUM_PIVOT.x,.18f,2.62f},{PENDULUM_PIVOT.x,2.90f,2.62f},.17f,dark);
        beam({PENDULUM_PIVOT.x,2.90f,2.62f},PENDULUM_PIVOT,.14f,steel);
        Mat4 pendulum=translate(PENDULUM_PIVOT)*rotateZ(p.pendulum);
        box({0,-PENDULUM_LENGTH*.5f,0},{.12f,PENDULUM_LENGTH,.14f},steel,pendulum);
        ball({0,-PENDULUM_LENGTH,0},STRIKER_RADIUS,contact(brass,Pendulum),pendulum);
        disk({0,0,.14f},.18f,.16f,brass,pendulum);
        // A damped end stop holds the striker after contact; no idle oscillation.
        box({PENDULUM_PIVOT.x-1.17f,1.15f,3.15f},{.30f,.12f,.56f},rubber);

        Mat4 rocker=translate(ROCKER_PIVOT)*rotateZ(p.rocker);
        box({-.03f,0,0},{2.30f,.16f,.24f},teal,rocker);
        box(ROCKER_PAD,ROCKER_PAD_SIZE,contact(brass,Pendulum),rocker);
        disk({0,0,.20f},.18f,.22f,steel,rocker);
        beam(transform(rocker,{-1.12f,0,0}),rockerTip(p.rocker),.11f,steel);
        ball(rockerTip(p.rocker),ROCKER_ROLLER_RADIUS,contact(brass,Rocker));
        mount({ROCKER_PIVOT.x,.09f,2.71f},{.8f,.18f,.75f});
        beam({ROCKER_PIVOT.x,.18f,2.71f},ROCKER_PIVOT+Vec3{0,0,-.3f},.16f,dark);
        disk(ROCKER_PIVOT+Vec3{0,0,-.08f},.085f,.6f,steel);

        // Rocker raises the collar to unlatch a vertically sliding pin. Its
        // withdrawal frees the spring drum; the drum then pulls the old bolt.
        Vec3 pin=RELEASE_PIN+Vec3{0,p.releasePin,0};
        box(pin,{.14f,PIN_LENGTH,.15f},brass);
        box(pin+Vec3{0,COLLAR_Y,0},COLLAR_SIZE,contact(steel,Rocker));
        for(float y:{1.66f,2.04f}){
            box({RELEASE_PIN.x,y,3.55f},{.42f,.18f,.18f},dark);
            shape(1,translate({RELEASE_PIN.x,y,3.70f})*rotateX(PI/2)*scale({.16f,.16f,.15f}),steel);
        }
        mount({RELEASE_PIN.x,.08f,3.35f},{.75f,.16f,.8f});
        beam({RELEASE_PIN.x,.16f,3.35f},{RELEASE_PIN.x,2.14f,3.35f},.13f,dark);
        Mat4 drum=translate(DRUM_CENTER)*rotateZ(p.releaseDrum);
        ring({0,0,0},DRUM_RADIUS,.19f,teal,drum);
        for(int i=0;i<4;++i)box({.2f,0,0},{.42f,.07f,.20f},brass,drum*rotateZ(i*PI/2));
        disk({0,0,.14f},.12f,.14f,steel,drum);
        box({0,.31f,0},{.16f,.20f,.22f},brass,drum);
        // An exposed pre-wound clock spring drives the spool once the pin lifts.
        // Outer spring end is fixed to the housing, inner end follows the axle.
        ring(DRUM_CENTER+Vec3{0,0,.24f},.47f,.06f,dark);
        for(int i=0;i<100;++i){
            auto spring=[&](float u){float a=6*PI*u+p.releaseDrum*(1-u),r=.09f+.23f*u;
                return DRUM_CENTER+Vec3{r*std::cos(a),r*std::sin(a),.29f};};
            beam(spring(i/100.f),spring((i+1)/100.f),.025f,contact(brass,Drum));
        }
        disk(DRUM_CENTER+Vec3{0,0,.22f},.085f,.28f,steel);
        beam(DRUM_CENTER+Vec3{.32f,0,.29f},DRUM_CENTER+Vec3{.53f,0,.29f},.07f,steel);
        beam(DRUM_CENTER+Vec3{.53f,0,.29f},DRUM_CENTER+Vec3{.53f,-.8f,.29f},.08f,dark);
        // Continuous cable: rotating anchor -> spool wrap -> bottom tangent ->
        // XY quarter guide -> vertical -> YZ quarter guide -> moving bolt end.
        float anchor=PI/2+p.releaseDrum;
        for(int i=0;i<40;++i){float a=lerp(anchor,1.5f*PI,i/40.f),b=lerp(anchor,1.5f*PI,(i+1)/40.f);
            beam(DRUM_CENTER+Vec3{DRUM_RADIUS*std::cos(a),DRUM_RADIUS*std::sin(a),0},
                 DRUM_CENTER+Vec3{DRUM_RADIUS*std::cos(b),DRUM_RADIUS*std::sin(b),0},.04f,cable);}
        ball(DRUM_CENTER+Vec3{DRUM_RADIUS*std::cos(anchor),DRUM_RADIUS*std::sin(anchor),0},.065f,steel);
        Vec3 first=DRUM_CENTER+Vec3{0,-DRUM_RADIUS,0},corner=CABLE_BEND_XY+Vec3{0,-CABLE_GUIDE_RADIUS,0};
        beam(first,corner,.04f,cable);
        for(float x:{-7.f,-3.f,1.f,5.f}){
            box({x,.32f,3.91f},{.09f,.64f,.1f},dark);
            shape(4,translate({x,.58f,3.70f})*rotateY(PI/2)*scale({.085f,.085f,.10f}),steel);
        }
        ring(CABLE_BEND_XY,CABLE_GUIDE_RADIUS,.10f,steel);
        shape(4,translate(CABLE_BEND_YZ)*rotateY(PI/2)*scale({CABLE_GUIDE_RADIUS,CABLE_GUIDE_RADIUS,.10f}),steel);
        for(int i=0;i<16;++i){float a=PI*.5f*i/16,b=PI*.5f*(i+1)/16,r=CABLE_GUIDE_RADIUS;
            beam(CABLE_BEND_XY+Vec3{r*std::sin(a),-r*std::cos(a),0},CABLE_BEND_XY+Vec3{r*std::sin(b),-r*std::cos(b),0},.04f,cable);
            beam(CABLE_BEND_YZ+Vec3{0,r*std::sin(a),r*std::cos(a)},CABLE_BEND_YZ+Vec3{0,r*std::sin(b),r*std::cos(b)},.04f,cable);
        }
        beam(CABLE_BEND_XY+Vec3{CABLE_GUIDE_RADIUS,0,0},CABLE_BEND_YZ+Vec3{0,0,CABLE_GUIDE_RADIUS},.04f,cable);
        beam(CABLE_BEND_YZ+Vec3{0,CABLE_GUIDE_RADIUS,0},boltCablePoint(p.bolt),.04f,cable);
        for(Vec3 center:{CABLE_BEND_XY,CABLE_BEND_YZ}){disk(center,.055f,.16f,brass);beam(center,{7.17f,center.y,3.70f},.07f,steel);}
        // Ferrules make the transmitted cable travel visible in the overview.
        ball({-4-p.bolt,.58f,3.70f},.065f,cream);ball({6.93f,1.75f,2.1f+p.bolt},.065f,cream);
        mount({7.17f,.08f,3.70f},{.40f,.16f,.55f});
        beam({7.17f,.16f,3.70f},{7.17f,1.95f,3.70f},.10f,dark);
    }
    void benchLamp(int lighting){
        // A clamped workshop inspection lamp: cubes, beams and a spherical bulb.
        // Front edge placement illuminates the faces seen in the presentation
        // without an artificial fill source behind the camera.
        const Vec3 foot{-.4f,.09f,4.05f},joint{-.4f,6.4f,4.05f};
        mount(foot,{.72f,.18f,.7f});
        beam(foot+Vec3{0,.09f,0},joint,.12f,steel);
        ball(joint,.18f,brass);
        beam(joint,LAMP_BULB+Vec3{0,.60f,0},.13f,steel);
        beam(joint-Vec3{0,.65f,0},LAMP_BULB+Vec3{.45f,.60f,0},.065f,steel);
        lampFirst=int(items.size());
        box(LAMP_BULB+Vec3{0,.49f,0},{.26f,.25f,.26f},brass);
        box(LAMP_BULB+Vec3{0,.36f,0},{.96f,.14f,.85f},teal);
        Vec3 tint=lightingPreset(lighting).lampColor;
        tint=tint*(1/std::max({tint.x,tint.y,tint.z}));
        ball(LAMP_BULB,LAMP_RADIUS,{tint,24,.12f,0,2.6f});
        // Open guard leaves the bulb visible and the downward light unobstructed.
        for(float x:{-.42f,.42f})for(float z:{-.36f,.36f})
            beam(LAMP_BULB+Vec3{x,.30f,z},LAMP_BULB+Vec3{x,-.40f,z},.035f,steel);
        for(float z:{-.36f,.36f})box(LAMP_BULB+Vec3{0,-.40f,z},{.88f,.045f,.04f},steel);
        lampEnd=int(items.size());
    }
    void build(const Pose& p,float time,int lighting=0) {
        items.clear();activeStage=stageAt(time);
        box({0,-2.7f,0},{200,.25f,200},{{.018f,.025f,.032f},20,.025f});
        const Material benchTop{{.10f,.13f,.145f},26,.12f},edge{{.17f,.115f,.07f},24,.10f};
        // Stepped workbench leaves the bridge gap open beside the new front row.
        box({-2.40f,-.24f,1.10f},{19.7f,.46f,6.8f},edge);
        box({-2.40f,-.005f,1.10f},{19.65f,.025f,6.75f},benchTop);
        box({9,-.24f,0},{3.3f,.46f,4.2f},edge);
        box({9,-.005f,0},{3.25f,.025f,4.15f},benchTop);
        for(float x:{-11.3f,-5.7f,.1f,6.2f,9.9f}){
            float front=x>7?1.5f:3.9f;
            for(float z:{-1.6f,front}){
                box({x,-1.5f,z},{.25f,2.1f,.25f},dark);
                box({x,-2.5f,z},{.65f,.16f,.58f},rubber);
            }
            beam({x,-1.85f,-1.6f},{x,-1.85f,front},.15f,steel);
        }
        for(float z:{-1.6f,3.9f})beam({-11.3f,-1.55f,z},{6.2f,-1.55f,z},.14f,dark);
        box({-2.4f,-.30f,4.51f},{19.5f,.08f,.045f},steel);
        // Fixed approach, hinged bridge across a real gap, and a driving apron.
        box({9,-.22f,2.55f},{3.3f,.42f,.9f},dark);
        box({9,.005f,2.55f},{3.1f,.05f,.9f},dark);
        box({9,-.22f,11.1f},{14,.42f,11.8f},dark);
        box({9,.005f,11.1f},{13.9f,.05f,11.7f},{{.07f,.08f,.085f},24,.15f});
        for(float z=6;z<17;z+=1.5f)box({9,.035f,z},{.07f,.012f,.65f},cream);
        for(float x:{2.15f,15.85f})box({x,.035f,11.1f},{.08f,.014f,11.4f},brass);
        Mat4 bridge=translate({9,.03f,3})*rotateX(-p.bridge);
        box({0,-.065f,1.1f},{3.1f,.13f,2.2f},steel,bridge);
        for(float x:{-1.43f,1.43f})box({x,.04f,1.1f},{.09f,.1f,2.2f},brass,bridge);
        for(float z=.2f;z<2.2f;z+=.3f)box({0,.007f,z},{2.7f,.015f,.025f},dark,bridge);
        shape(1,translate({9,.03f,3})*rotateY(PI/2)*scale({.13f,.13f,3.4f}),brass);
        // Weight depresses a foot pedal; its rod releases the bridge damper.
        float pedalY=std::min(.42f,1.65f-p.weightDrop);
        box({7.21f,pedalY,.1f},{.85f,.1f,.7f},brass);
        beam({7.21f,pedalY,.6f},{7.4f,.22f,2.85f},.07f,steel);
        beam({7.4f,.22f,2.85f},transform(bridge,{-1.43f,0,1.65f}),.09f,steel);
        for(float x:{-11.f,-6.f,-1.f,4.f,10.f})for(float z:{-1.7f,1.7f})mount({x,.025f,z},{.35f,.04f,.3f});
        for(float x:{7.5f,10.5f})box({x,.06f,2.55f},{.055f,.06f,.9f},steel);
        axleStand(-10,3.25f);axleStand(-8.25f,3.25f);
        gear(-10,.70f,12,p.gearSmall,brass);
        gear(-8.25f,1.05f,18,p.gearLarge+PI/18,steel);
        // The output cam is stationary while the input winds the spring.
        Mat4 cam=translate({-8.25f,3.25f,.32f})*rotateZ(p.cam+radians(-100));
        disk({0,0,.08f},.33f,.14f,dark,cam);
        disk({.85f,0,.5f},.12f,.4f,brass,cam);
        beam(transform(cam,{0,0,.5f}),transform(cam,{.85f,0,.5f}),.08f,steel);
        // Exposed torsion spring tightens as the gear train winds it.
        for(int i=0;i<100;++i){
            auto spring=[&](float u){float a=u*8*PI+p.gearLarge*.12f;
                float r=.2f+u*(.5f-.17f*std::min(p.gearLarge/(2*PI),1.f));
                return Vec3{-8.25f+r*std::cos(a),3.25f+r*std::sin(a),.53f};};
            beam(spring(i/100.f),spring((i+1)/100.f),.028f,brass);
        }
        // Glove: cube palm/fingers and sphere knuckles, on a simple articulated arm.
        mount({-11.5f,.1f,1.1f},{.6f,.2f,.8f});
        Vec3 shoulder{-11.5f,1.7f,1.1f},elbow{-11.55f,p.hand.y+.5f,1.1f};
        beam({-11.5f,.2f,1.1f},shoulder,.15f,dark);beam(shoulder,elbow,.12f,steel);beam(elbow,p.hand,.12f,steel);
        ball(shoulder,.16f,brass);ball(elbow,.15f,brass);
        box(p.hand,{.5f,.3f,.3f},cream);
        for(int i=0;i<3;++i){float z=p.hand.z+(i-1)*.12f;
            ball({p.hand.x+.23f,p.hand.y-.05f,z},.075f,cream);
            Mat4 finger=translate({p.hand.x+.23f,p.hand.y-.05f,z})*rotateZ(-.4f-.6f*p.grip);
            box({0,-.23f,0},{.11f,.46f,.095f},cream,finger);
        }
        box(p.hand+Vec3{-.2f+.08f*p.grip,-.23f,0},{.13f,.4f,.16f},cream);
        box({-10.8f,.15f,1.2f},{.8f,.2f,.65f},dark);
        ring({-10,2.68f,.8f},.18f,.15f,teal);
        beam({-10,2.68f,.7f},{-10,3.25f,.45f},.08f,steel);
        disk(p.pin,.10f,.38f,brass);ball(p.pin+Vec3{0,0,.21f},.14f,brass);
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
        Vec3 paddle{-1.05f+.3f*p.paddle,.72f,.12f};
        box(paddle,{.1f,.75f,.65f},brass);
        // The trigger trips a spring-driven indexing unit, not a rigid rod that
        // would pull the rolling wheel backwards on each return stroke.
        beam({paddle.x,.47f,.42f},{-.45f,.32f,.55f},.065f,steel);
        Mat4 indexer=translate({-.45f,.32f,.55f})*rotateZ(-p.wiperIndex*2*PI/3);
        ring({0,0,0},.3f,.08f,brass,indexer);
        for(int i=0;i<3;++i)ball(transform(indexer,{.23f*std::cos(i*2*PI/3),.23f*std::sin(i*2*PI/3),.08f}),.05f,cream);
        box({-.17f,.42f,.55f},{.12f,.08f,.15f},steel);
        box({-1.13f,.15f,0},{.17f,.22f,1.03f},rubber);
        // Tipping bar depresses a clutch pedal below the crank.
        Mat4 bar=translate({1.6f,.075f,.1f})*rotateZ(p.bar);
        box({0,.98f,0},{.19f,1.96f,.32f},red,bar);
        box({-.16f,1.23f,0},{.48f,.16f,.4f},brass,bar);
        disk({1.6f,.1f,.1f},.15f,.68f,steel);
        float clutch=-p.bar/radians(84);
        box({3.5f,.35f-.12f*clutch,.1f},{.4f,.1f,.45f},brass);
        beam({3.5f,.35f-.12f*clutch,.38f},{3.55f,1.55f,.38f},.065f,steel);
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
        shape(1,translate(slider+Vec3{.18f,0,0})*rotateY(PI/2)*scale({.13f,.13f,.25f}),brass);
        // One compression stroke pushes the rack and lifts the ball stop.
        box({5.65f+.95f*p.rack,2.25f,.78f},{1.3f,.14f,.22f},brass);
        for(int i=0;i<9;++i)box({5.05f+.15f*i+.95f*p.rack,2.36f,.78f},{.065f,.09f,.22f},steel);
        for(float x:{5.05f,6.45f}){
            box({x,1.14f,1},{.1f,2.28f,.12f},dark);ring({x,2.25f,.78f},.2f,.3f,steel);
        }
        Vec3 pawl=slider+Vec3{0,.25f,.65f};
        beam(pawl,{std::min(6.8f,pawl.x+.16f),2.38f,.78f},.065f,steel);
        disk(pawl,.1f,.14f,brass);
        disk({6.93f,1.75f,.2f+p.bolt},.07f,.75f,brass);
        box({6.93f,1.75f,.8f},{.28f,.28f,.16f},dark);
        relay(p);
        // The pin withdraws along Z, allowing the catch to rotate down.
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
        // Bridge end-stop enables the starter shaft; its coupling retracts
        // completely before the car is released at 70 active seconds.
        mount({10.85f,.12f,1.25f},{.7f,.24f,.8f});
        shape(1,translate({10.8f,.83f,1.25f})*rotateY(PI/2)*scale({.32f,.32f,.65f}),dark);
        Mat4 starter=translate(starterTip(p)+Vec3{.45f,0,0})*rotateY(PI/2)*rotateZ(p.starter);
        disk({0,0,0},.09f,.9f,steel,starter);ring({0,0,.27f},.27f,.12f,brass,starter);
        for(int i=0;i<4;++i)box({.12f,0,.27f},{.24f,.045f,.13f},steel,starter*rotateZ(i*PI/2));
        // Coupling withdrawal pulls a wheel shoe sideways out of the car path.
        // The shoe initially touches the right front tire; both must clear.
        box(starterShoe(p),{.28f,.40f,.15f},contact(brass,Starter));
        beam(starterTip(p)+Vec3{.55f,0,0},starterShoe(p)+Vec3{0,.2f,0},.075f,steel);
        beam({9.77f,.08f,1.92f},{10.5f,.08f,1.92f},.09f,dark);
        Vec3 bridgeTrip=transform(bridge,{1.43f,0,1.9f});
        beam(bridgeTrip,{10.85f,.25f,3},.04f,brass);beam({10.85f,.25f,3},{10.85f,.7f,1.25f},.04f,brass);
        // The car travels toward the viewer along +Z, on the same bench level.
        car(p);
        benchLamp(lighting);
    }
};
}
